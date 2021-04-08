/*!
 * @file cdrom_cdc.cpp
 * @brief CD-ROM Conbtroller Device class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-31
 * @copyright GPLv2
 */

#include "../device.h"

#include "./cdrom_skelton.h"
#include "./cdrom_cue.h"
//#include "./cdrom_ccd.h"
//#include "./cdrom_iso.h"

namespace FMTOWNS {

#define EVENT_CMD_SEEK		1 //!< UNTIL SEEK COMPLETED	
#define EVENT_READ_SEEK		2 //!< UNTIL SEEK COMPLETED	
#define EVENT_TIMEOUT		3 //!< UNTIL TIMEOUT 
#define EVENT_DRQ			4 //!< DRQ PER BYTE
#define EVENT_TIMEOUT		5 //!< GENERIC TIMEOUT
#define EVENT_NEXT_SECTOR	6 //!< TO NEXT SECTOR
	
CDROM_CDC::CDROM_CDC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
{
}

CDROM_CDC::~CDROM_CDC()
{
}

void CDROM_CDC::initialize()
{
	cdrom = NULL;
	fifo_size = 8192;
	max_address = (uint32_t)(sizeof(ram));
	event_seek = -1;
	event_drq = -1;
	event_timeout = -1;
	event_read_wait = -1;
	volume_l = 0; // OK?
	volume_r = 0; // OK?
	enable_prefetch = false;
}

void CDROM_CDC::release()
{
	if(cdrom != nullptr) {
		delete cdrom;
		cdrom = NULL;
	}
}

void CDROM_CDC::reset()
{
	// Reset FIFO until data.
	memset(ram, 0x00, sizeof(ram));
	read_ptr = 0x0000;
	write_ptr = 0x0000;
	data_count = 0;
	data_count_tmp = 0;
	tmp_bytes_count = 0;
	access_status = false;
	sectors_remain = 0;
	sample_pos = 0;
	fadeout_level = 16;
	is_playing = false;

	in_track = false;
	target_lba = 0;
	
	cdda_samples[0].w = 0;
	cdda_samples[1].w = 0;
	clear_event(event_seek);
	clear_event(event_drq);
	clear_event(event_read_wait);
	clear_event(event_timeout);

	if(cdrom != nullptr) {
		cdrom->seek_absolute_lba(target_lba, in_track);
	}
	
}

bool CDROM_CDC::seek_to_lba_msf(uint8_t m, uint8_t s, uint8_t s, uint8_t cmdtype)
{
	if(cdrom != NULL) {
		double usec = cdrom->get_seek_time(m, s, f);
		if(usec < 0.0) {
			return false;
		}
		clear_event(event_read_wait);
		clear_event(event_timeout);
		force_register_event(this, (cmdtype == 0x00) ? EVENT_CMD_SEEK : EVENT_READ_SEEK,
							 usec, false, event_seek);
		return true;
	}
	return false;
}
	
pair16_t CDROM_CDC::read_cdda_sample()
{
	pair16_t sample;
	sample.w = 0;
	if(data_count > 0) {
		sample.b.l = ram[read_ptr];
		read_ptr++;
		if(read_ptr >= fifo_size) {
			read_ptr = 0;
		}
		data_count--;
	}
	if(data_count > 0) {
		sample.b.h = ram[read_ptr];
		read_ptr++;
		if(read_ptr >= fifo_size) {
			read_ptr = 0;
		}
		data_count--;
	}
	return sample;
}
	
uint32_t CDROM_CDC::read_dma_io8(uint32_t addr)
{
	uint8_t dat = 0x00;
	int count_bak = data_count;
	if(data_count > 0) {
		dat = ram[read_ptr];
		read_ptr++;
		if(read_ptr >= fifo_size) {
			read_ptr = 0;
		}
		data_count--;
	}
	// Check DRQ status.
	if(data_count <= 0) {
		// Make EOT
		if(count_bak > 0) {
			register_delay_dma_eot();
		}
	}
	return (uint32_t)dat;
}

uint32_t CDROM_CDC::read_io8(uint32_t addr)
{
	/*
	 * 04C0h : Master status
	 * 04C2h : CDC status
	 * 04C4h : DATA
	 * 04CCh : SUBQ CODE
	 * 04CDh : SUBQ STATUS 
	 */
	uint8_t val = 0x00;
	switch(addr & 0x0f) {
	case 0x00:
		val = val | ((mcu_intr)					? 0x80 : 0x00);
		val = val | ((dma_intr)					? 0x40 : 0x00);
		val = val | ((pio_transfer_phase)		? 0x20 : 0x00);
		val = val | ((dma_transfer_phase)		? 0x10 : 0x00); // USING DMAC ch.3
		val = val | ((has_status)				? 0x02 : 0x00);
		val = val | ((mcu_ready)				? 0x01 : 0x00);
		break;
	case 0x02:
		val = read_status();
		break;
	case 0x04:
		if(pio_transfer_phase) {
			if(data_count > 0) {
				val = ram[read_ptr];
				read_ptr++;
				if(read_ptr >= fifo_size) {
					read_ptr = 0;
				}
				data_count--;
			}
			// Check DRQ status.
			if(data_count <= 0) {
				// Make EOT
				pio_transfer_phase = false;
				pio_transfer = false; // OK?
				// Q: Will interrupt?
				mcu_ready = false;
				dma_intr = true;
				mcu_intr = false;
				clear_event(this, event_time_out);
				clear_event(this, event_eot);
				clear_event(this, event_drq);

			}
		}
		break;
	case 0x0c: // SUBQ STATUS
		break;
	case 0x0d: // SUBQ DATA
		break;
	}
	return (uint32_t)val;
}

void CDROM_CDC::write_io8(uint32_t addr, uint32_t data)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
	uint32_t naddr = addr & 0x0f;
	w_regs[naddr] = data;
	switch(naddr) {
	case 0x00: // Master control register
		break;
	case 0x02: // Command
		break;
	case 0x04: // Param
		break;
	case 0x06:
		dma_transfer = ((data & 0x10) != 0) ? true : false;
		pio_transfer = ((data & 0x08) != 0) ? true : false;
		if(dma_transfer) {
			wait_for_dma = false; // From TSUGARU.
			if(data_count > 0) {
				if(!(dma_transfer_phase)) {
					dma_transfer_phase = true;
					force_register_event(this, EVENT_CDROM_DRQ,
										 /*0.25 * 1.0e6 / ((double)transfer_speed * 150.0e3 ) */ 1.0 / 8.0,
										 true, event_drq);
				}
			}
		} else if(pio_transfer) {
			if(data_count > 0) {
				if(!(pio_transfer_phase)) {
					pio_transfer_phase = true;
				}
			}
		}
	}
}

void CDROM_CDC::write_signal(int id, uint32_t data, uint32_t mask)
{
}

uint32_t CDROM_CDC::read_signal(int id)
{
}

void CDROM_CDC::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_CMD_SEEK:
		event_seek = -1;
		status_seek = false;
		if(cdrom != nullptr) {
			cdrom->seek_absolute_lba(target_lba, in_track);
		}
		if(req_status) {
			status_accept(1, 0x00, 0x00);
		}
		if(cdda_playing) {
			set_cdda_status(CDDA_OFF);
			set_subq();
		}
		if(!(req_status)) {
			mcu_ready = true;
			mcu_intr = true;
			write_interrupt(0xffffffff);
		}
		break;
	case EVENT_READ_SEEK:
		event_seek = -1;
		clear_event(event_read_wait);
		clear_event(event_timeout);
		if(cdrom != nullptr) {
			cdrom->seek_absolute_lba(target_lba, in_track);
		}
		
		// If still exists data on ram and if disabled to prefetch.
		tmp_bytes_count = 0;
		if(!(enable_prefetch) && (data_count > 0)) {
			// Retry after some uSecs.
			double usec = 1.0 / 150.0e3;
			if(cdrom != nullptr) {
				usec = cdrom->get_transfer_time_us();
				if(usec <= 0.0) {
					usec = 1.0 / 150.0e3;
				}
			}
			force_register_event(this, EVENT_DATA_SEEK, 16.0 * usec, false, event_seek);
			return;
		}
		
		// OK. Try to read from sector(s).
		if(sectors_remain > 0) {
			double usec = 1.0 / 150.0e3;
			uint32_t bytes = 2352;
			uint32_t lbytes = 2048;
			if(cdrom != nullptr) {
				bytes = cdrom->get_physical_block_size();
				lbytes = cdrom->get_logical_block_size();
				usec = cdrom->get_transfer_time_us();
				if((bytes <= 0) || (usec <= 0.0) || (lbytes <= 0)) {
					// Return error.
					return;
				}
				if(!(cdrom->is_available())) {
					// Return error
					return;
				}
				ssize_t _readsize = 0;
				int r_sectors = 1;
				if(enable_prefetch) {
					if((fifo_size - data_count_tmp) < (lbytes * sectors_remain)) {
						r_sectors = (fifo_size - data_count_tmp) / lbytes;
					}
				}
				if(r_sectors > sectors_remain) r_sectors = sectors_remain;
				if(r_sectors > 0) {
					_readsize = read_sectors(r_sectors);
					if(_readsize != (bytes * r_sectors)) {
						// Error trap?
					}
					usec = usec * (double)bytes * (double)r_sectors;
					usec = usec / 2.0; // OK?
					tmp_bytes_count = lbytes * r_sectors;
					target_lba = target_lba + r_sectors;
					force_register_event(this, (is_audio) ? EVENT_START_CDDA : EVENT_DATA_IN, usec, false, event_data_in);
					sectors_remain -= r_sectors;
				} else {
					// Retry after some uSecs.
					force_register_event(this, EVENT_DATA_SEEK, 16.0 * usec, false, event_seek);
				}
			} else {
				// Disc empty.
			}
		} else {
			// End of reading.
		}
		break;
	case EVENT_DATA_IN:
		event_data_in = -1;
		// Set Status and interrupts
		if(tmp_bytes_count > 0) {
			data_count += tmp_bytes_count;
			if(data_count > fifo_size) {
				data_count = fifo_size;
			}
			if(event_drq < 0) {
				// Register DRQ
				double usec = 1.0 / 150.0e3;
				if(cdrom != nullptr) {
					usec = cdrom->get_transfer_time_us();
				}
				if(usec <= 0.0) usec = 1.0 / 150.0e3;
				usec = usec / 4.0; // OK?
				force_register_event(this, EVENT_DRQ, usec, true, event_drq);
			}
		}
		tmp_bytes_count = 0;
		// ToDo: Make status to DATA IN.
		if(sectors_remain > 0) {
			double usec = 1.0 / 150.0e3;
			uint32_t bytes = 2352;
			// Go To Next sector.
			if(cdrom != nullptr) {
				bytes = cdrom->get_physical_block_size();
				usec = cdrom->get_transfer_time_us();
				if((bytes <= 0) || (usec <= 0.0)) {
					bytes = 2352;
					usec = 1.0 / 150.0e3;
				}
			}
			usec = (usec * (double)bytes) / 2.0; // OK?
//			usec = 5.0e3; // GAP
			force_register_event(this, EVENT_DATA_SEEK,
								 usec, false, event_seek);
		} else {
			// All data has read.
		}
		break;
	case EVENT_DRQ:
		write_drq_signal();
		// MAKE DRQ
		// -> SIGNAL TO DMAC
		// -> DMA:READ FROM CDC(CDROM_CDC::read_dma_io8())
		// -> Check DRQ STATUS.
		break;
	case EVENT_DMA_EOT:
		if(dma_transfer_phase) {
			dma_transfer = false; // OK?
			dma_transfer_phase = false;
			mcu_ready = false;
			dma_intr = true;
			mcu_intr = false;
			clear_event(this, event_time_out);
			clear_event(this, event_eot);
			clear_event(this, event_drq);
			if((data_count <= 0) && (srctors_remain <= 0)) {
				clear_event(this, event_next_sector);
				clear_event(this, event_seek_completed);
				status_read_done(false);
				cdrom_debug_log(_T("EOT(DMA)"));
				
			} else {
				cdrom_debug_log(_T("NEXT(DMA)"));

			}
 			if(!(dma_intr_mask) && (stat_reply_intr)) {
				write_signals(&outputs_mcuint, 0xffffffff);
			}
		}
		break;
	case EVENT_START_CDDA:
		event_data_in = -1;
		if(!(cdda_started)) {
			// Make status to CDDA OK.
			cdda_started = true;
			sample_pos = 0;
			cdda_samples[0].w = 0;
			cdda_samples[1].w = 0;
		}
		update_subq();
		if(event_cdda < 0) {
			// Register DRQ
			double usec = 1.0 / 150.0e3;
			force_register_event(this, EVENT_CDDA, usec, true, event_cdda);
		}
		if(sectors_remain > 0) {
			double usec = (2352.0 * (1.0 / 150.0e3)) / 2.0;
			force_register_event(this, EVENT_DATA_SEEK,
								 usec, false, event_seek);
		}			
		break;
	case EVENT_CDDA:
		// Check if CDDA is end.
		fadeout_level = 16;
		if(!(check_cdda_playing())) {
			// End.
			clear_event(event_cdda);
			double usec = 10.0e3; // OK?
			force_register_event(this, EVENT_FADEOUT_CDDA, usec, true, event_cdda);
			if(is_loop_cdda()) {
				// Re-Seek to begin LBA.
			}
			// Make status.
			
		}
		if(is_audio) {
			if(cdda_started) {
				if(data_count > 0) {
					pair16_t sample = read_cdda_sample();
					cdda_samples[sample_pos & 1].w = sample.w;
					sample_pos++;
				}
			}
		}
		break;
	case EVENT_FADEOUT_CDDA:
		sample_pos = 0;
		if(fadeout_level < 0) {
			cdda_cample[0].w = 0;
			cdda_cample[1].w = 0;
			is_playing = false;
			clear_event(event_cdda);
			return;
		}
		fadeout_level--:
		break;
	}
}

void CDROM_CDC::mix(int32_t* buffer, int cnt)
{
	if((is_playing) && (fadeout_level > 0)) {
		int32_t l = (int32_t)(cdda_sample[0].sw);
		int32_t r = (int32_t)(cdda_sample[1].sw);
		int flevel = (fadeout_level * 3) - 48.0;
		l = apply_volume(l, flevel + volume_l);
		r = apply_volume(r, flevel + volume_r);
		
		for(int i = 0, j = 0; i < cnt; i++, j += 2) {
			int32_t sl, sr;
			sl = l + buf[j + 0];
			sr = r + buf[j + 1];
			if(sl > 32767) {
				sl = 32767;
			} else if(sl < -32768) {
				sl = -32768;
			}
			if(sr > 32767) {
				sr = 32767;
			} else if(sr < -32768) {
				sr = -32768;
			}
			buf[j + 0] = sl;
			buf[j + 1] = sr;
		}
	}
}
	
/*!
 * @brief READ a sector from CDROM_SKELTON::read.
 */
ssize_t CDROM_CDC::read_sectors(int sectors)
{
	ssize_t read_size = 0;
	ssize_t read_count = 0;
	enum CDROM_META::CDIMAGE_TRACK_TYPE trktype = CDROM_SKELTON::TRACKTYPE_NONE;	
	
	if((cdrom != nullptr) && (mounted())) {
		trktype = cdrom->get_track_type();
		if(trktype == CDROM_META::TRACKTYPE_AUDIO) {
			pait16_t *buf = NULL;
			buf = malloc(sectors * 2352);
			if(buf == nullptr) return 0;
			
			for(int i = 0; i < (sectors * (2352 / 2)) ; i++) {
				buf.w = 0;
			}
			read_size = cdrom->read_cdda(buf, 2352, 1) * 2;
			if(read_size <= 0) {
				free(buf);
				return 0;
			}
			for(int i = 0; i < (read_size / 2); i++) {
				if(data_count_tmp >= fifo_size) break;
				ram[write_ptr] = buf[i].b.l;
				write_ptr++;
				if(write_ptr >= fifo_size) {
					write_ptr = 0;
				}
				data_count_tmp++;
				read_count++;
				
				if(data_count_tmp >= fifo_size) break;
				ram[write_ptr] = buf[i].b.h;
				write_ptr++;
				if(write_ptr >= fifo_size) {
					write_ptr = 0;
				}
				data_count_tmp++;
				read_count++;
			}
			free(buf);
		} else {
			uint8_t *buf = NULL; // ToDo: Larger sector.
			buf = malloc(sectors * 2352);
			if(buf == nullptr) return 0;
			memset(buf, 0x00, sectors * 2352);
			
			switch(trktype) {
			case CDROM_META::TRACKTYPE_MODE1_2048:
			case CDROM_META::TRACKTYPE_MODE1_2352:
			case CDROM_META::TRACKTYPE_MODE1_ISO:
				read_size = cdrom->read_mode1(buf, 2352 * sectors, sectors);
				break;
			case CDROM_META::TRACKTYPE_MODE2_2336:
			case CDROM_META::TRACKTYPE_MODE2_2352:
			case CDROM_META::TRACKTYPE_MODE2_ISO:
				read_size = cdrom->read_mode2(buf, 2352 * sectors, sectors);
				break;
			}
			if(read_size <= 0) {
				free(buf);
				return 0;
			}
			
			for(int i = 0; i < read_size; i++) {
				if(data_count_tmp >= fifo_size) break;
				
				ram[write_ptr] = buf[i];
				write_ptr++;
				if(write_ptr >= fifo_size) {
					write_ptr = 0;
				}
				data_count_tmp++;
				read_count++;
			}
			free(buf);
		}
	}
	return read_count;
}
	
bool CDROM_CDC::mounted()
{
	if(cdrom != nullptr) {
		return cdrom->is_available();
	}
	return false;
}

bool CDROM_CDC::accessed()
{
	return access_status;
}

void CDROM_CDC::open(const _TCHAR* file_path)
{
	close();
	if(FILEIO::IsFileExists(file_path)) {
		CDROM_META::CDIMAGE_TYPE type = CDROM_META::check_type(file_path);
		switch(type) {
		case IMAGETYPE_CUE:
			cdrom = new CDROM_CUE();
			break;
		case IMAGETYPE_ISO:
			cdrom = new CDROM_ISO();
			break;
		case IMAGETYPE_CCD:
			cdrom = new CDROM_CCD();
			break;
		}
		if(cdrom != nullptr) {
			if(cdrom->open(file_path, type)) {
				return; // Succeded.
			} else {
				delete cdrom;
				cdrom = NULL;
				return;
			}
		}
		return;
	}
}

void CDROM_CDC::close()
{
	if(cdrom != nullptr) {
		cdrom->close();
		delete cdrom;
		cdrom = NULL;
	}
}

uint32_t CDROM_CDC::read_debug_data8(uint32_t addr)
{
	uint32_t r_addr = addr;
	if(max_address == 0) {
		return 0;
	}
	r_addr = r_addr % max_address;
	uint8_t data = ram[r_addr];
	return (uint32_t)data;
}

void CDROM_CDC::write_debug_data8(uint32_t addr, uint32_t data)
{
	uint32_t r_addr = addr;
	if(max_address == 0) {
		return;
	}
	r_addr = r_addr % max_address;
	ram[r_addr] = data;
}

bool CDROM_CDC::get_debug_regs_info(_TCHAR* buffer, size_t buffer_len)
{
	if((buffer == nullptr) || (buffer_len <= 1)) {
		return false;
	}
	memset(buffer, 0x00, buffer_len * sizeof(_TCHAR));

	// Print regs
	_TCHAR tmpstr[256] = {0};
	my_stprintf_s(tmpstr, 256 - 1, _T("REGS: +0 +1 +2 +3 +4 +5 +6 +7\n"));
	my_tcscat_s(buffer, buffer_len - 1, tmpstr);
		
	for(r = 0; r < 16; r += 8) {
		memset(tmpstr, 0x00, sizeof(tmpstr));
		my_stprintf_s(tmpstr, 256 - 1, _T("R%d  : %02X %02X %02X %02X %02X %02X %02X %02X\n"),
					  r,
					  regs[r + 0], regs[r + 1], regs[r + 2], regs[r + 3],
					  regs[r + 4], regs[r + 5], regs[r + 6], regs[r + 7]
			);
		my_tcscat_s(buffer, buffer_len - 1, tmpstr);
	}
	memset(tmpstr, 0x00, sizeof(tmpstr));
	
	if(cdrom != nullptr) {
		int64_t lba;
		int track;
		uint8_t m, s, f;
		int64_t sectors;
		bool is_legal = false;
		if(cdrom->is_available()) {
			track = cdrom->get_track();
			lba = cdrom->get_lba();
			sectors = cdrom->get_sectors_of_this_track();
			if(!(cdrom->lba_to_msf(lba, m, s, f))) {
				m = 0;
				s = 0;
				f = 0;
			} else {
				is_legal = true;
			}
			my_stprintf_s(tmpstr, 256 - 1, _T("\nTRACK:%d LBA=%d (M=%d S=%d F=%d)\n"),
						  track, lba, m, s, f);
			my_tcscat_s(buffer, buffer_len - 1, tmpstr);
			memset(tmpstr, 0x00, sizeof(tmpstr));
			
		} else {
			my_stprintf_s(tmpstr, 256 - 1, _T("\n** CDROM HAS NOT INSERTED **\n"));
			my_tcscat_s(buffer, buffer_len - 1, tmpstr);
		}
	} else {
		my_stprintf_s(tmpstr, 256 - 1, _T("\n** CDROM HAS NOT INSERTED **\n"));
		my_tcscat_s(buffer, buffer_len - 1, tmpstr);
	}
	return true;
}

bool CDROM_CDC::write_debug_reg(_TCHAR* reg, uint32_t data)
{
	if(reg == nullptr) {
		return false;
	}
	if((reg[0] == 'r') || (reg[0] == 'R')) {
		_TCHAR* np = &(p[1]);
		if((np[0] < '0') && (np[0] > '9')) {
			return false;
		}
		_TCHAR regstr[4] = {0};
		for(int i = 0; i < 2; i++) {
			if((np[i] < '0') && (np[i] > '9')) break;
			regstr[i] = np[i];
		}
		if(strlen(regstr) <= 0) {
			return false;
		}
		int regnum = atoi(np);
		if((regnum < 0) || (regnum > 15)) {
			return false;
		}
		// Must apply after update register.
		write_reg(regnum, data);
		return true;
	}
	return false;
}

void CDROM_CDC::set_volume(int volume)
{
	volume_l = volume;
	volume_r = volume;
}

void CDROM_CDC::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_l;
	volume_r = decibel_r;
}










