/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ MC6843 / HD46503 ]
*/

#include "mc6843.h"
#include "disk.h"
#include "noise.h"

#define FDC_CMD_SEEK_TRACK_ZERO			0x02
#define FDC_CMD_SEK				0x03
#define FDC_CMD_SINGLE_SECTOR_READ		0x04
#define FDC_CMD_SINGLE_SECTOR_WRITE		0x05
#define FDC_CMD_READ_CRC			0x06
#define FDC_CMD_SINGLE_SECTOR_WRITE_WITH_DDM	0x07
#define FDC_CMD_FREE_FORMAT_READ		0x0a
#define FDC_CMD_FREE_FORWAT_WRITE		0x0b
#define FDC_CMD_MULTIPLE_SECTOR_READ		0x0c
#define FDC_CMD_MULTIPLE_SECTOR_WRITE		0x0d

#define FDC_CMR_MACRO_COMMAND			0x0f
#define FDC_CMR_FWF				0x10
#define FDC_CMR_DMA_FLAG			0x20
#define FDC_CMR_ISR_INTERRUPT_MASK		0x40
#define FDC_CMR_FUNCTION_INTERRUPT_MASK		0x80

#define FDC_ISR_MACRO_COMMAND_COMPLETE		0x01
#define FDC_ISR_SETTING_TIME_COMPLETE		0x02
#define FDC_ISR_STATUS_SENSE_REQUEST		0x04
#define FDC_ISR_STRB				0x08

#define FDC_STRA_DATA_TRANSFER_REQUEST		0x01
#define FDC_STRA_DELETE_DATA_MARK_DETECTED	0x02
#define FDC_STRA_DRIVE_READY			0x04
#define FDC_STRA_TRACK_ZERO			0x08
#define FDC_STRA_WRITE_PROTECT			0x10
#define FDC_STRA_TRACK_NOT_EQUAL		0x20
#define FDC_STRA_INDEX				0x40
#define FDC_STRA_BUSY				0x80

#define FDC_STRB_DATA_TRANSFER_ERROR		0x01
#define FDC_STRB_CRC_ERR			0x02
#define FDC_STRB_DATA_MARK_UNDETECTED		0x04
#define FDC_STRB_SECTOR_ADDRESS_UNDETECTED	0x08
#define FDC_STRB_FILE_INOPERABLE		0x10
#define FDC_STRB_WRITE_ERROR			0x20
#define FDC_STRB_HARD_ERROR			0x40

#define FDC_CCR_CRC_ENABLE			0x01
#define FDC_CCR_SHIFT_CRC			0x02

#define MACRO_COMMAND				(cmr & FDC_CMR_MACRO_COMMAND)
#define RUNNING_MACRO_COMMAND			(running_cmr & FDC_CMR_MACRO_COMMAND)

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

void MC6843::cancel_my_event(int event)
{
	if(register_id[event] != -1) {
		cancel_event(this, register_id[event]);
		register_id[event] = -1;
	}
}

void MC6843::register_my_event(int event, double usec)
{
	cancel_my_event(event);
	register_event(this, (event << 8) | (cmdtype & 0xff), usec, false, &register_id[event]);
}

void MC6843::register_seek_event(bool first)
{
	cancel_my_event(EVENT_SEEK);
	if(fdc[drvreg].track == seektrk) {
		register_event(this, (EVENT_SEEK << 8) | (cmdtype & 0xff), 1, false, &register_id[EVENT_SEEK]);
	} else {
		register_event(this, (EVENT_SEEK << 8) | (cmdtype & 0xff), seek_wait_lo[cmr & 3] - (first ? 500 : 0), false, &register_id[EVENT_SEEK]);
	}
	now_seek = true;
}

void MC6843::register_drq_event(int bytes)
{
	double usec = disk[drvreg]->get_usec_per_bytes(bytes) - get_passed_usec(prev_drq_clock);
	if(usec < 4) {
		usec = 4;
	}
	cancel_my_event(EVENT_DRQ);
	register_event(this, (EVENT_DRQ << 8) | (cmdtype & 0xff), usec, false, &register_id[EVENT_DRQ]);
}

void MC6843::register_lost_event(int bytes)
{
	cancel_my_event(EVENT_LOST);
	register_event(this, (EVENT_LOST << 8) | (cmdtype & 0xff), disk[drvreg]->get_usec_per_bytes(bytes), false, &register_id[EVENT_LOST]);
}

void MC6843::initialize()
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
	prev_drq_clock = 0;
}

void MC6843::release()
{
	// release d88 handler
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void MC6843::reset()
{
	// finish previous command
	if(MACRO_COMMAND == FDC_CMD_SINGLE_SECTOR_WRITE || MACRO_COMMAND == FDC_CMD_MULTIPLE_SECTOR_WRITE) {
		// abort sector write command
		if(sector_changed) {
			disk[drvreg]->set_data_crc_error(false);
		}
	} else if(cmdtype == FDC_CMD_FREE_FORWAT_WRITE) {
		// abort free format write command
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

	cmr &= ~FDC_CMR_MACRO_COMMAND;
	isr =  0;
	stra &= (FDC_STRA_DELETE_DATA_MARK_DETECTED | FDC_STRA_TRACK_ZERO | FDC_STRA_WRITE_PROTECT | FDC_STRA_INDEX);
	sar =  0;
	strb &=  FDC_STRB_WRITE_ERROR;

	status_update( );



	stra = stra_tmp = cmr = cmr_tmp = ctar = sar = dir = cmdtype = 0;
	now_search = now_seek = sector_changed = false;
	seektrk = 0;
	seekvct = false;
}

void MC6843::write_io8(uint32_t addr, uint32_t data)
{
	bool ready;
	
	switch(addr & 7) {
	case 0:
		// DOR (Data Out Register)
		write_dma_io8(0, data);
		break;
	case 1:
		// CTAR (Current Track Address Register)
		ctar = data;
		break;
	case 2:
		// CMR (Command Register)
		cmr = data;
		process_command();
		update_status();
		break;
	case 3:
		// SUR (Set Up Register)
		break;
	case 4:
		// SUR (Sector Address Register)
		break;
	case 5:
		// GCR (General Count Register)
		break;
	case 6:
		// CCR (CRC Control Register)
		break;
	case 7:
		// LTAR (Logical Track Register)
		break;
	}
}

uint32_t MC6843::read_io8(uint32_t addr)
{
	switch(addr & 7) {
	case 0:
		// DIR (Data In Register)
		return read_dma_io8();
	case 1:
		// CTAR (Current Track Address Register)
		return ctar & 0x7f; // Bit 7 is read as '0'
	case 2:
		// ISR (Interrupt Status Register)
		val = isr;
		isr &= ~FDC_ISR_STRB;
		update_status();
		return val;
	case 3:
		// STRA (Status Register A)
		if(disk[drvreg]->inserted && motor_on) {
			stra |=  FDC_STRA_DRIVE_READY;
		} else {
			stra &= ~FDC_STRA_DRIVE_READY;
		}
		if(fdc[drvreg].track == 0) {
			stra |=  FDC_STRA_TRACK_ZERO;
		} else {
			stra &= ~FDC_STRA_TRACK_ZERO;
		}
		if(disk[drvreg]->inserted && disk[drvreg]->write_protected) {
			stra |=  FDC_STRA_WRITE_PROTECT;
		} else {
			stra &= ~FDC_STRA_WRITE_PROTECT;
		}
		if((stra & 0x04) && get_cur_position() < disk[drvreg]->get_bytes_per_usec(5000)) {
			stra |=  FDC_STRA_INDEX;
		} else {
			stra &= ~FDC_STRA_INDEX;
		}
		return stra;
	case 4:
		// STRB (Status Register B)
		val = strb;
		strb &= FDC_STRB_DATA_MARK_UNDETECTED;
		update_status();
		return val;
	}



	return 0xff;
}

void MC6843::write_dma_io8(uint32_t addr, uint32_t data)
{
		// data reg
		dir = data;
		ready = ((stra & FDC_ST_DRQ) && !now_search);
		if(!motor_on) ready = false;
//		if(motor_on && (stra & FDC_ST_DRQ) && !now_search) {
		if(ready) {
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				// write or multisector write
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					if(!disk[drvreg]->write_protected) {
						if(disk[drvreg]->sector[fdc[drvreg].index] != dir) {
							disk[drvreg]->sector[fdc[drvreg].index] = dir;
							sector_changed = true;
						}
						// dm, ddm
						disk[drvreg]->set_deleted((cmr & 1) != 0);
					} else {
						stra |= FDC_ST_WRITEFAULT;
						stra &= ~FDC_ST_BUSY;
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
						stra &= ~FDC_ST_BUSY;
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
				} else if(stra & FDC_ST_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				stra &= ~FDC_ST_DRQ;
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
						} else if(dir == 0xf5) {
							// write a1h in missing clock
						} else if(dir == 0xf6) {
							// write c2h in missing clock
						} else if(dir == 0xf7) {
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
									disk[drvreg]->sector[fdc[drvreg].sector_index] = dir;
								}
								fdc[drvreg].sector_index++;
							} else if(dir == 0xf8 || dir == 0xfb) {
								// data mark
								disk[drvreg]->set_deleted(dir == 0xf8);
								fdc[drvreg].sector_found = true;
							}
						}
						disk[drvreg]->track[fdc[drvreg].index] = dir;
					} else {
						stra |= FDC_ST_WRITEFAULT;
						stra &= ~FDC_ST_BUSY;
						stra &= ~FDC_ST_DRQ;
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
					stra &= ~FDC_ST_BUSY;
					cmdtype = 0;
					set_irq(true);
				} else if(stra & FDC_ST_DRQ) {
					if(fdc[drvreg].index == 0) {
						register_drq_event(fdc[drvreg].bytes_before_2nd_drq);
					} else {
						register_drq_event(1);
					}
				}
				stra &= ~FDC_ST_DRQ;
			}
			if(!(stra & FDC_ST_DRQ)) {
				cancel_my_event(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
		break;
}

uint32_t MC6843::read_dma_io8(uint32_t addr)
{
	ready = ((stra & FDC_STRA_DATA_TRANSFER_REQUEST) && !now_search);
	if(!motor_on) ready = false;

	if(ready) {
		if((cmr & FDC_CMR_MACRO_COMMAND) == CMD_SSR || (cmr & FDC_CMR_MACRO_COMMAND) == CMD_MSR) {
			if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
				dir = disk[drvreg]->sector[fdc[drvreg].index];
				//fdc[drvreg].index++;
			}
			if((fdc[drvreg].index + 1) >= disk[drvreg]->sector_size.sd) {
				if(disk[drvreg]->data_crc_error && !disk[drvreg]->ignore_crc()) {
					// data crc error
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF SECTOR (DATA CRC ERROR)\n"));
#endif
					strb |=  FDC_STRB_CRC_ERR;
					stra &= ~FDC_ST_BUSY;
					cmr  &= ~FDC_CMR_MACRO_COMMAND;
					update_status();
				} else if((cmr & FDC_CMR_MACRO_COMMAND) == FDC_CMD_RD_SEC) {
					// single sector read
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF SECTOR\n"));
#endif
					stra &= ~FDC_ST_BUSY;
					cmr  &= ~FDC_CMR_MACRO_COMMAND;
					update_status();
				} else {
					// multi sector read
					if(--cgr == 0xff) {
#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR\n"));
#endif
						stra &= ~FDC_ST_BUSY;
						cmr  &= ~FDC_CMR_MACRO_COMMAND;
						update_status();
					} else {

#ifdef _FDC_DEBUG_LOG
						this->out_debug_log(_T("FDC\tEND OF SECTOR (SEARCH NEXT)\n"));
#endif
						register_my_event(EVENT_MULTI1, 30);
						register_my_event(EVENT_MULTI2, 60);
					}
				}
			} else {
				register_drq_event(1);
			}
			stra &= ~FDC_STRA_DATA_TRANSFER_REQUEST;

		} else if((cmr & 0x0f) == FDC_CMD_RD_ADDR) {
				// read address
				if(fdc[drvreg].index < 6) {
					dir = disk[drvreg]->id[fdc[drvreg].index];
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= 6) {
					if(disk[drvreg]->addr_crc_error && !disk[drvreg]->ignore_crc()) {
						// id crc error
						stra |= FDC_ST_CRCERR;
					}
					stra &= ~FDC_ST_BUSY;
					cmr &= ~0x0f;
					set_irq(true);
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF ID FIELD\n"));
#endif
				} else {
					register_drq_event(1);
				}
				stra &= ~FDC_ST_DRQ;
			} else if((cmr & 0x0f) == FDC_CMD_RD_TRK) {
				// read track
				if(fdc[drvreg].index < disk[drvreg]->get_track_size()) {
					dir = disk[drvreg]->track[fdc[drvreg].index];
					//fdc[drvreg].index++;
				}
				if((fdc[drvreg].index + 1) >= disk[drvreg]->get_track_size()) {
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tEND OF TRACK\n"));
#endif
					stra &= ~FDC_ST_BUSY;
					stra |= FDC_ST_LOSTDATA;
					cmr &= ~0x0f;
					set_irq(true);
				} else {
					register_drq_event(1);
				}
				stra &= ~FDC_ST_DRQ;
			}
			if(!(stra & FDC_ST_DRQ)) {
				cancel_my_event(EVENT_LOST);
				set_drq(false);
				fdc[drvreg].access = true;
			}
		}
	}
	return dir;
}

void MC6843::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MC6843_DRIVEREG) {
		drvreg = data & DRIVE_MASK;
	} else if(id == SIG_MC6843_SIDEREG) {
		sidereg = (data & mask) ? 1 : 0;
	} else if(id == SIG_MC6843_MOTOR) {
		motor_on = ((data & mask) != 0);
	}
}

uint32_t MC6843::read_signal(int ch)
{
	if(ch == SIG_MC6843_DRIVEREG) {
		return drvreg & DRIVE_MASK;
	} else if(ch == SIG_MC6843_SIDEREG) {
		return sidereg & 1;
	} else if(ch == SIG_MC6843_MOTOR) {
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

void MC6843::event_callback(int event_id, int err)
{
	int event = event_id >> 8;
	int cmd = event_id & 0xff;
	register_id[event] = -1;
	
	// cancel event if the command is finished or other command is executed
	if(cmd != MACRO_COMMAND) {
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
		if(seektrk != fdc[drvreg].track) {
			register_seek_event(false);
			break;
		}
		if(MACRO_COMMAND == FDC_CMD_SEEK_TRACK_ZERO) {
			ctar = gcr = 0;
		} else {
			ctar = gcr;
		}
		sar = 0;
		finish_command();
		break;

	case EVENT_SEARCH:
		now_search = false;
		if(stra_tmp & FDC_ST_RECNFND) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tSEARCH NG\n"));
#endif

			stra = stra_tmp & ~(FDC_ST_BUSY | FDC_ST_DRQ);



			cmr &= ~0x0f;
			set_irq(true);
		} else if(stra_tmp & FDC_ST_WRITEFAULT) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tWRITE PROTECTED\n"));
#endif
			stra = stra_tmp & ~(FDC_ST_BUSY | FDC_ST_DRQ);
			cmr &= ~0x0f;
			set_irq(true);
		} else {



			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
				register_lost_event(8);
			} else if(cmdtype == FDC_CMD_WR_TRK) {
				register_lost_event(3);
			} else {
				register_lost_event(1);

			}
			fdc[drvreg].cur_position = fdc[drvreg].next_trans_position;
			fdc[drvreg].prev_clock = prev_drq_clock = get_current_clock();
			if (!(m_CMR & 0x20)) {
				m_ISR |= 0x04; /* if no DMA, set Status Sense */
			}
			stra |= 0x01;
			update_status();
			set_drq(true);
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tSEARCH OK\n"));
#endif
		}
		break;
	case EVENT_DRQ:
		if(stra & FDC_ST_BUSY) {
			stra |= FDC_ST_DRQ;
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
		sar++;
		break;
	case EVENT_MULTI2:
		if(cmdtype == FDC_CMD_RD_MSEC) {
			cmd_readdata(false);
		} else if(cmdtype == FDC_CMD_WR_MSEC) {
			cmd_writedata(false);
		}
		break;
	case EVENT_LOST:
		if(stra & FDC_ST_BUSY) {
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("FDC\tDATA LOST\n"));
#endif
			if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC || cmdtype == FDC_CMD_WR_TRK) {
				if(fdc[drvreg].index == 0) {
					stra &= ~FDC_ST_BUSY;
					//stra &= ~FDC_ST_DRQ;
					cmr &= ~0x0f;
					set_irq(true);
					//set_drq(false);
				} else {
					write_io8(3, 0x00);
				}
			} else {
				read_io8(3);
			}
			stra |= FDC_ST_LOSTDATA;
		}
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

static const _TCHAR *cmdstr[0x10] = {
	_T("---", _T("---"), _T("STZ"), _T("SEK"), _T("SSR"), _T("SSW"), _T("RCR"), _T("SWD"),
	_T("---", _T("---"), _T("FFR"), _T("FFW"), _T("MSR"), _T("MSW"), _T("---"), _T("---"),
};

void MC6843::process_cmd()
{
	stra &= ~FDC_STRA_DATA_TRANSFER_REQUEST;
	stra &= ~FDC_STRA_BUSY;
	
	switch(MACRO_COMMAND) {
	case FDC_CMD_SEEK_TRACK_ZERO:
		cmd_seek_track_zero();
		update_head_flag(drvreg, false);
		break;
	case FDC_CMD_SEEK:
		cmd_seek();
		update_head_flag(drvreg, false);
		break;


	case 0x00: case 0x08:
		cmd_restore();
	case 0x20: case 0x28:
	case 0x30: case 0x38:
		cmd_step();
		update_head_flag(drvreg, false);
		break;
	case 0x40: case 0x48:
	case 0x50: case 0x58:
		cmd_stepin();
		update_head_flag(drvreg, false);
		break;
	case 0x60: case 0x68:
	case 0x70: case 0x78:
		cmd_stepout();
		update_head_flag(drvreg, false);
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

void MC6843::cmd_seek_track_zero()
{
	stra &= ~FDC_STRA_DATA_TRANSFER_REQUEST;
	stra |=  FDC_STRA_BUSY;
	seektrk = 0;
	
	register_seek_event(true);
}

void MC6843::cmd_seek()
{
	stra &= ~FDC_STRA_DATA_TRANSFER_REQUEST;
	stra |=  FDC_STRA_BUSY;
	seektrk = max(gcr, disk[drvreg]->get_max_tracks() - 1);
	
	register_seek_event(true);
}


void MC6843::cmd_read_sector()
{
	stra = FDC_ST_BUSY;
	stra_tmp = search_sector();
	now_search = true;
	
	double time;
	if(stra_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5);
	} else {
		time = get_usec_to_start_trans();
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}


#define FDC_CMD_SEEK_TRACK_ZERO			0x02
#define FDC_CMD_SEK				0x03
#define FDC_CMD_SINGLE_SECTOR_READ		0x04
#define FDC_CMD_SINGLE_SECTOR_WRITE		0x05
#define FDC_CMD_READ_CRC			0x06
#define FDC_CMD_SINGLE_SECTOR_WRITE_WITH_DDM	0x07
#define FDC_CMD_FREE_FORMAT_READ		0x0a
#define FDC_CMD_FREE_FORWAT_WRITE		0x0b
#define FDC_CMD_MULTIPLE_SECTOR_READ		0x0c
#define FDC_CMD_MULTIPLE_SECTOR_WRITE		0x0d


void MC6843::cmd_writedata()
{
	// type-2 write data
	cmdtype = (cmr & 0x10) ? FDC_CMD_WR_MSEC : FDC_CMD_WR_SEC;
	stra = FDC_ST_BUSY;
	stra_tmp = search_sector() & ~FDC_ST_RECTYPE;
	now_search = true;
	sector_changed = false;
	
	double time;
	if(stra_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5);
	} else if(stra & FDC_ST_WRITEFAULT) {
		time = (cmr & 4) ? DELAY_AFTER_HLD : 1;
	} else {
		time = get_usec_to_start_trans();
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MC6843::cmd_readaddr()
{
	// type-3 read address
	cmdtype = FDC_CMD_RD_ADDR;
	stra = FDC_ST_BUSY;
	stra_tmp = search_addr();
	now_search = true;
	
	double time;
	if(stra_tmp & FDC_ST_RECNFND) {
		time = get_usec_to_detect_index_hole(5);
	} else {
		time = get_usec_to_start_trans();
	}
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MC6843::cmd_readtrack()
{
	// type-3 read track
	cmdtype = FDC_CMD_RD_TRK;
	stra = FDC_ST_BUSY;
	stra_tmp = 0;
	
	disk[drvreg]->make_track(fdc[drvreg].track, sidereg);
	fdc[drvreg].index = 0;
	now_search = true;
	
	fdc[drvreg].next_trans_position = 1;
	double time = get_usec_to_detect_index_hole(1);
	register_my_event(EVENT_SEARCH, time);
	cancel_my_event(EVENT_LOST);
}

void MC6843::cmd_writetrack()
{
	// type-3 write track
	cmdtype = FDC_CMD_WR_TRK;
	stra = FDC_ST_BUSY;
	stra_tmp = 0;
	
	fdc[drvreg].index = 0;
	fdc[drvreg].id_written = false;
	now_search = true;
	
	double time;
	if(disk[drvreg]->write_protected) {
		stra_tmp = FDC_ST_WRITEFAULT;
		time = (cmr & 4) ? DELAY_AFTER_HLD : 1;
	} else {
		if(cmr & 4) {
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

void MC6843::cmd_forceint()
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
			if((cmr_tmp & 0x10) || ((cmr_tmp & 0xf0) == 0)) {
				ctar = fdc[drvreg].track;
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
	
	if(cmdtype == 0 || !(stra & FDC_ST_BUSY)) {
		cmdtype = FDC_CMD_TYPE1;
		stra = FDC_ST_HEADENG;
	}
	stra &= ~FDC_ST_BUSY;
	
	// force interrupt if bit0-bit3 is high
	if(cmr & 0x0f) {
		set_irq(true);
	}
	
	cancel_my_event(EVENT_SEEK);
	cancel_my_event(EVENT_SEARCH);
	cancel_my_event(EVENT_DRQ);
	cancel_my_event(EVENT_MULTI1);
	cancel_my_event(EVENT_MULTI2);
	cancel_my_event(EVENT_LOST);
}

void MC6843::update_head_flag(int drv, bool head_load)
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


void MC6843::finish_command()
{
	if(MACRO_COMMAND == CMD_STZ || MACRO_COMMAND == CMD_SEK) {
		m_ISR |= FDC_ISR_SETTING_TIME_COMPLETE;
	} else {
		m_ISR |= FDC_ISR_MACRO_COMMAND_COMPLETE;
	}
	m_STRA &= ~FDC_STRA_BUSY;
	m_CMR  &= ~FDC_CMR_MACRO_COMMAND;
	update_status();
}

void MC6843::update_status()
{
	int irq = 0;

	/* ISR3 */
	if((cmr & 0x40) || !strb) {
		isr &= ~0x08;
	} else {
		isr |=  0x08;
	}
	/* interrupts */
	if(isr & 0x04) {
		irq = 1; /* unmaskable */
	}
	if (!(cmr & 0x80)) {
		/* maskable */
		if( isr & ~0x04) {
			irq = 1;
		}
	}

	m_write_irq( irq );
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

#define FDC_STRA_DATA_TRANSFER_REQUEST		0x01
#define FDC_STRA_DELETE_DATA_MARK_DETECTED	0x02
#define FDC_STRA_DRIVE_READY			0x04
#define FDC_STRA_TRACK_ZERO			0x08
#define FDC_STRA_WRITE_PROTECT			0x10
#define FDC_STRA_TRACK_NOT_EQUAL		0x20
#define FDC_STRA_INDEX				0x40
#define FDC_STRA_BUSY				0x80

#define FDC_STRB_DATA_TRANSFER_ERROR		0x01
#define FDC_STRB_CRC_ERR			0x02
#define FDC_STRB_DATA_MARK_UNDETECTED		0x04
#define FDC_STRB_SECTOR_ADDRESS_UNDETECTED	0x08
#define FDC_STRB_FILE_INOPERABLE		0x10
#define FDC_STRB_WRITE_ERROR			0x20
#define FDC_STRB_HARD_ERROR			0x40

bool MC6843::search_track()
{
	// get track
	int track = fdc[drvreg].track;
	
	if(!disk[drvreg]->get_track(track, sidereg)){
		return false;
	}
	
	// verify track number
	if(disk[drvreg]->ignore_crc()) {
		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			disk[drvreg]->get_sector(-1, -1, i);
			if(disk[drvreg]->id[0] == ctar) {
				fdc[drvreg].next_trans_position = disk[drvreg]->id_position[i] + 4 + 2;
				fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[i];
				return true;
			}
		}
	} else {
		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			disk[drvreg]->get_sector(-1, -1, i);
			if(disk[drvreg]->id[0] == ctar && !disk[drvreg]->addr_crc_error) {
				fdc[drvreg].next_trans_position = disk[drvreg]->id_position[i] + 4 + 2;
				fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[i];
				return true;
			}
		}


		for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
			disk[drvreg]->get_sector(-1, -1, i);
			if(disk[drvreg]->id[0] == ctar) {
				return FDC_ST_SEEKERR | FDC_ST_CRCERR;
			}
		}
	}
	return FDC_ST_SEEKERR;
}

bool MC6843::search_sector()
{
	stra_tmp = stra;
	strb_tmp = strb;
	dir_tmp = dir;
	
	// write protect
	if(cmdtype == FDC_CMD_WR_SEC || cmdtype == FDC_CMD_WR_MSEC) {
		if(disk[drvreg]->write_protected) {
			stra_tmp |= FDC_STRA_WRITE_PROTECT;
			return false;
		}
	}
	
	// get track
	int track = fdc[drvreg].track;
	bool track_found = false;
	bool addr_crc_error = false;
	
	if(!disk[drvreg]->get_track(track, sidereg)) {
		strb_tmp |= FDC_STRB_SECTOR_ADDRESS_UNDETECTED
		return false;
	}
	for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
		disk[drvreg]->get_sector(-1, -1, i);
		if(disk[drvreg]->id[0] == ltar) {
			if(!disk[drvreg]->ignore_crc() && disk[drvreg]->addr_crc_error) {
				addr_crc_error = true;
			} else {
				track_found = true;
				break;
			}
		}
	}
	if(!track_found) {
		if(addr_crc_error) {
			m_STRB |= FDC_STRB_CRC_ERR;
		}
		strb_tmp |= FDC_STRB_SECTOR_ADDRESS_UNDETECTED;
		return false;
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
		disk[drvreg]->get_sector(-1, -1, index);
		
		// check id
		if(disk[drvreg]->id[0] != ctar) {
			continue;
		}
		if(disk[drvreg]->id[2] != sar) {
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
		if(disk[drvreg]->deleted) {
			strb |= 0x02;
		}
		return true;
	}
	
	// sector not found
	disk[drvreg]->sector_size.sd = 0;
	return false;
}

uint8_t MC6843::search_addr()
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
	if(disk[drvreg]->get_sector(-1, -1, first_sector)) {
		fdc[drvreg].next_trans_position = disk[drvreg]->id_position[first_sector] + 1;
		fdc[drvreg].next_am1_position = disk[drvreg]->am1_position[first_sector];
		fdc[drvreg].index = 0;
		sar = disk[drvreg]->id[0];
		return 0;
	}
	
	// sector not found
	disk[drvreg]->sector_size.sd = 0;
	return FDC_ST_RECNFND;
}

// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int MC6843::get_cur_position()
{
	return (fdc[drvreg].cur_position + disk[drvreg]->get_bytes_per_usec(get_passed_usec(fdc[drvreg].prev_clock))) % disk[drvreg]->get_track_size();
}

double MC6843::get_usec_to_start_trans()
{
	// get time from current position
	double time = get_usec_to_next_trans_pos();
	return time;
}

double MC6843::get_usec_to_next_trans_pos()
{
	int position = get_cur_position();
	
	if(disk[drvreg]->invalid_format) {
		// XXX: this track is invalid format and the calculated sector position may be incorrect.
		// so use the constant period
		return 50000;
	} else if(/*disk[drvreg]->no_skew &&*/ !disk[drvreg]->correct_timing()) {
		// XXX: this image may be a standard image or coverted from a standard image and skew may be incorrect,
		// so use the period to search the next sector from the current position
		int sector_num = disk[drvreg]->sector_num.sd;
		int bytes = -1;
		
		if(position > disk[drvreg]->sync_position[sector_num - 1]) {
			position -= disk[drvreg]->get_track_size();
		}
		for(int i = 0; i < sector_num; i++) {
			if(position < disk[drvreg]->sync_position[i]) {
				if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) {
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
	int bytes = fdc[drvreg].next_trans_position - position;
	if(fdc[drvreg].next_sync_position < position || bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

double MC6843::get_usec_to_detect_index_hole(int count)
{
	int position = get_cur_position();
	int bytes = disk[drvreg]->get_track_size() * count - position;
	if(bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

// ----------------------------------------------------------------------------
// irq / drq
// ----------------------------------------------------------------------------

void MC6843::set_irq(bool val)
{
	write_signals(&outputs_irq, val ? 0xffffffff : 0);
}

void MC6843::set_drq(bool val)
{
	write_signals(&outputs_drq, val ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void MC6843::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void MC6843::close_disk(int drv)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->close();
		if(drv == drvreg) {
			cmr &= ~0x0f;
		}
		update_head_flag(drv, false);
	}
}

bool MC6843::is_disk_inserted(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void MC6843::is_disk_protected(int drv, bool value)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool MC6843::is_disk_protected(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

uint8_t MC6843::get_media_type(int drv)
{
	if(drv < MAX_DRIVE) {
		if(disk[drv]->inserted) {
			return disk[drv]->media_type;
		}
	}
	return MEDIA_TYPE_UNK;
}

void MC6843::set_drive_type(int drv, uint8_t type)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t MC6843::get_drive_type(int drv)
{
	if(drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void MC6843::set_drive_rpm(int drv, int rpm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void MC6843::set_drive_mfm(int drv, bool mfm)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}

void MC6843::set_track_size(int drv, int size)
{
	if(drv < MAX_DRIVE) {
		disk[drv]->track_size = size;
	}
}

void MC6843::update_config()
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
bool MC6843::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	int position = get_cur_position();
	
	my_stprintf_s(buffer, buffer_len,
	_T("cmr=%02X (%s) DATAREG=%02X DRVREG=%02X ctar=%02X SIDEREG=%d sar=%02X\n")
	_T("UNIT: DRIVE=%d TRACK=%2d SIDE=%d POSITION=%5d/%d"),
	cmr, cmdstr[cmr >> 4], datareg, drvreg, ctar, sidereg, sar,
	drvreg, fdc[drvreg].track, sidereg,
	position, disk[drvreg]->get_track_size());
	
	for(int i = 0; i < disk[drvreg]->sector_num.sd; i++) {
		uint8_t c, h, r, n;
		int length;
		if(disk[drvreg]->get_sector_info(-1, -1, i, &c, &h, &r, &n, &length)) {
			my_tcscat_s(buffer, buffer_len,
			create_string(_T("\nSECTOR %2d: C=%02X H=%02X R=%02X N=%02X SIZE=%4d AM1=%5d DATA=%5d"), i + 1, c, h, r, n, length, disk[drvreg]->am1_position[i], disk[drvreg]->data_position[i]));
			if(position >= disk[drvreg]->am1_position[i] && position < disk[drvreg]->data_position[i] + length) {
				my_tcscat_s(buffer, buffer_len, _T(" <==="));
			}
		}
	}
	return true;
}
#endif

#define STATE_VERSION	6

bool MC6843::process_state(FILEIO* state_fio, bool loading)
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
	state_fio->StateValue(stra);
	state_fio->StateValue(stra_tmp);
	state_fio->StateValue(cmr);
	state_fio->StateValue(cmr_tmp);
	state_fio->StateValue(ctar);
	state_fio->StateValue(sar);
	state_fio->StateValue(datareg);
	state_fio->StateValue(drvreg);
	state_fio->StateValue(sidereg);
	state_fio->StateValue(cmdtype);
	state_fio->StateArray(register_id, sizeof(register_id), 1);
	state_fio->StateValue(now_search);
	state_fio->StateValue(now_seek);
	state_fio->StateValue(sector_changed);
	state_fio->StateValue(seektrk);
	state_fio->StateValue(seekvct);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(prev_drq_clock);
	return true;
}
