/*
	Skelton for retropc emulator

	Origin : XM7
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ MB8877 / MB8876 / MB8866 / MB89311 ]
*/

#include "mb8877.h"
#include "disk.h"
#include "noise.h"

#define FDC_ST_BUSY		0x01	// busy
#define FDC_ST_INDEX		0x02	// index hole
#define FDC_ST_DRQ		0x02	// data request
#define FDC_ST_TRACK00		0x04	// track0
#define FDC_ST_LOSTDATA		0x04	// data lost
#define FDC_ST_CRCERR		0x08	// crc error
#define FDC_ST_SEEKERR		0x10	// seek error
#define FDC_ST_RECNFND		0x10	// sector not found
#define FDC_ST_HEADENG		0x20	// head engage
#define FDC_ST_RECTYPE		0x20	// record type
#define FDC_ST_WRITEFAULT	0x20	// write fault
#define FDC_ST_WRITEP		0x40	// write protectdc
#define FDC_ST_NOTREADY		0x80	// media not inserted

#define FDC_CMD_TYPE1		1
#define FDC_CMD_RD_SEC		2
#define FDC_CMD_RD_MSEC		3
#define FDC_CMD_WR_SEC		4
#define FDC_CMD_WR_MSEC		5
#define FDC_CMD_RD_ADDR		6
#define FDC_CMD_RD_TRK		7
#define FDC_CMD_WR_TRK		8

#define EVENT_SEEK		0
#define EVENT_SEEKEND		1
#define EVENT_SEARCH		2
#define EVENT_DRQ		3
#define EVENT_MULTI1		4
#define EVENT_MULTI2		5
#define EVENT_LOST		6

#define DRIVE_MASK		(MAX_DRIVE - 1)

#define DELAY_AFTER_HLD		(disk[drvreg]->drive_type == DRIVE_TYPE_2HD ? 15000 : 30000)

static const int seek_wait_hi[4] = {3000,  6000, 10000, 16000};	// 2MHz
static const int seek_wait_lo[4] = {6000, 12000, 20000, 30000};	// 1MHz

void MB8877::cancel_my_event(int event)
{
	if(register_id[event] != -1) {
		cancel_event(this, register_id[event]);
		register_id[event] = -1;
	}
}

void MB8877::register_my_event(int event, double usec)
{
	cancel_my_event(event);
	register_event(this, (event << 8) | (cmdtype & 0xff), usec, false, &register_id[event]);
}

void MB8877::register_seek_event(bool first)
{
	cancel_my_event(EVENT_SEEK);
	if(fdc[drvreg].track == seektrk) {
		register_event(this, (EVENT_SEEK << 8) | (cmdtype & 0xff), 1, false, &register_id[EVENT_SEEK]);
	} else if(disk[drvreg]->drive_type == DRIVE_TYPE_2HD) {
		register_event(this, (EVENT_SEEK << 8) | (cmdtype & 0xff), seek_wait_hi[cmdreg & 3] - (first ? 250 : 0), false, &register_id[EVENT_SEEK]);
	} else {
		register_event(this, (EVENT_SEEK << 8) | (cmdtype & 0xff), seek_wait_lo[cmdreg & 3] - (first ? 500 : 0), false, &register_id[EVENT_SEEK]);
	}
	now_seek = true;
}

void MB8877::register_drq_event(int bytes)
{
	double usec = disk[drvreg]->get_usec_per_bytes(bytes) - get_passed_usec(prev_drq_clock);
	if(usec < 4) {
		usec = 4;
	}
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	if((disk[drvreg]->is_special_disk == SPECIAL_DISK_FM7_GAMBLER) ||
	   (disk[drvreg]->is_special_disk == SPECIAL_DISK_FM77AV_PSYOBLADE)) {
		usec = 4;
	}
#endif
	cancel_my_event(EVENT_DRQ);
	register_event(this, (EVENT_DRQ << 8) | (cmdtype & 0xff), usec, false, &register_id[EVENT_DRQ]);
}

void MB8877::register_lost_event(int bytes)
{
	cancel_my_event(EVENT_LOST);
	register_event(this, (EVENT_LOST << 8) | (cmdtype & 0xff), disk[drvreg]->get_usec_per_bytes(bytes), false, &register_id[EVENT_LOST]);
}

void MB8877::initialize()
{
	// initialize d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i] = new DISK(emu);
		disk[i]->set_device_name(_T("%s/Disk #%d"), this_device_name, i + 1);
	}
	
	// initialize noise
	if(d_noise_seek != NULL) {
		d_noise_seek->set_device_name(_T("Noise Player (FDD Seek)"));
		if(!d_noise_seek->load_wav_file(_T("FDDSEEK.WAV"))) {
			if(!d_noise_seek->load_wav_file(_T("FDDSEEK1.WAV"))) {
				d_noise_seek->load_wav_file(_T("SEEK.WAV"));
			}
		}
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_device_name(_T("Noise Player (FDD Head Load)"));
		d_noise_head_down->load_wav_file(_T("HEADDOWN.WAV"));
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_device_name(_T("Noise Player (FDD Head Unload)"));
		d_noise_head_up->load_wav_file(_T("HEADUP.WAV"));
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
	
	// initialize fdc
	memset(fdc, 0, sizeof(fdc));
//	drvreg = sidereg = 0;
	cmdtype = 0;
//	motor_on = drive_sel = false;
	prev_drq_clock = seekend_clock = 0;
}

void MB8877::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void MB8877::reset()
{
	// finish previous command
	if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
		// abort write sector command
		if(sector_changed) {
			disk[drvreg]->set_data_crc_error(false);
		}
	} else if(cmdtype == FDC_CMD_WR_TRK) {
		// abort write track command
		if(!disk[drvreg]->write_protected) {
			if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
				// data mark of last sector is not written
				disk[drvreg]->set_data_mark_missing();
			}
			disk[drvreg]->sync_buffer();
		}
	}
	
	// single events are automatically canceled in event manager
	for(int i = 0; i < (int)array_length(register_id); i++) {
		register_id[i] = -1;
	}
	
	// reset fdc
	memset(fdc, 0, sizeof(fdc));
	status = status_tmp = cmdreg = cmdreg_tmp = trkreg = secreg = datareg = cmdtype = 0;
	now_search = now_seek = sector_changed = false;
	no_command = seektrk = 0;
	seekvct = false;
#ifdef HAS_MB89311
	extended_mode = true;
#endif
	update_ready();
}

void MB8877::write_io8(uint32_t addr, uint32_t data)
{
	bool ready;
	
	switch(addr & 3) {
	case 0:
		// command reg
		cmdreg_tmp = cmdreg;
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		cmdreg = (~data) & 0xff;
#else
		cmdreg = data;
#endif
		process_cmd();
		no_command = 0;
		break;
	case 1:
		// track reg
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		trkreg = (~data) & 0xff;
#else
		trkreg = data;
#endif
		if((status & FDC_ST_BUSY) && (fdc[drvreg].index == 0)) {
			// track reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 2:
		// sector reg
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		secreg = (~data) & 0xff;
#else
		secreg = data;
#endif
		if((status & FDC_ST_BUSY) && (fdc[drvreg].index == 0)) {
			// sector reg is written after command starts
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				process_cmd();
			}
		}
		break;
	case 3:
		// data reg
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		datareg = (~data) & 0xff;
#else
		datareg = data;
#endif
		ready = ((status & FDC_ST_DRQ) && !now_search);
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(disk[drvreg]->is_special_disk != SPECIAL_DISK_FM7_RIGLAS)
#endif
		{
			if(!motor_on) ready = false;
		}
//		if(motor_on && (status & FDC_ST_DRQ) && !now_search) {
		if(ready) {
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				// write or multisector write
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					if(!disk[drvreg]->write_protected) {
						if(disk[drvreg]->sector[fdc[drvreg].index] != datareg) {
							disk[drvreg]->sector[fdc[drvreg].index] = datareg;
							sector_changed = true;
						}
						// dm, ddm
						disk[drvreg]->set_deleted((cmdreg & 1) != 0);
					} else {
						status |= FDC_ST_WRITEFAULT;
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					}
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->sector_size.sd) {
					if(cmdtype == FDC_CMD_WR_SEC) {
						// single sector
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR\n"));
#endif
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					} else {
						// multisector
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR (SEARCH NEXT)\n"));
#endif
						// 2HD: 360rpm, 10410bytes/track -> 0.06246bytes/us
						register_my_event(EVENT_MULTI1, 30); // 0.06246bytes/us * 30us = 1.8738bytes < GAP3
						register_my_event(EVENT_MULTI2, 60); // 0.06246bytes/us * 60us = 3.7476bytes < GAP3
					}
					sector_changed = false;
				} else if(status & FDC_ST_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				status &= ~FDC_ST_DRQ;
			} else if(cmdtype == FDC_CMD_WR_TRK) {
				// write track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					if(!disk[drvreg]->write_protected) {
						if(fdc[drvreg].index == 0) {
							disk[drvreg]->format_track(fdc[drvreg].track, sidereg);
							fdc[drvreg].id_written = false;
							fdc[drvreg].side = sidereg;
							fdc[drvreg].side_changed = false;
						}
						if(fdc[drvreg].side != sidereg) {
							fdc[drvreg].side_changed = true;
						}
						if(fdc[drvreg].side_changed) {
							// abort write track because disk side is changed
						} else if(datareg == 0xf5) {
							// write a1h in missing clock
						} else if(datareg == 0xf6) {
							// write c2h in missing clock
						} else if(datareg == 0xf7) {
							// write crc
							if(!fdc[drvreg].id_written) {
								// insert new sector with data crc error
write_id:
								uint8_t c = 0, h = 0, r = 0, n = 0;
								fdc[drvreg].id_written = true;
								fdc[drvreg].sector_found = false;
								if(fdc[drvreg].index >= 4) {
									c = disk[drvreg]->track[fdc[drvreg].index - 4];
									h = disk[drvreg]->track[fdc[drvreg].index - 3];
									r = disk[drvreg]->track[fdc[drvreg].index - 2];
									n = disk[drvreg]->track[fdc[drvreg].index - 1];
								}
								fdc[drvreg].sector_length = 0x80 << (n & 3);
								fdc[drvreg].sector_index = 0;
								disk[drvreg]->insert_sector(c, h, r, n, false, true, 0xe5, fdc[drvreg].sector_length);
							} else if(fdc[drvreg].sector_found) {
								// clear data crc error if all sector data are written
								if(fdc[drvreg].sector_index == fdc[drvreg].sector_length) {
									disk[drvreg]->set_data_crc_error(false);
								}
								fdc[drvreg].id_written = false;
							} else {
								// data mark of current sector is not written
								disk[drvreg]->set_data_mark_missing();
								goto write_id;
							}
						} else if(fdc[drvreg].id_written) {
							if(fdc[drvreg].sector_found) {
								// sector data
								if(fdc[drvreg].sector_index < fdc[drvreg].sector_length) {
									disk[drvreg]->sector[fdc[drvreg].sector_index] = datareg;
								}
								fdc[drvreg].sector_index++;
							} else if(datareg == 0xf8 || datareg == 0xfb) {
								// data mark
								disk[drvreg]->set_deleted(datareg == 0xf8);
								fdc[drvreg].sector_found = true;
							}
						}
						disk[drvreg]->track[fdc[drvreg].index] = datareg;
					} else {
						status |= FDC_ST_WRITEFAULT;
						status &= ~FDC_ST_BUSY;
						status &= ~FDC_ST_DRQ;
						cmdtype = 0;
						set_irq(true);
					}
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->get_track_size()) {
					if(!disk[drvreg]->write_protected) {
						if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
							// data mark of last sector is not written
							disk[drvreg]->set_data_mark_missing();
						}
						disk[drvreg]->sync_buffer();
					}
					status &= ~FDC_ST_BUSY;
					cmdtype = 0;
					set_irq(true);
				} else if(status & FDC_ST_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				status &= ~FDC_ST_DRQ;
			}
			if(!(status & FDC_ST_DRQ)) {
				cancel_my_event(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
		break;
	}
}

uint32_t MB8877::read_io8(uint32_t addr)
{
	uint32_t val;
	bool not_ready;
	bool ready;
	
	switch(addr & 3) {
	case 0:
		// status reg
		if(now_search) {
			// now sector search
			val = FDC_ST_BUSY;
		} else {
			// disk not inserted, motor stop
			not_ready = !disk[drvreg]->inserted;
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
			if(disk[drvreg]->is_special_disk != SPECIAL_DISK_FM7_RIGLAS)
#endif
			{
				if(!motor_on) not_ready = true;
			}
//			if(!disk[drvreg]->inserted || !motor_on) {
			if(not_ready) {
				status |= FDC_ST_NOTREADY;
			} else {
				status &= ~FDC_ST_NOTREADY;
			}
			// write protected
			if(cmdtype == FDC_CMD_TYPE1 || cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) {
				if(disk[drvreg]->inserted && disk[drvreg]->write_protected) {
					status |= FDC_ST_WRITEP;
				} else {
					status &= ~FDC_ST_WRITEP;
				}
			} else {
				status &= ~FDC_ST_WRITEP;
			}
			// track0, index hole
			if(cmdtype == FDC_CMD_TYPE1) {
				if(fdc[drvreg].track == 0) {
					status |= FDC_ST_TRACK00;
				} else {
					status &= ~FDC_ST_TRACK00;
				}
				// index hole signal width is 5msec (thanks Mr.Sato)
				if(!(status & FDC_ST_NOTREADY) && get_cur_position() < disk[drvreg]->get_bytes_per_usec(5000)) {
					status |= FDC_ST_INDEX;
				} else {
					status &= ~FDC_ST_INDEX;
				}
			}
			// show busy a moment
			val = status;
			if(cmdtype == FDC_CMD_TYPE1 && !now_seek) {
				status &= ~FDC_ST_BUSY;
#ifdef MB8877_NO_BUSY_AFTER_SEEK
	#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
				if(disk[0]->is_special_disk != SPECIAL_DISK_FM7_XANADU2_D)
	#endif
				{
					val &= ~FDC_ST_BUSY;
				}
#endif
			}
		}
		if(cmdtype == 0 && !(status & FDC_ST_NOTREADY)) {
			// MZ-2000 HuBASIC invites NOT READY status
			if(++no_command == 16) {
				val |= FDC_ST_NOTREADY;
			}
		} else {
			no_command = 0;
		}
		// reset irq/drq
		if(!(status & FDC_ST_DRQ)) {
			set_drq(false);
		}
		if(!(status & FDC_ST_BUSY)) {
			set_irq(false);
		}
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC\tSTATUS=%2x\n"), val);
#endif
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~val) & 0xff;
#else
		return val;
#endif
	case 1:
		// track reg
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~trkreg) & 0xff;
#else
		return trkreg;
#endif
	case 2:
		// sector reg
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~secreg) & 0xff;
#else
		return secreg;
#endif
	case 3:
		// data reg
		ready = ((status & FDC_ST_DRQ) && !now_search);
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(disk[drvreg]->is_special_disk != SPECIAL_DISK_FM7_RIGLAS)
#endif
		{
			if(!motor_on) ready = false;
		}
//		if(motor_on && (status & FDC_ST_DRQ) && !now_search) {
		if(ready) {
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC) {
				// read or multisector read
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					uint8_t mask = disk[drvreg]->unstable ? disk[drvreg]->unstable[fdc[drvreg].index] : 0;
					datareg = (disk[drvreg]->sector[fdc[drvreg].index] & ~mask) | (rand() & mask);
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->sector_size.sd) {

					if(disk[drvreg]->data_crc_error && !disk[drvreg]->ignore_crc()) {
						// data crc error
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR (DATA CRC ERROR)\n"));
#endif
						status |= FDC_ST_CRCERR;
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					} else if(cmdtype == FDC_CMD_RD_SEC) {
						// single sector
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR\n"));
#endif
						status &= ~FDC_ST_BUSY;
						cmdtype = 0;
						set_irq(true);
					} else {
						// multisector
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR (SEARCH NEXT)\n"));
#endif
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					}
				} else {
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
			} else if(cmdtype == FDC_CMD_RD_ADDR) {
				// read address
				if(fdc[drvreg].index < 6) {
					datareg = disk[drvreg]->id[fdc[drvreg].index];
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= 6) {
					if(disk[drvreg]->addr_crc_error && !disk[drvreg]->ignore_crc()) {
						// id crc error
						status |= FDC_ST_CRCERR;
					}
					status &= ~FDC_ST_BUSY;
					cmdtype = 0;
					set_irq(true);
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF ID FIELD\n"));
#endif
				} else {
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
			} else if(cmdtype == FDC_CMD_RD_TRK) {
				// read track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					datareg = disk[drvreg]->track[fdc[drvreg].index];
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->get_track_size()) {
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF TRACK\n"));
#endif
					status &= ~FDC_ST_BUSY;
					status |= FDC_ST_LOSTDATA;
					cmdtype = 0;
					set_irq(true);
				} else {
					register_drq_event(1);
				}
				status &= ~FDC_ST_DRQ;
			}
			if(!(status & FDC_ST_DRQ)) {
				cancel_my_event(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
#ifdef _FDC_DEBUG_LOG
		this->force_out_debug_log(_T("FDC\tDATA=%2x\n"), datareg);
#endif
#if defined(HAS_MB8866) || defined(HAS_MB8876)
		return (~datareg) & 0xff;
#else
		return datareg;
#endif
	}
	return 0xff;
}

void MB8877::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(3, data);
}

uint32_t MB8877::read_dma_io8(uint32_t addr)
{
	return read_io8(3);
}

void MB8877::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MB8877_DRIVEREG) {
		drvreg = data & DRIVE_MASK;
		drive_sel = true;
		seekend_clock = get_current_clock();
		update_ready();
	} else if(id == SIG_MB8877_SIDEREG) {
		sidereg = (data & mask) ? 1 : 0;
	} else if(id == SIG_MB8877_MOTOR) {
		motor_on = ((data & mask) != 0);
		update_ready();
	}
}

uint32_t MB8877::read_signal(int ch)
{
	if(ch == SIG_MB8877_DRIVEREG) {
		return drvreg & DRIVE_MASK;
	} else if(ch == SIG_MB8877_SIDEREG) {
		return sidereg & 1;
	} else if(ch == SIG_MB8877_MOTOR) {
		return motor_on ? 1 : 0;
	}
	
	// get access status
	uint32_t stat = 0;
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	if(now_search) {
		stat |= 1 << drvreg;
	}
	return stat;
}

void MB8877::event_callback(int event_id, int err)
{
	int event = event_id >> 8;
	int cmd = event_id & 0xff;
	register_id[event] = -1;
	
	// cancel event if the command is finished or other command is executed
	if(cmd != cmdtype) {
		if(event == EVENT_SEEK || event == EVENT_SEEKEND) {
			now_seek = false;
		} else if(event == EVENT_SEARCH) {
			now_search = false;
		}
		return;
	}
	
	switch(event) {
	case EVENT_SEEK:
#ifdef _FDC_DEBUG_LOG
		//this->out_debug_log(_T("FDC\tSEEK START\n"));
#endif
		if(seektrk > fdc[drvreg].track) {
			fdc[drvreg].track++;
			if(d_noise_seek != NULL) d_noise_seek->play();
		} else if(seektrk < fdc[drvreg].track) {
			fdc[drvreg].track--;
			if(d_noise_seek != NULL) d_noise_seek->play();
		}
		if((cmdreg & 0x10) || ((cmdreg & 0xf0) == 0)) {
			trkreg = fdc[drvreg].track;
		}
		if(seektrk != fdc[drvreg].track) {
			register_seek_event(false);
			break;
		}
		seekend_clock = get_current_clock();
#ifdef HAS_MB89311
		if(extended_mode) {
			if((cmdreg & 0xf4) == 0x44) {
				// read-after-seek
				cmd_readdata(true);
				break;
			} else if((cmdreg & 0xf4) == 0x64) {
				// write-after-seek
				cmd_writedata(true);
				break;
			}
		}
#endif
		status_tmp = status;
		if(cmdreg & 4) {
			// verify
			status_tmp |= search_track();
			double time;
			if(status_tmp & FDC_ST_SEEKERR) {
				time = get_usec_to_detect_index_hole(5, true);
			} else {
				time = get_usec_to_next_trans_pos(true);
			}
			register_my_event(EVENT_SEEKEND, time);
			break;
		}
	case EVENT_SEEKEND:
		now_seek = false;
		status = status_tmp;
		set_irq(true);
#ifdef _FDC_DEBUG_LOG
		//this->out_debug_log(_T("FDC\tSEEK END\n"));
#endif
		break;
	case EVENT_SEARCH:
		now_search = false;
		if(status_tmp & FDC_ST_RECNFND) {
#if defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
			// for SHARP X1 Batten Tanuki
			if(disk[drvreg]->is_special_disk == SPECIAL_DISK_X1_BATTEN && drive_sel) {
				status_tmp &= ~FDC_ST_RECNFND;
			}
#endif
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tSEARCH NG\n"));
#endif
			status = status_tmp & ~(FDC_ST_BUSY | FDC_ST_DRQ);
			cmdtype = 0;
			set_irq(true);
		} else if(status_tmp & FDC_ST_WRITEFAULT) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tWRITE PROTECTED\n"));
#endif
			status = status_tmp & ~(FDC_ST_BUSY | FDC_ST_DRQ);
			cmdtype = 0;
			set_irq(true);
		} else {
			status = status_tmp | (FDC_ST_BUSY | FDC_ST_DRQ);
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				register_lost_event(8);
			} else if(cmdtype == FDC_CMD_WR_TRK) {
				register_lost_event(3);
			} else {
				register_lost_event(1);
			}
			fdc[drvreg].cur_position = fdc[drvreg].next_trans_position;
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			set_drq(true);
			drive_sel = false;
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tSEARCH OK\n"));
#endif
		}
		break;
	case EVENT_DRQ:
		if(status & FDC_ST_BUSY) {
			status |= FDC_ST_DRQ;
			register_lost_event(1);
			if((cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) && fdc[drvreg].index == 0) {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + fdc[drvreg].bytes_before_2nd_drq) % disk[drvreg]->get_track_size();
			} else {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + 1) % disk[drvreg]->get_track_size();
			}
			if(cmdtype == FDC_CMD_RD_SEC || cmdtype == FDC_CMD_RD_MSEC ||
			   cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC ||
			   cmdtype == FDC_CMD_RD_TRK || cmdtype == FDC_CMD_WR_TRK  ||
			   cmdtype == FDC_CMD_RD_ADDR) {
				fdc[drvreg].index++;
			}
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			set_drq(true);
#ifdef _FDC_DEBUG_LOG
			//this->out_debug_log(_T("FDC\tDRQ!\n"));
#endif
		}
		break;
	case EVENT_MULTI1:
		secreg++;
		break;
	case EVENT_MULTI2:
		if(cmdtype == FDC_CMD_RD_MSEC) {
			cmd_readdata(false);
		} else if(cmdtype == FDC_CMD_WR_MSEC) {
			cmd_writedata(false);
		}
		break;
	case EVENT_LOST:
		if(status & FDC_ST_BUSY) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tDATA LOST\n"));
#endif
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) {
				if(fdc[drvreg].index == 0) {
					status &= ~FDC_ST_BUSY;
					//status &= ~FDC_ST_DRQ;
					cmdtype = 0;
					set_irq(true);
					//set_drq(false);
				} else {
					write_io8(3, 0x00);
				}
			} else {
				read_io8(3);
			}
			status |= FDC_ST_LOSTDATA;
		}
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

static const _TCHAR *cmdstr[0x10] = {
	_T("RESTORE "),	_T("SEEK    "),	_T("STEP    "),	_T("STEP    "),
	_T("STEP IN "),	_T("STEP IN "),	_T("STEP OUT"),	_T("STEP OUT"),
	_T("RD DATA "),	_T("RD DATA "),	_T("RD DATA "),	_T("WR DATA "),
	_T("RD ADDR "),	_T("FORCEINT"),	_T("RD TRACK"),	_T("WR TRACK")
};

void MB8877::process_cmd()
{
	set_irq(false);
	set_drq(false);
	
#ifdef HAS_MB89311
	// MB89311 mode commands
	if(cmdreg == 0xfc) {
		// delay (may be only in extended mode)
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (DELAY   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmdtype = status = 0;
		return;
	} else if(cmdreg == 0xfd) {
		// assign parameter
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN PAR) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmdtype = status = 0;
		return;
	} else if(cmdreg == 0xfe) {
		// assign mode
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (ASGN MOD) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		extended_mode = !extended_mode;
		cmdtype = status = 0;
		return;
	} else if(cmdreg == 0xff) {
		// reset (may be only in extended mode)
		#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tCMD=%2xh (RESET   ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
		#endif
		cmd_forceint();
		extended_mode = true;
		return;
	} else if(extended_mode) {
		// type-1
		if((cmdreg & 0xeb) == 0x21) {
			#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tCMD=%2xh (STEP IN ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
			#endif
			cmd_stepin();
			return;
		} else if((cmdreg & 0xeb) == 0x22) {
			#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tCMD=%2xh (STEP OUT) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
			#endif
			cmd_stepout();
			return;
		// type-2
		} else if((cmdreg & 0xf4) == 0x44) {
			// read-after-seek
			#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tCMD=%2xh (RDaftSEK) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
			#endif
			cmd_seek();
			return;
		} else if((cmdreg & 0xf4) == 0x64) {
			// write-after-seek
			#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tCMD=%2xh (WRaftSEK) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
			#endif
			cmd_seek();
			return;
		// type-3
		} else if((cmdreg & 0xfb) == 0xf1) {
			// format
			#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tCMD=%2xh (FORMAT  ) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, datareg, drvreg, trkreg, sidereg, secreg);
			#endif
			cmd_format();
			return;
		}
	}
#endif
	
	// MB8877 mode commands
#ifdef _FDC_DEBUG_LOG
	this->out_debug_log(_T("FDC\tCMD=%2xh (%s) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, cmdstr[cmdreg >> 4], datareg, drvreg, trkreg, sidereg, secreg);
#endif
	
	switch(cmdreg & 0xf8) {
	// type-1
	case 0x00: case 0x08:
		cmd_restore();
		update_head_flag(drvreg, (cmdreg & 8) != 0);
		break;
	case 0x10: case 0x18:
		cmd_seek();
		update_head_flag(drvreg, (cmdreg & 8) != 0);
		break;
	case 0x20: case 0x28:
	case 0x30: case 0x38:
		cmd_step();
		update_head_flag(drvreg, (cmdreg & 8) != 0);
		break;
	case 0x40: case 0x48:
	case 0x50: case 0x58:
		cmd_stepin();
		update_head_flag(drvreg, (cmdreg & 8) != 0);
		break;
	case 0x60: case 0x68:
	case 0x70: case 0x78:
		cmd_stepout();
		update_head_flag(drvreg, (cmdreg & 8) != 0);
		break;
	// type-2
	case 0x80: case 0x88:
	case 0x90: case 0x98:
		cmd_readdata(true);
		update_head_flag(drvreg, true);
		break;
	case 0xa0:case 0xa8:
	case 0xb0: case 0xb8:
		cmd_writedata(true);
		update_head_flag(drvreg, true);
		break;
	// type-3
	case 0xc0:
		cmd_readaddr();
		update_head_flag(drvreg, true);
		break;
	case 0xe0:
		cmd_readtrack();
		update_head_flag(drvreg, true);
		break;
	case 0xf0:
		cmd_writetrack();
		update_head_flag(drvreg, true);
		break;
	// type-4
	case 0xd0: case 0xd8:
		cmd_forceint();
		break;
	// unknown command
	default:
		break;
	}
}

void MB8877::cmd_restore()
{
	// type-1 restore
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	trkreg = 0xff;
	datareg = 0;
	
	seektrk = 0;
	seekvct = true;
	
	register_seek_event(true);
}

void MB8877::cmd_seek()
{
	// type-1 seek
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
//	seektrk = (uint8_t)(fdc[drvreg].track + datareg - trkreg);
	seektrk = datareg;
	if(seektrk >= disk[drvreg]->get_max_tracks()) {
		seektrk = disk[drvreg]->get_max_tracks() - 1;
	} else if(seektrk < 0) {
		seektrk = 0;
	}
//	seekvct = !(datareg > trkreg);
	seekvct = !(seektrk > fdc[drvreg].track);
	
	if(cmdreg & 4) {
		// verify
		if(trkreg != fdc[drvreg].track) {
			status |= FDC_ST_SEEKERR;
			trkreg = fdc[drvreg].track;
		}
	}
	register_seek_event(true);
}

void MB8877::cmd_step()
{
	// type-1 step
	if(seekvct) {
		cmd_stepout();
	} else {
		cmd_stepin();
	}
}

void MB8877::cmd_stepin()
{
	// type-1 step in
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
	seektrk = fdc[drvreg].track + 1;
	if(seektrk >= disk[drvreg]->get_max_tracks()) {
		seektrk = disk[drvreg]->get_max_tracks() - 1;
	} else if(seektrk < 0) {
		seektrk = 0;
	}
	seekvct = false;
	
	if(cmdreg & 4) {
		// verify
		if(trkreg != fdc[drvreg].track) {
			status |= FDC_ST_SEEKERR;
//			trkreg = fdc[drvreg].track;
		}
	}
	register_seek_event(true);
}

void MB8877::cmd_stepout()
{
	// type-1 step out
	cmdtype = FDC_CMD_TYPE1;
	status = FDC_ST_HEADENG | FDC_ST_BUSY;
	
	seektrk = fdc[drvreg].track - 1;
	if(seektrk >= disk[drvreg]->get_max_tracks()) {
		seektrk = disk[drvreg]->get_max_tracks() - 1;
	} else if(seektrk < 0) {
		seektrk = 0;
	}
	seekvct = true;
	
	if(cmdreg & 4) {
		// verify
		if(trkreg != fdc[drvreg].track) {
			status |= FDC_ST_SEEKERR;
//			trkreg = fdc[drvreg].track;
		}
	}
	register_seek_event(true);
}

void MB8877::cmd_readdata(bool first_sector)
{
	// type-2 read data
	cmdtype = (cmdreg & 0x10) ? FDC_CMD_RD_MSEC : FDC_CMD_RD_SEC;
	status = FDC_ST_BUSY;
	status_tmp = search_sector();
	now_search = true;
	
	double time;
	if(status_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5, first_sector && ((cmdreg & 4) != 0));
	} else {
		time = get_usec_to_start_trans(first_sector);
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MB8877::cmd_writedata(bool first_sector)
{
	// type-2 write data
	cmdtype = (cmdreg & 0x10) ? FDC_CMD_WR_MSEC : FDC_CMD_WR_SEC;
	status = FDC_ST_BUSY;
	status_tmp = search_sector() & ~FDC_ST_RECTYPE;
	now_search = true;
	sector_changed = false;
	
	double time;
	if(status_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5, first_sector && ((cmdreg & 4) != 0));
	} else if(status & FDC_ST_WRITEFAULT) {
		time = (cmdreg & 4) ? DELAY_AFTER_HLD : 1;
	} else {
		time = get_usec_to_start_trans(first_sector);
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MB8877::cmd_readaddr()
{
	// type-3 read address
	cmdtype = FDC_CMD_RD_ADDR;
	status = FDC_ST_BUSY;
	status_tmp = search_addr();
	now_search = true;
	
	double time;
	if(status_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5, ((cmdreg & 4) != 0));
	} else {
		time = get_usec_to_start_trans(true);
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MB8877::cmd_readtrack()
{
	// type-3 read track
	cmdtype = FDC_CMD_RD_TRK;
	status = FDC_ST_BUSY;
	status_tmp = 0;
	
	disk[drvreg]->make_track(fdc[drvreg].track, sidereg);
	fdc[drvreg].index = 0;
	now_search = true;
	
	fdc[drvreg].next_trans_position = 1;
	double time = get_usec_to_detect_index_hole(1, ((cmdreg & 4) != 0));
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MB8877::cmd_writetrack()
{
	// type-3 write track
	cmdtype = FDC_CMD_WR_TRK;
	status = FDC_ST_BUSY;
	status_tmp = 0;
	
	fdc[drvreg].index = 0;
	fdc[drvreg].id_written = false;
	now_search = true;
	
	double time;
	if(disk[drvreg]->write_protected) {
		status_tmp = FDC_ST_WRITEFAULT;
		time = (cmdreg & 4) ? DELAY_AFTER_HLD : 1;
	} else {
		if(cmdreg & 4) {
			// wait 15msec before raise first drq
			fdc[drvreg].next_trans_position = (get_cur_position() + disk[drvreg]->get_bytes_per_usec(DELAY_AFTER_HLD)) % disk[drvreg]->get_track_size();
			time = DELAY_AFTER_HLD;
		} else {
			// raise first drq soon
			fdc[drvreg].next_trans_position = (get_cur_position() + 1) % disk[drvreg]->get_track_size();
			time = disk[drvreg]->get_usec_per_bytes(1);
		}
		// wait at least 3bytes before check index hole and raise second drq
		fdc[drvreg].bytes_before_2nd_drq = disk[drvreg]->get_track_size() - fdc[drvreg].next_trans_position;
		if(fdc[drvreg].bytes_before_2nd_drq < 3) {
			fdc[drvreg].bytes_before_2nd_drq += disk[drvreg]->get_track_size();
		}
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

#ifdef HAS_MB89311
void MB8877::cmd_format()
{
	// type-3 format (FIXME: need to implement)
	cmdtype = FDC_CMD_WR_TRK;
	status = FDC_ST_BUSY;
	status_tmp = 0;
	
	fdc[drvreg].index = 0;
	fdc[drvreg].id_written = false;
	now_search = true;
	
	status_tmp = FDC_ST_WRITEFAULT;
	double time = (cmdreg & 4) ? DELAY_AFTER_HLD : 1;
	
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}
#endif

void MB8877::cmd_forceint()
{
	// type-4 force interrupt
	if(cmdtype == FDC_CMD_TYPE1) {
		// abort restore/seek/step command
		if(now_seek) {
			if(seektrk > fdc[drvreg].track) {
				fdc[drvreg].track++;
			} else if(seektrk < fdc[drvreg].track) {
				fdc[drvreg].track--;
			}
			if((cmdreg_tmp & 0x10) || ((cmdreg_tmp & 0xf0) == 0)) {
				trkreg = fdc[drvreg].track;
			}
		}
	} else if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
		// abort write sector command
		if(sector_changed) {
			disk[drvreg]->set_data_crc_error(false);
		}
	} else if(cmdtype == FDC_CMD_WR_TRK) {
		// abort write track command
		if(!disk[drvreg]->write_protected) {
			if(fdc[drvreg].id_written && !fdc[drvreg].sector_found) {
				// data mark of last sector is not written
				disk[drvreg]->set_data_mark_missing();
			}
			disk[drvreg]->sync_buffer();
		}
	}
	now_search = now_seek = sector_changed = false;
	
#ifdef HAS_MB89311
	if(cmdreg == 0xff) {
		// reset command
		cmdtype = FDC_CMD_TYPE1;
		status = FDC_ST_HEADENG;
	} else {
#endif
		if(cmdtype == 0 || !(status & FDC_ST_BUSY)) {
			cmdtype = FDC_CMD_TYPE1;
			status = FDC_ST_HEADENG;
		}
		status &= ~FDC_ST_BUSY;
		
		// force interrupt if bit0-bit3 is high
		if(cmdreg & 0x0f) {
			set_irq(true);
		}
#ifdef HAS_MB89311
	}
#endif
	
	cancel_my_event(EVENT_SEEK);
	cancel_my_event(EVENT_SEEKEND);
	cancel_my_event(EVENT_SEARCH);
	cancel_my_event(EVENT_DRQ);
	cancel_my_event(EVENT_MULTI1);
	cancel_my_event(EVENT_MULTI2);
	cancel_my_event(EVENT_LOST);
}

void MB8877::update_head_flag(int drv, bool head_load)
{
	if(fdc[drv].head_load != head_load) {
		if(head_load) {
			if(d_noise_head_down != NULL) d_noise_head_down->play();
		} else {
			if(d_noise_head_up != NULL) d_noise_head_up->play();
		}
		fdc[drv].head_load = head_load;
	}
}

void MB8877::update_ready()
{
	write_signals(&outputs_rdy, is_drive_ready() ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

uint8_t MB8877::search_track()
{
	// get track
	int track = fdc[drvreg].track;
	
	if(!disk[drvreg]->get_track(track, sidereg)){
		return FDC_ST_SEEKERR;
	}
	
	// verify track number
	if(disk[drvreg]->ignore_crc()) {
		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			if(!disk[drvreg]->get_sector(-1, -1, i)) {
				continue;
			}
			if(disk[drvreg]->drive_mfm != disk[drvreg]->sector_mfm) {
				continue;
			}
			if(disk[drvreg]->id[0] == trkreg) {
				fdc[drvreg].next_trans_position = disk[drvreg]->id_position[i] + 4 + 2;
				fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[i];
				return 0;
			}
		}
	} else {
		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			if(!disk[drvreg]->get_sector(-1, -1, i)) {
				continue;
			}
			if(disk[drvreg]->drive_mfm != disk[drvreg]->sector_mfm) {
				continue;
			}
			if(disk[drvreg]->id[0] == trkreg && !disk[drvreg]->addr_crc_error) {
				fdc[drvreg].next_trans_position = disk[drvreg]->id_position[i] + 4 + 2;
				fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[i];
				return 0;
			}
		}
		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			if(!disk[drvreg]->get_sector(-1, -1, i)) {
				continue;
			}
			if(disk[drvreg]->drive_mfm != disk[drvreg]->sector_mfm) {
				continue;
			}
			if(disk[drvreg]->id[0] == trkreg) {
				return FDC_ST_SEEKERR | FDC_ST_CRCERR;
			}
		}
	}
	return FDC_ST_SEEKERR;
}

uint8_t MB8877::search_sector()
{
	// write protect
	if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
		if(disk[drvreg]->write_protected) {
			return FDC_ST_WRITEFAULT;
		}
	}
	
	// get track
	int track = fdc[drvreg].track;
	
	if(!disk[drvreg]->get_track(track, sidereg)) {
		return FDC_ST_RECNFND;
	}
	
	// get current position
	int sector_num = disk[drvreg]->sector_num.sd;
	int position = get_cur_position();
	
	if(position > disk[drvreg]->am1_position[sector_num - 1]) {
		position -= disk[drvreg]->get_track_size();
	}
	
	// first scanned sector
	int first_sector = 0;
	for(int i = 0; i < sector_num; i++) {
		if(position < disk[drvreg]->am1_position[i]) {
			first_sector = i;
			break;
		}
	}
	
	// scan sectors
	for(int i = 0; i < sector_num; i++) {
		// get sector
		int index = (first_sector + i) % sector_num;
		
		if(!disk[drvreg]->get_sector(-1, -1, index)) {
			continue;
		}
		if(disk[drvreg]->drive_mfm != disk[drvreg]->sector_mfm) {
			continue;
		}
		// check id
		if(disk[drvreg]->id[0] != trkreg) {
			continue;
		}
#if !defined(HAS_MB8866)
		if((cmdreg & 2) && (disk[drvreg]->id[1] & 1) != ((cmdreg >> 3) & 1)) {
			continue;
		}
#endif
		if(disk[drvreg]->id[2] != secreg) {
			continue;
		}
		if(disk[drvreg]->sector_size.sd == 0) {
			continue;
		}
		if(disk[drvreg]->addr_crc_error && !disk[drvreg]->ignore_crc()) {
			// id crc error
			disk[drvreg]->sector_size.sd = 0;
			return FDC_ST_RECNFND | FDC_ST_CRCERR;
		}
		
		// sector found
		if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
			fdc[drvreg].next_trans_position = disk[drvreg]->id_position[index] + 4 + 2;
			fdc[drvreg].bytes_before_2nd_drq = disk[drvreg]->data_position[index] - fdc[drvreg].next_trans_position;
		} else {
			fdc[drvreg].next_trans_position = disk[drvreg]->data_position[index] + 1;
		}
		fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[index];
		fdc[drvreg].index = 0;
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC\tSECTOR FOUND SIZE=$%04x ID=%02x %02x %02x %02x CRC=%02x %02x CRC_ERROR=%d\n"),
			disk[drvreg]->sector_size.sd,
			disk[drvreg]->id[0], disk[drvreg]->id[1], disk[drvreg]->id[2], disk[drvreg]->id[3],
			disk[drvreg]->id[4], disk[drvreg]->id[5],
			disk[drvreg]->data_crc_error ? 1 : 0);
#endif
		return (disk[drvreg]->deleted ? FDC_ST_RECTYPE : 0);
	}
	
	// sector not found
	disk[drvreg]->sector_size.sd = 0;
	return FDC_ST_RECNFND;
}

uint8_t MB8877::search_addr()
{
	// get track
	int track = fdc[drvreg].track;
	
	if(!disk[drvreg]->get_track(track, sidereg)) {
		return FDC_ST_RECNFND;
	}
	
	// get current position
	int sector_num = disk[drvreg]->sector_num.sd;
	int position = get_cur_position();
	
	if(position > disk[drvreg]->am1_position[sector_num - 1]) {
		position -= disk[drvreg]->get_track_size();
	}
	
	// first scanned sector
	int first_sector = 0;
	for(int i = 0; i < sector_num; i++) {
		if(position < disk[drvreg]->am1_position[i]) {
			first_sector = i;
			break;
		}
	}
	
	// get sector
	for(int i = 0; i < sector_num; i++) {
		int index = (first_sector + i) % sector_num;
		
		if(!disk[drvreg]->get_sector(-1, -1, index)) {
			continue;
		}
		if(disk[drvreg]->drive_mfm != disk[drvreg]->sector_mfm) {
			continue;
		}
		fdc[drvreg].next_trans_position = disk[drvreg]->id_position[index] + 1;
		fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[index];
		fdc[drvreg].index = 0;
		secreg = disk[drvreg]->id[0];
		return 0;
	}
	
	// sector not found
	disk[drvreg]->sector_size.sd = 0;
	return FDC_ST_RECNFND;
}

// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int MB8877::get_cur_position()
{
	return (fdc[drvreg].cur_position + disk[drvreg]->get_bytes_per_usec(get_passed_usec(fdc[drvreg].prev_clock))) % disk[drvreg]->get_track_size();
}

double MB8877::get_usec_to_start_trans(bool first_sector)
{
	// get time from current position
	double time = get_usec_to_next_trans_pos(first_sector && ((cmdreg & 4) != 0));
	
#ifdef MB8877_DELAY_AFTER_SEEK
	// wait 60ms to start read/write after seek is finished (FM-Techknow, p.180)
	if(first_sector && time < MB8877_DELAY_AFTER_SEEK - get_passed_usec(seekend_clock)) {
		time += disk[drvreg]->get_usec_per_track();
	}
#endif
	return time;
}

double MB8877::get_usec_to_next_trans_pos(bool delay)
{
	int position = get_cur_position();
	
	if(disk[drvreg]->invalid_format) {
		// XXX: this track is invalid format and the calculated sector position may be incorrect.
		// so use the constant period
		return 50000;
	} else if(/*disk[drvreg]->no_skew && */!disk[drvreg]->correct_timing()) {
		// XXX: this image may be a standard image or coverted from a standard image and skew may be incorrect,
		// so use the period to search the next sector from the current position
		int sector_num = disk[drvreg]->sector_num.sd;
		int bytes = -1;
		
		if(position > disk[drvreg]->am1_position[sector_num - 1]) {
			position -= disk[drvreg]->get_track_size();
		}
		for(int i = 0; i < sector_num; i++) {
			if(position < disk[drvreg]->am1_position[i]) {
				if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
					bytes = (disk[drvreg]->id_position[i] + 4 + 2) - position;
				} else {
					bytes = (disk[drvreg]->data_position[i] + 1) - position;
				}
				if(bytes < 0) {
					bytes += disk[drvreg]->get_track_size(); // to make sure
				}
				break;
			}
		}
		if(bytes > 0) {
			return disk[drvreg]->get_usec_per_bytes(bytes);
		}
		return 50000;
	}
	if(delay) {
		position = (position + disk[drvreg]->get_bytes_per_usec(DELAY_AFTER_HLD)) % disk[drvreg]->get_track_size();
	}
	int bytes = fdc[drvreg].next_trans_position - position;
	if(fdc[drvreg].next_am1_position < position || bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	if(delay) {
		time += DELAY_AFTER_HLD;
	}
	return time;
}

double MB8877::get_usec_to_detect_index_hole(int count, bool delay)
{
	int position = get_cur_position();
	if(delay) {
		position = (position + disk[drvreg]->get_bytes_per_usec(DELAY_AFTER_HLD)) % disk[drvreg]->get_track_size();
	}
	int bytes = disk[drvreg]->get_track_size() * count - position;
	if(bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	double time = disk[drvreg]->get_usec_per_bytes(bytes);
	if(delay) {
		time += DELAY_AFTER_HLD;
	}
	return time;
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------

void MB8877::set_irq(bool val)
{
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void MB8877::set_drq(bool val)
{
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void MB8877::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
		update_ready();
	}
}

void MB8877::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
		if(drv == drvreg) {
			cmdtype = 0;
		}
		update_head_flag(drv, false);
		update_ready();
	}
}

bool MB8877::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

bool MB8877::is_disk_changed(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->changed;
	}
	return false;
}

void MB8877::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MB8877::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

bool MB8877::is_drive_ready()
{
	return is_drive_ready(drvreg);
}

bool MB8877::is_drive_ready(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted && motor_on;
	}
	return false;
}

uint8_t MB8877::get_media_type(int drv)
{
	if(drv < MAX_DRIVE) {
		if(disk[drv]->inserted) {
			return disk[drv]->media_type;
		}
	}
	return MEDIA_TYPE_UNK;
}

void MB8877::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t MB8877::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MB8877::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void MB8877::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}

void MB8877::set_track_size(int drv, int size)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->track_size = size;
	}
}

uint8_t MB8877::fdc_status()
{
	// for each virtual machines
#if defined(_FMR50) || defined(_FMR60)
	return disk[drvreg]->inserted ? 2 : 0;
#else
	return 0;
#endif
}

void MB8877::update_config()
{
	if(d_noise_seek != NULL) {
		d_noise_seek->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_down != NULL) {
		d_noise_head_down->set_mute(!config.sound_noise_fdd);
	}
	if(d_noise_head_up != NULL) {
		d_noise_head_up->set_mute(!config.sound_noise_fdd);
	}
}

#ifdef USE_DEBUGGER
bool MB8877::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	int position = get_cur_position();
	
	my_stprintf_s(buffer, buffer_len,
	_T("CMDREG=%02X (%s) DATAREG=%02X DRVREG=%02X TRKREG=%02X SIDEREG=%d SECREG=%02X\n")
	_T("UNIT: DRIVE=%d TRACK=%2d SIDE=%d POSITION=%5d/%d"),
	cmdreg, cmdstr[cmdreg >> 4], datareg, drvreg, trkreg, sidereg, secreg,
	drvreg, fdc[drvreg].track, sidereg,
	position, disk[drvreg]->get_track_size());
	
	for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
		uint8_t c, h, r, n;
		bool mfm;
		int length;
		if(disk[drvreg]->get_sector_info(-1, -1, i, &c, &h, &r, &n, &mfm, &length)) {
			my_tcscat_s(buffer, buffer_len,
			create_string(_T("\nSECTOR %2d: C=%02X H=%02X R=%02X N=%02X DENS=%s SIZE=%4d AM1=%5d DATA=%5d"),
				i + 1, c, h, r, n, mfm ? "MFM" : " FM", length, disk[drvreg]->am1_position[i], disk[drvreg]->data_position[i]));
			if(position >= disk[drvreg]->am1_position[i] && position < disk[drvreg]->data_position[i] + length) {
				my_tcscat_s(buffer, buffer_len, _T(" <==="));
			}
		}
	}
	return true;
}
#endif

#define STATE_VERSION	6

bool MB8877::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < array_length(fdc); i++) {
		state_fio->StateValue(fdc[i].track);
		state_fio->StateValue(fdc[i].index);
		state_fio->StateValue(fdc[i].access);
		state_fio->StateValue(fdc[i].head_load);
		state_fio->StateValue(fdc[i].id_written);
		state_fio->StateValue(fdc[i].sector_found);
		state_fio->StateValue(fdc[i].sector_length);
		state_fio->StateValue(fdc[i].sector_index);
		state_fio->StateValue(fdc[i].side);
		state_fio->StateValue(fdc[i].side_changed);
		state_fio->StateValue(fdc[i].cur_position);
		state_fio->StateValue(fdc[i].next_trans_position);
		state_fio->StateValue(fdc[i].bytes_before_2nd_drq);
		state_fio->StateValue(fdc[i].next_am1_position);
		state_fio->StateValue(fdc[i].prev_clock);
	}
	for(int i = 0; i < array_length(disk); i++) {
		if(!disk[i]->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateValue(status);
	state_fio->StateValue(status_tmp);
	state_fio->StateValue(cmdreg);
	state_fio->StateValue(cmdreg_tmp);
	state_fio->StateValue(trkreg);
	state_fio->StateValue(secreg);
	state_fio->StateValue(datareg);
	state_fio->StateValue(drvreg);
	state_fio->StateValue(sidereg);
	state_fio->StateValue(cmdtype);
	state_fio->StateArray(register_id, sizeof(register_id), 1);
	state_fio->StateValue(now_search);
	state_fio->StateValue(now_seek);
	state_fio->StateValue(sector_changed);
	state_fio->StateValue(no_command);
	state_fio->StateValue(seektrk);
	state_fio->StateValue(seekvct);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(drive_sel);
	state_fio->StateValue(prev_drq_clock);
	state_fio->StateValue(seekend_clock);
	return true;
}
