/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM based on SCSI CDROM]
*/

/*
 * Note: Re-Write CD-ROM from SCSI_CDROM, but not related from SCSI_DEV.
 * -- 20200411 K.O
 */
#include "../cdrom.h"
#include "../dmac.h"
#include "../../../fifo.h"
#include "../../../fileio.h"
#include "../../debugger.h"
#include "../../types/util_sound.h"

//#include <iostream>
//#include <utility>

// SAME AS SCSI_CDROM::
#define CDDA_OFF		0
#define CDDA_PLAYING	1
#define CDDA_PAUSED		2
#define CDDA_ENDED		3

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA							100
#define EVENT_CDDA_DELAY_PLAY				101
#define EVENT_CDROM_SEEK					102
#define EVENT_CDROM_DELAY_INTERRUPT_ON		103
#define EVENT_CDROM_DELAY_INTERRUPT_OFF		104
#define EVENT_CDDA_DELAY_STOP				105
#define EVENT_CDROM_SEEK_COMPLETED			106
#define EVENT_CDROM_DRQ						107
#define EVENT_CDROM_NEXT_SECTOR				108
#define EVENT_CDROM_DELAY_READY				109
#define EVENT_CDROM_READY_NOSTATUS			110

#define EVENT_CDROM_EOT						112
#define EVENT_CDROM_RESTORE					113
#define EVENT_CDROM_WAIT					114
#define EVENT_CDROM_TIMEOUT					115
#define EVENT_CDROM_READY_EOT				116
#define EVENT_CDROM_READY_CDDAREPLY			117

//#define _CDROM_DEBUG_LOG

// Event must be larger than 116.


namespace FMTOWNS {

// crc table from vm/disk.cpp
const uint16_t TOWNS_CDROM::crc_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};



uint16_t TOWNS_CDROM::calc_subc_crc16(uint8_t *databuf, int bytes, uint16_t initval)
{
	uint16_t crc16 = initval;
	for(int i = 0; i < bytes ; i++) {
		crc16 = (uint16_t)((crc16 << 8) ^ crc_table[(uint8_t)(crc16 >> 8) ^ databuf[i]]);
	}
	return crc16;
}

void TOWNS_CDROM::cdrom_debug_log(const char *fmt, ...)
{
		char strbuf[4096];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(strbuf, 4095, fmt, ap);
		out_debug_log_with_switch(((__CDROM_DEBUG_LOG) || (force_logging)),
								  "%s",
								  strbuf);
		va_end(ap);
}

void TOWNS_CDROM::initialize()
{
   	DEVICE::initialize();
//	__CDROM_DEBUG_LOG = osd->check_feature(_T("_CDROM_DEBUG_LOG"));
	__CDROM_DEBUG_LOG = true;
	_USE_CDROM_PREFETCH = osd->check_feature(_T("USE_CDROM_PREFETCH"));
	force_logging = false;

	subq_overrun = false;
	stat_track = 0;
	track_num = 0;
	status_queue = new FIFO(4); // 4 * (6 + 100 + 2) // With PAD
	// ToDo: MasterDevice::initialize()
	fio_img = new FILEIO();

	memset(img_file_path_bak, 0x00, sizeof(img_file_path_bak));

	if(44100 % emu->get_sound_rate() == 0) {
		mix_loop_num = 44100 / emu->get_sound_rate();
	} else {
		mix_loop_num = 0;
	}
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	event_drq = -1;

	event_next_sector = -1;
	event_seek_completed = -1;
	event_seek = -1;
	event_delay_ready = -1;
	event_cdda_delay_stop = -1;
	event_time_out = -1;
	event_eot = -1;

	// ToDo: larger buffer for later VMs.
	max_fifo_length = ((machine_id == 0x0700) || (machine_id >= 0x0900)) ? 65536 : 8192;
	fifo_length = 8192;
//	fifo_length = 65536;
	databuffer = new FIFO(max_fifo_length);

	cdda_status = CDDA_OFF;
	is_cue = false;
	is_iso = false;
	current_track = 0;
	read_sector = 0;

	transfer_speed = 1;
	for(int i = 0; i < 99; i++) {
		memset(track_data_path[i], 0x00, _MAX_PATH * sizeof(_TCHAR));
	}
	/*!
	  @note values related muting/voluming are set by electric volume,
	        not inside of CD-ROM DRIVE.
	*/
	_decibel_l = 0;
	_decibel_r = 0;
	mute_left = false;
	mute_right = false;

}

void TOWNS_CDROM::release()
{
	if(fio_img != NULL) {
		if(fio_img->IsOpened()) {
			fio_img->Fclose();
		}
		fio_img = NULL;
		delete fio_img;
	}
	if(databuffer != NULL) {
		databuffer->release();
		delete databuffer;
		databuffer = NULL;
	}
	if(status_queue != NULL) {
		status_queue->release();
		delete status_queue;
		status_queue = NULL;
	}

}

void TOWNS_CDROM::reset()
{
	cdrom_debug_log("RESET");
	reset_device();
	// Q: Does not seek to track 0? 20181118 K.O
}

void TOWNS_CDROM::set_dma_intr(bool val)
{
//	cdrom_debug_log(_T("set_dma_intr(%s) MASK=%s stat_reply_intr = %s"),
//				  (val) ? _T("true ") : _T("false"),
//				  (dma_intr_mask) ? _T("ON ") : _T("OFF"),
//				  (stat_reply_intr) ? _T("ON ") : _T("OFF"));
	if(val) {
		// At least, DMA interrupt mask is needed (by TownsOS v.1.1) 20200511 K.O
		dma_intr = true;
		if(!(dma_intr_mask)) {
//			if(mcu_intr) write_signals(&outputs_mcuint, 0x0);
				write_mcuint_signals(0xffffffff);
		}
	} else {
		dma_intr = false;
		write_mcuint_signals(0x0);
	}
}

/*!
 * @brief set interrupt without stat_reply_intr.
 * @param mcu interrupt value to set.
 * @param dma interrupt value to set.
 * @param use_mask true when masking mcu and dma.
 */
void TOWNS_CDROM::set_intrs_force(bool mcu, bool dma, bool use_mask)
{
	__UNLIKELY_IF(use_mask) {
		mcu = (mcu) && !(mcu_intr_mask);
		dma = (dma) && !(dma_intr_mask);
	}
	bool val = mcu | dma;
	write_mcuint_signals((val) ? 0xffffffff : 0);
}

void TOWNS_CDROM::set_mcu_intr(bool val)
{
//	cdrom_debug_log(_T("set_mcu_intr(%s) MASK=%s stat_reply_intr = %s"),
//				  (val) ? _T("true ") : _T("false"),
//				  (mcu_intr_mask) ? _T("ON ") : _T("OFF"),
//				  (stat_reply_intr) ? _T("ON ") : _T("OFF"));
	if(stat_reply_intr) {
		mcu_intr = val;
		write_mcuint_signals((val) ? 0xffffffff : 0);
	} else {
		mcu_intr = val;
		if(!(val)) {
			write_mcuint_signals(0);
		}
	}
	// Execute next command if reserved.
//	if((mcu_ready) && (command_received) && (param_ptr >= 8)) {
//		mcu_ready = false;
//		execute_command(latest_command);
//	}
}

void TOWNS_CDROM::do_dma_eot(bool by_signal)
{
	static const _TCHAR by_dma[] = _T("SIGNAL");
	static const _TCHAR by_event[] =  _T("EVENT");

//	mcu_ready = false;
//	dma_intr = true;
//	mcu_intr = false;

	clear_event(this, event_time_out);
	clear_event(this, event_eot);
//	clear_event(this, event_drq);

	if((read_length <= 0) && (databuffer->empty())) {
		write_signals(&outputs_drq, 0x00000000);
		clear_event(this, event_next_sector);
		clear_event(this, event_seek_completed);
		clear_event(this, event_drq);
		cdrom_debug_log(_T("EOT(%s/DMA)"), (by_signal) ? by_dma : by_event);

		if(dma_transfer_phase) {
			dma_transfer_phase = false;
			dma_transfer = false;
			status_seek = false;
			set_dma_intr(true);
			status_read_done(false);
		}
	} else {
		//cdrom_debug_log(_T("NEXT(%s/DMA)"), (by_signal) ? by_dma : by_event);
		//cdrom_debug_log(_T("NEXT(%s/DMA)"), (by_signal) ? by_dma : by_event);
		// TRY: Register after EOT. 20201123 K.O
		write_signals(&outputs_drq, 0x00000000);
		if(dma_transfer_phase) {
			dma_transfer_phase = false;
			dma_intr = true;
//			if(stat_reply_intr) {
				set_dma_intr(true);
//			}
		}
	}
}

void TOWNS_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool _b = ((data & mask) != 0);

	switch(id) {
	case SIG_TOWNS_CDROM_CDDA_STOP:
		if(cdda_status != CDDA_OFF) {
			if(_b) set_cdda_status(CDDA_OFF);
		}
		break;
	case SIG_TOWNS_CDROM_CDDA_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			if(_b) set_cdda_status(CDDA_PLAYING);
		}
		break;
	case SIG_TOWNS_CDROM_CDDA_PAUSE:
		if(cdda_status != CDDA_PAUSED) {
			if(_b) set_cdda_status(CDDA_PAUSED);
		}
		break;
	case SIG_TOWNS_CDROM_SET_TRACK:
		if(((data < 100) && (data >= 0)) || (data == 0xaa)) {
			stat_track = data;
		}
		break;
	case SIG_TOWNS_CDROM_RESET:
		if((data & mask) != 0) {
			reset_device();
		}
		break;
	case SIG_TOWNS_CDROM_MUTE_L:
		mute_left = ((data & mask) != 0) ? true : false;
		break;
	case SIG_TOWNS_CDROM_MUTE_R:
		mute_right = ((data & mask) != 0) ? true : false;
		break;
	case SIG_TOWNS_CDROM_MUTE_ALL:
		mute_left = ((data & mask) != 0) ? true : false;
		mute_right = mute_left;
		break;
		// By DMA/TC, EOT.
	case SIG_TOWNS_CDROM_DMAINT:
		if(((data & mask) != 0) && (dma_transfer_phase)) {
			//cdrom_debug_log(_T("CAUSED DMA INTERRUPT FROM DMAC"));
			//clear_event(this, event_drq);
			do_dma_eot(true);
		}
		break;
	case SIG_TOWNS_CDROM_DMAACK:
		if(((data & mask) != 0) && (dma_transfer_phase)) {
			//force_register_event(this, EVENT_CDROM_DRQ, 1.0 / 8.0, false, event_drq);
			#if 1
			if(/* (read_length <= 0) && */(databuffer->empty())) {
				clear_event(this, event_drq);
				if((read_length <= 0)) { // ToDo: With prefetch.
					cdrom_debug_log(_T("MAYBE COMPLETE TO TRANSFER ALL DATA"));
					//force_register_event(this, EVENT_CDROM_EOT, 1.0, false, event_eot);
					do_dma_eot(false);
				}
			} else {

			}
			#endif
		}
		break;
	default:
		// ToDo: Implement master devices.
		break;
	}

}

bool TOWNS_CDROM::status_media_changed_or_not_ready(bool forceint)
{
	if(status_not_ready(forceint)) {
		return true;
	}
	if(status_media_changed(forceint)) {
		return true;
	}
	return false;
}

bool TOWNS_CDROM::status_not_ready(bool forceint)
{
	bool _b = mounted();
	if(!(_b)) {
		cdrom_debug_log(_T("CMD (%02X) BUT DISC NOT ACTIVE"), latest_command);
		set_status((forceint) ? true : req_status, 0,
				   TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_DRIVE_NOT_READY, 0, 0);
	}
	return !(_b);
}

bool TOWNS_CDROM::status_media_changed(bool forceint)
{
	bool _b = media_changed;
	if(media_changed) {
		set_status((forceint) ? true : req_status, 0,
				   TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_MEDIA_CHANGED, 0, 0);
	}
	media_changed = false;
	return _b;
}

void TOWNS_CDROM::status_hardware_error(bool forceint)
{
	set_status((forceint) ? true : req_status, 0,
			   TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_HARDWARE_ERROR_04, 0, 0);
}

void TOWNS_CDROM::status_parameter_error(bool forceint)
{
	set_status((forceint) ? true : req_status, 0,
			   TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_PARAMETER_ERROR, 0, 0);
}

void TOWNS_CDROM::status_read_done(bool forceint)
{
	if(forceint) stat_reply_intr = true;
	set_status_read_done(req_status, 0, TOWNS_CD_STATUS_READ_DONE, 0, 0, 0);
}

void TOWNS_CDROM::status_data_ready(bool forceint)
{
	set_status((forceint) ? true : req_status, 0, TOWNS_CD_STATUS_DATA_READY, 0, 0, 0);
//	dma_transfer_phase = false;
//	pio_transfer_phase = false;
//	dma_intr = false;
//	mcu_intr = true;
//	set_status(true, 0, TOWNS_CD_STATUS_DATA_READY, 0, 0, 0);
	cdrom_debug_log(_T("DATA READY"));
}

void TOWNS_CDROM::status_illegal_lba(int extra, uint8_t s1, uint8_t s2, uint8_t s3)
{
	cdrom_debug_log(_T("Error on reading (ILLGLBLKADDR): EXTRA=%d s1=%02X s2=%02X s3=%02X LBA=%d\n"), extra, s1, s2, s3, read_sector);
	set_status(req_status, extra, TOWNS_CD_STATUS_CMD_ABEND, s1, s2, s3);
}

void TOWNS_CDROM::status_accept(int extra, uint8_t s3, uint8_t s4)
{
	// Note: 2020-05-29 K.O
	// Second byte (ARG s1) may below value (Thanks to Soji Yamakawa-San).
	// 00h : NOT PLAYING CDDA
	// 01h : DATA TRACK? (from Towns Linux)
	// 03h : PLAYING CDDA
	// 09h : MEDIA CHANGED? (from Towns Linux)
	// 0Dh : After STOPPING CD-DA.Will clear.
	// 01h and 09h maybe incorrect.
	uint8_t playcode = TOWNS_CD_ACCEPT_DATA_TRACK; // OK?

	if(media_changed) {
		playcode = TOWNS_CD_ACCEPT_MEDIA_CHANGED;
		goto _next_phase;
	}
	if(mounted()) {
		if(toc_table[current_track].is_audio) {
			playcode = TOWNS_CD_ACCEPT_NOERROR;
			if(cdda_status == CDDA_PAUSED) {
				//!< @note From TSUGARU:
				// Probably: Response to A0H, 00 01 xx xx means CDDA is paused.
				// 0256:00001F01 A03100                    MOV     AL,[0031H]   // Second byte of the status code, if the first byte is 00H
				// 0256:00001F04 3C01                      CMP     AL,01H
				// 0256:00001F06 7408                      JE      00001F10
				// 0256:00001F08 E83006                    CALL    0000253B
				// 0256:00001F0B E8EF05                    CALL    000024FD { ()}
				// 0256:00001F0E EB02                      JMP     00001F12
				// 0256:00001F10 B422                      MOV     AH,22H   // Already-Paused Error.
				// 0256:00001F12 C3                        RET
				playcode = TOWNS_CD_ACCEPT_CDDA_PAUSED;
			} else if(cdda_status == CDDA_PLAYING) {
				//!< @note From TSUGARU:
				// Probably: Response to A0H (80H+REQSTA), 00 03 xx xx means CDDA is playing.
				// Confirmed: Shadow of the Beast 2 checks the 2nd byte to be 03 for verifying that the CDDA started playing.
				//    000C:0006F30B 80FC03                    CMP     AH,03H
				//    000C:0006F30E 75ED                      JNE     0006F2FD
				playcode = TOWNS_CD_ACCEPT_CDDA_PLAYING;
			} else if((cdda_status == CDDA_ENDED) || (cdda_stopped)) {
				playcode = TOWNS_CD_ACCEPT_WAIT;
			}
		} else {
//			playcode = TOWNS_CD_ACCEPT_NOERROR;
			if((latest_command & 0xa0) == 0xa0) {
				switch(param_queue[0]) {
				case 0x04:
					playcode = TOWNS_CD_ACCEPT_04H_FOR_CMD_A0H;
					break;
				case 0x08:
					playcode = TOWNS_CD_ACCEPT_08H_FOR_CMD_A0H;
					//playcode = TOWNS_CD_ACCEPT_NOERROR;
					break;
				}
			}
		}
	}
_next_phase:
	cdda_stopped = false;
	media_changed = false;
	cdrom_debug_log(_T("status_accept() %02X %02X %02X EXTRA=%d"), playcode, s3, s4, extra);
	set_status(req_status, extra,
			   TOWNS_CD_STATUS_ACCEPT, playcode, s3, s4);

}

void TOWNS_CDROM::send_mcu_ready()
{
	mcu_ready = true;
	set_mcu_intr(true);
}

void TOWNS_CDROM::set_delay_ready()
{
	// From Towns Linux 2.2
	// But, some software (i.e. Star Cruiser II) failes to loading at 300uS.
	// May need *at least* 1000uS. - 20200517 K.O
  	force_register_event(this, EVENT_CDROM_DELAY_READY, 1000.0, false, event_delay_ready);
}

void TOWNS_CDROM::set_delay_ready_nostatus()
{
	// From Towns Linux 2.2
	// But, some software (i.e. Star Cruiser II) failes to loading at 300uS.
	// May need *at least* 1000uS. - 20200517 K.O
	force_register_event(this, EVENT_CDROM_READY_NOSTATUS, 1000.0, false, event_delay_ready);
}

void TOWNS_CDROM::set_delay_ready_eot()
{
	force_register_event(this, EVENT_CDROM_READY_EOT, 1000.0, false, event_delay_ready);
}

void TOWNS_CDROM::set_delay_ready_cddareply()
{
	force_register_event(this, EVENT_CDROM_READY_CDDAREPLY, 100.0, false, event_delay_ready);
}

void TOWNS_CDROM::status_not_accept(int extra, uint8_t s1, uint8_t s2, uint8_t s3)
{
	set_status(req_status, extra, TOWNS_CD_STATUS_NOT_ACCEPT, s1, s2, s3);
}

const _TCHAR* TOWNS_CDROM::get_cdda_status_name(int _status)
{
	const _TCHAR *playstatus = _T("UNKNOWN");
	switch(_status) {
	case CDDA_OFF:
		playstatus = _T("OFF    ");
		break;
	case CDDA_PLAYING:
		playstatus = _T("PLAY   ");
		break;
	case CDDA_PAUSED:
		playstatus = _T("PAUSE  ");
		break;
	case CDDA_ENDED:
		playstatus = _T("ENDED  ");
		break;
	}
	return playstatus;
}

const _TCHAR* TOWNS_CDROM::get_command_name_from_command(uint8_t cmd)
{
	const _TCHAR *cmdname = _T("UNKNOWN");
	switch(cmd & 0x9f) {
	case CDROM_COMMAND_SEEK: // 00h
		cmdname = _T("CDROM SEEK/RESTORE");
		break;
	case CDROM_COMMAND_READ_MODE2: // 01h
		cmdname = _T("CDROM READ MODE2");
		break;
	case CDROM_COMMAND_READ_MODE1: // 02h
		cmdname = _T("CDROM READ MODE1");
		break;
	case CDROM_COMMAND_READ_RAW: // 03h
		cmdname = _T("CDROM READ RAW");
		break;
	case CDROM_COMMAND_PLAY_TRACK: // 04h
		cmdname = _T("CDROM PLAY TRACK");
		break;
	case CDROM_COMMAND_READ_TOC: // 05h
		cmdname = _T("CDROM READ TOC");
		break;
	case CDROM_COMMAND_READ_CDDA_STATE: // 06h
		cmdname = _T("CDROM READ CDDA STATE (SUBQ)");
		break;
	case CDROM_COMMAND_1F:
		cmdname = _T("CDROM UNKNOWN CMD 1Fh");
		break;
	case CDROM_COMMAND_SET_STATE: // 80h
		cmdname = _T("CDROM SET STATE");
		break;
	case CDROM_COMMAND_SET_CDDASET: // 81h
		cmdname = _T("CDROM SET CDDA STATUS");
		break;
	case CDROM_COMMAND_STOP_CDDA: // 84h
		cmdname = _T("CDROM STOP CDDA");
		break;
	case CDROM_COMMAND_PAUSE_CDDA: // 85h
		cmdname = _T("CDROM PAUSE CDDA");
		break;
	case CDROM_COMMAND_RESUME_CDDA: // 87h
		cmdname = _T("CDROM RESUME CDDA");
		break;
	}
	return cmdname;
}
/*!
 * @brief Execute CD-ROM command.
 * @arg command  CD-ROM command.
 * @note structure of comman is below:
 * @note Bit7 and Bit4-0 : command code
 * @note Bit6            : Interrupt on status at '1'
 * @note Bit5            : Require status (must read from queue)..
 */
void TOWNS_CDROM::execute_command(uint8_t command)
{
	set_mcu_intr(false);
	clear_event(this, event_time_out);

	mcu_ready = false;

	latest_command = command;

	stat_reply_intr	= ((command & 0x40) != 0) ? true : false;
	req_status		= ((command & 0x20) != 0) ? true : false;

	param_ptr = 0;

	command_received = false;
	extra_status = 0;

	switch(command & 0x9f) {
	case CDROM_COMMAND_SEEK: // 00h (RESTORE?)
		{
//			set_cdda_status(CDDA_OFF);
			uint8_t m, s, f;
			m = FROM_BCD(param_queue[0]);
			s = FROM_BCD(param_queue[1]);
			f = FROM_BCD(param_queue[2]);
			int32_t lba = ((m * (60 * 75)) + (s * 75) + f) - 150;
			if(lba < 0) lba = 0;
			next_seek_lba = lba;
			cdrom_debug_log(_T("CMD SEEK(%02X) M/S/F = %d/%d/%d  M2/S2/F2 = %d/%d/%d LBA=%d"), command,
						  TO_BCD(m), TO_BCD(s), TO_BCD(f),
						  TO_BCD(param_queue[3]), TO_BCD(param_queue[4]), TO_BCD(param_queue[5]),
						  lba
			);
			double usec = get_seek_time(0); // At first, seek to track 0.
			if(usec < 10.0) usec = 10.0;
			usec *= 2.0;
			//mcu_ready = true; // From TSUGARU; MCU ready immediately. 20220128 K.O
			// 20200626 K.O
			// At first, SEEK to LBA0.
			// Next, SEEK TO ARG's LBA.
			// Then, If set status to queue if (CMD & 20h) == 20h.
			// Last, *FORCE TO MAKE* interrupt even (CMD & 20h) != 20h..
			// See event_callback(EVENT_CDROM_RESTORE, foo).
			//req_status = true;
			//status_seek = true;

			force_register_event(this,
						   EVENT_CDROM_RESTORE,
						   usec, false, event_seek);
		}
		break;
	case CDROM_COMMAND_READ_MODE2: // 01h
		cdrom_debug_log(_T("CMD READ MODE2(%02X)"), command);
		read_mode = CDROM_READ_MODE2;
		read_cdrom();
//		status_not_accept(0, 0xff, 0xff, 0xff);
		break;
	case CDROM_COMMAND_READ_MODE1: // 02h
		cdrom_debug_log(_T("CMD READ MODE1(%02X)"), command);
		read_mode = CDROM_READ_MODE1;
		read_cdrom();
		break;
	case CDROM_COMMAND_READ_RAW: // 03h
		cdrom_debug_log(_T("CMD READ RAW(%02X)"), command);
		read_mode = CDROM_READ_RAW;
		read_cdrom();
		break;
	case CDROM_COMMAND_PLAY_TRACK: // 04h
		cdrom_debug_log(_T("CMD PLAY TRACK(%02X)"), command);
		play_cdda_from_cmd(); // ToDo : Re-Implement.
//		play_cdda(req_status);
		break;
	case CDROM_COMMAND_READ_TOC: // 05h
		cdrom_debug_log(_T("CMD READ TOC(%02X)"), command);
//		status_seek = false;
		if(req_status) {
			if(status_media_changed_or_not_ready(false)) {
				break;
			}
			status_accept(1, 0x00, 0x00);
//			set_status(true, 1, TOWNS_CD_STATUS_ACCEPT, 0, 0, 0);
		} else {
			set_status(true, 2, TOWNS_CD_STATUS_TOC_ADDR, 0, 0xa0, 0);
		}
		// TOC READING
		break;
	case CDROM_COMMAND_READ_CDDA_STATE: // 06h
		// ToDo: Some softwares check via this command, strongly to be fixed.
		// -- 20220125 K.O
		if(req_status) {
			if(status_media_changed_or_not_ready(false)) {
				break;
			}
			status_accept(1, 0x00, 0x00);
		} else {
			set_delay_ready(); // OK? 20230127 K.O
		}
		cdrom_debug_log(_T("CMD SET CDDA STATE(%02X)"), command);
		break;
	case CDROM_COMMAND_1F:
		cdrom_debug_log(_T("CMD UNKNOWN 1F(%02X)"), command);
		if(req_status) {
			if(status_media_changed_or_not_ready(false)) {
				break;
			}
			status_accept(0, 0x00, 0x00);
		} else {
			set_delay_ready(); // OK? 20230127 K.O
		}
		break;
	case CDROM_COMMAND_SET_STATE: // 80h
		cdrom_debug_log(_T("CMD SET STATE(%02X) PARAM=%02X %02X %02X %02X %02X %02X %02X %02X"),
					  command,
					  param_queue[0],
					  param_queue[1],
					  param_queue[2],
					  param_queue[3],
					  param_queue[4],
					  param_queue[5],
					  param_queue[6],
					  param_queue[7]
			);
		if(req_status) {
			// ToDo:
			if(status_media_changed_or_not_ready(false)) {
				break;
			}

			if(((cdda_status == CDDA_ENDED) || (cdda_status == CDDA_OFF))
			   && ((prev_command & 0x9f) == CDROM_COMMAND_STOP_CDDA)) {
				/// @note RANCEIII (and maybe others) polls until below status.
				/// @note 20201110 K.O
				set_status(req_status, 0, TOWNS_CD_STATUS_ACCEPT, TOWNS_CD_ACCEPT_WAIT, 0x00, 0x00);
				if(cdda_status == CDDA_ENDED) {
					set_cdda_status(CDDA_OFF);
				}
				break;
			} else if(cdda_status == CDDA_PLAYING) {
				status_accept(1, 0x00, 0x00);
				//set_status(req_status, 1, 0x00, TOWNS_CD_ACCEPT_CDDA_PLAYING, 0x00, 0x00);
				break;
			}
			if((cdda_status == CDDA_PAUSED) &&
				((prev_command & 0x9f) == CDROM_COMMAND_PAUSE_CDDA)) {
				/// @note SUPER READ MAHJONG PIV (and maybe others) polls until below status.
				/// @note 20201110 K.O
				set_status(req_status, 0, TOWNS_CD_STATUS_ACCEPT, TOWNS_CD_ACCEPT_WAIT, 0x00, 0x00);
				break;
			}
			if(status_seek) {
				// @note In SUPER REAL MAHJONG PIV, maybe check below.
				// @note 20201110 K.O
				uint8_t __s2 = (toc_table[current_track].is_audio) ? TOWNS_CD_ACCEPT_WAIT : TOWNS_CD_ACCEPT_DATA_TRACK;
				set_status(req_status, 0, TOWNS_CD_STATUS_ACCEPT, __s2, 0x00, 0x00);
				break;
			}
			status_accept(0, 0x00, 0x00);
		} else {
			set_delay_ready(); // OK? 20230127 K.O
		}
		break;
	case CDROM_COMMAND_SET_CDDASET: // 81h
//		stat_reply_intr = true; // OK?
		cdrom_debug_log(_T("CMD CDDA SET(%02X)"), command);
		if(req_status) {
			// FROM Tsugaru:
			// I don't know what to do with this command.
			// CDROM BIOS AH=52H fires command 0xA1 with parameters {07 FF 00 00 00 00 00 00}
			if(!(status_media_changed_or_not_ready(false))) {
				status_accept(0, 0x00, 0x00);
			}
		} else {
			set_delay_ready(); // OK? 20230127 K.O
		}
		break;
	case CDROM_COMMAND_STOP_CDDA: // 84h
		cdrom_debug_log(_T("CMD STOP CDDA(%02X)"), command);
		///@note From Tsugaru : 20200530 K.O
		if(cdda_status == CDDA_PLAYING) {
			force_register_event(this, EVENT_CDDA_DELAY_STOP, 1000.0, false, event_cdda_delay_stop);
		} else {
			clear_event(this, event_cdda_delay_stop);
			stop_cdda_from_cmd();
		}
		break;
	case CDROM_COMMAND_PAUSE_CDDA: // 85h
		cdrom_debug_log(_T("CMD PAUSE CDDA2(%02X)"), command);
		pause_cdda_from_cmd(); // ToDo : Re-Implement.
		break;
	case CDROM_COMMAND_RESUME_CDDA: // 87h
		cdrom_debug_log(_T("CMD RESUME CDDA(%02X)"), command);
		unpause_cdda_from_cmd();
		break;
	default:
		cdrom_debug_log(_T("CMD Illegal(%02X)"), command);
		stat_reply_intr = true; // OK?
		status_not_accept(0, 0x00, 0x00, 0x00); // ToDo: Will implement
		break;
	}
}

void TOWNS_CDROM::set_status_extra(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	status_queue->write(s0);
	status_queue->write(s1);
	status_queue->write(s2);
	status_queue->write(s3);
	cdrom_debug_log(_T("SET EXTRA STATUS %02x: %02x %02x %02x %02x EXTRA COUNT=%d"), latest_command, s0, s1, s2, s3, extra_status);
	set_delay_ready();
	//set_delay_ready_cddareply();
}

void TOWNS_CDROM::set_status_extra_toc_addr(uint8_t s1, uint8_t s2, uint8_t s3)
{
	set_status_extra(TOWNS_CD_STATUS_TOC_ADDR, s1, s2, s3);
	extra_status++;
}

void TOWNS_CDROM::set_status_extra_toc_data(uint8_t s1, uint8_t s2, uint8_t s3)
{
	set_status_extra(TOWNS_CD_STATUS_TOC_DATA, s1, s2, s3);
	extra_status++;
}

uint8_t TOWNS_CDROM::read_status()
{
	uint8_t val = 0xff;
	if(status_queue->empty()) {
		return val;
	}
	val = status_queue->read();
	if((status_queue->empty()) && (extra_status > 0)) {
		set_extra_status();
	}
	return val;
}

uint32_t TOWNS_CDROM::read_dma_io8(uint32_t addr)
{
//	bool is_empty = databuffer->empty();
	if(dma_transfer_phase) {
		fetch_datareg_8();
		// ToDo: Force register EOT JOB IF (read_length <= 0) && (databuffer->empty()).
	}
	return data_reg.b.l;
}

void TOWNS_CDROM::write_dma_io8(uint32_t addr, uint32_t data)
{
	// data_reg = data;
	return; // OK?
}

uint32_t TOWNS_CDROM::read_dma_io16(uint32_t addr)
{
//	bool is_empty = databuffer->empty();
	if(dma_transfer_phase) {
		fetch_datareg_16();
		// ToDo: Force register EOT JOB IF (read_length <= 0) && (databuffer->empty()).
	}
	return data_reg.w;
}

void TOWNS_CDROM::write_dma_io16(uint32_t addr, uint32_t data)
{
	// data_reg = data;
	return; // OK?
}

void TOWNS_CDROM::read_cdrom()
{
//	read_pos = 0;
//	databuffer->clear();
	if((cdda_status != CDDA_OFF) && (cdda_status != CDDA_ENDED)) {
		// @note In SUPER REAL MAHJONG PIV, use PAUSE (A5h) command before reading.
		// @note 20201110 K.O
		set_cdda_status(CDDA_ENDED);
	} else {
		set_cdda_status(CDDA_OFF);
	}
//	if(status_not_ready(false)) {
//		return;
//	}
	if((track_num <= 0) || !(mounted())) {
		set_status(req_status, 0,
				   TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_MEDIA_CHANGED, 0, 0);
		return;
	}

	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
//	uint8_t pad1, dcmd;

	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];

	int32_t lba1 = ((m1 * (60 * 75)) + (s1 * 75) + f1) - 150;
	int32_t lba2 = ((m2 * (60 * 75)) + (s2 * 75) + f2) - 150;

	uint32_t __remain;
	int track = 0;
	extra_status = 0;
	read_length = 0;

	cdrom_debug_log(_T("READ_CDROM LBA1=%d LBA2=%d M1/S1/F1=%02X/%02X/%02X M2/S2/F2=%02X/%02X/%02X PAD=%02X DCMD=%02X"), lba1, lba2,
				  param_queue[0], param_queue[1], param_queue[2],
				  param_queue[3], param_queue[4], param_queue[5],
				  pad1, dcmd);
	if((lba1 > lba2) || (lba1 < 0) || (lba2 < 0)) { // NOOP?
		status_parameter_error(false);
		return;
	}
	set_cdda_status(CDDA_OFF);
	track = get_track(lba1);

	if((track <= 0) || (track >= track_num)) {
 		status_parameter_error(false);
		return;
	}

	__remain = (lba2 - lba1 + 1);
	read_length = __remain * logical_block_size();

	read_sector = lba1;
	dma_transfer_phase = false;
	pio_transfer_phase = false;
	dma_intr = false;
	mcu_intr = false;

	clear_event(this, event_drq);
	clear_event(this, event_next_sector);
	clear_event(this, event_seek_completed);


	// Kick a first
	double usec = get_seek_time(lba1);
	status_seek = false;
	if(usec < 10.0) usec = 10.0;
	databuffer->clear();
	register_event(this, EVENT_CDROM_SEEK_COMPLETED, usec, false, &event_seek_completed);
	/*
	if(req_status) {
		// May not need extra status, integrated after reading. 20200906 K.O
//		set_status(req_status, 0, 0x00, 0x00, 0x00, 0x00);
		set_status(req_status, 2, 0x00, 0x00, 0x00, 0x00);
	} else {
		if(pio_transfer) {
			set_status(true, 0, TOWNS_CD_STATUS_CMD_ABEND, 0x00, 0x00, 0x00); // OK?
		} else {
			set_status(true, 0, TOWNS_CD_STATUS_DATA_READY, 0x00, 0x00, 0x00);
		}
	}
	*/
	status_accept(2, 0x00, 0x00);
}


void TOWNS_CDROM::set_status(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	cdrom_debug_log(_T("SET STATUS: %02X %02X %02X %02X, EXTRA=%d REQ_STATUS=%s"),
					s0, s1, s2, s3,
					extra,
					(_req_status) ? _T("Yes"): _T("No")
		);
	status_queue->clear();
	extra_status = 0;

	if(_req_status) {
		if(extra > 0) extra_status = extra;
		status_queue->write(s0);
		status_queue->write(s1);
		status_queue->write(s2);
		status_queue->write(s3);
		set_delay_ready();
	} else {
		set_delay_ready_nostatus();
	}
}

void TOWNS_CDROM::set_status_read_done(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	cdrom_debug_log(_T("SET STATUS(READ DONE): %02X %02X %02X %02X, EXTRA=%d REQ_STATUS=%s"),
					s0, s1, s2, s3,
					extra,
					(_req_status) ? _T("Yes"): _T("No")
		);
	status_queue->clear();
	extra_status = 0;
//	if(_req_status) {
		if(extra > 0) extra_status = extra;
		status_queue->write(s0);
		status_queue->write(s1);
		status_queue->write(s2);
		status_queue->write(s3);
//	} else {
//		set_delay_ready_eot();
//	}
	set_delay_ready_eot();

}

void TOWNS_CDROM::set_status_cddareply(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	cdrom_debug_log(_T("SET STATUS(CDDA REPLY): %02X %02X %02X %02X, EXTRA=%d REQ_STATUS=%s"),
					s0, s1, s2, s3,
					extra,
					(_req_status) ? _T("Yes"): _T("No")
		);
	status_queue->clear();
	extra_status = 0;

	if(_req_status) {
		if(extra > 0) extra_status = extra;
		status_queue->write(s0);
		status_queue->write(s1);
		status_queue->write(s2);
		status_queue->write(s3);
	}
	set_delay_ready_cddareply();
}

void TOWNS_CDROM::set_status_immediate(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	cdrom_debug_log(_T("SET STATUS(IMMEDIATE): %02X %02X %02X %02X, EXTRA=%d REQ_STATUS=%s"),
					s0, s1, s2, s3,
					extra,
					(_req_status) ? _T("Yes"): _T("No")
		);
	status_queue->clear();
	extra_status = 0;
	if(_req_status) {
		if(extra > 0) extra_status = extra;
		status_queue->write(s0);
		status_queue->write(s1);
		status_queue->write(s2);
		status_queue->write(s3);
	}
	mcu_ready = true;
	mcu_intr = true;
	dma_intr = false;

	if((_req_status) && (stat_reply_intr) && !(mcu_intr_mask)) {
		write_mcuint_signals(0xffffffff);
	}
//	cdrom_debug_log(_T("SET STATUS %02x: %02x %02x %02x %02x EXTRA=%d"), latest_command, s0, s1, s2, s3, extra_status);
}

void TOWNS_CDROM::set_extra_status()
{
	switch(latest_command & 0x9f) {
	//switch(prev_command & 0x9f) {
	case CDROM_COMMAND_SEEK: // seek
		set_status_extra(TOWNS_CD_STATUS_SEEK_COMPLETED, 0x00, 0x00, 0x00);
		extra_status = 0;
		break;
	case CDROM_COMMAND_READ_MODE1:
	case CDROM_COMMAND_READ_MODE2:
	case CDROM_COMMAND_READ_RAW:
		if(extra_status == 2) {
			set_status_extra(TOWNS_CD_STATUS_DATA_READY, 0, 0, 0);
		}
		extra_status = 0;
		break;
	case CDROM_COMMAND_PLAY_TRACK: // PLAY CDDA
		set_status_extra(TOWNS_CD_STATUS_PLAY_DONE, 0, 0, 0);
		extra_status = 0;
		break;
	case CDROM_COMMAND_READ_TOC: // 0x05
			switch(extra_status) {
			case 1:
				set_status_extra_toc_addr(0x00, 0xa0, 0x00);
				break;
			case 2: // st1 = first_track_number
				set_status_extra_toc_data(TO_BCD(0x01), 0x00, 0x00);
				break;
			case 3:
				set_status_extra_toc_addr(0x00, 0xa1, 0x00);
				break;
			case 4:
				set_status_extra_toc_data(TO_BCD(track_num - 1), 0x00, 0x00); // OK?
				break;
			case 5:
				set_status_extra_toc_addr(0x00, 0xa2, 0x00);
				break;
			case 6:
				{
					pair32_t msf;
					msf.d= read_signal(SIG_TOWNS_CDROM_START_MSF_AA);
					set_status_extra_toc_data(msf.b.h2, msf.b.h, msf.b.l); // OK?
					stat_track = 1;
				}
				break;
			default:
				if((extra_status & 0x01) != 0) {
//					stat_track = (extra_status - 2) >> 1;
					uint32_t adr_control = cdrom_get_adr(stat_track);
					set_status_extra_toc_addr(((adr_control & 0x0f) << 4) | ((adr_control >> 4) & 0x0f), TO_BCD((extra_status / 2) - 2), 0x00);
				} else {
					pair32_t msf;
					msf.d = read_signal(SIG_TOWNS_CDROM_START_MSF);
					cdrom_debug_log(_T("TRACK=%d M:S:F=%02X:%02X:%02X"), stat_track - 1, msf.b.h2, msf.b.h, msf.b.l);
					set_status_extra_toc_data(msf.b.h2, msf.b.h, msf.b.l); // OK?
					if((track_num <= 0) || (stat_track >= track_num)) {
						extra_status = 0; // It's end.
					}
				}
				break;
			}
		break;
	case CDROM_COMMAND_SET_STATE: // 80h Thank to Yamakawa-San.20200626 K.O
		if(extra_status > 0) {
			set_status_extra(TOWNS_CD_STATUS_PLAY_DONE, 0x00, 0x00, 0x00);
			extra_status = 0;
		}
		break;
	case CDROM_COMMAND_READ_CDDA_STATE: // READ CDDA status
			switch(extra_status) {
			case 1: // Get current track
				set_status_extra(TOWNS_CD_STATUS_SUBQ_READ, 0x00, read_signal(SIG_TOWNS_CDROM_CURRENT_TRACK), 0x00);
				extra_status++;
				break;
			case 2: // Get current position
				{
					uint32_t msf = read_signal(SIG_TOWNS_CDROM_RELATIVE_MSF);
					set_status_extra(0x19, (msf >> 16) & 0xff, (msf >> 8) & 0xff, msf & 0xff);
					extra_status++;
				}
				break;
			case 3: // Current_msf
				{
					uint32_t msf = read_signal(SIG_TOWNS_CDROM_ABSOLUTE_MSF);
					set_status_extra(0x19, 0x00, (msf >> 16) & 0xff, (msf >> 8) & 0xff);
					extra_status++;
				}
				break;
			case 4:
				{
					uint32_t msf = read_signal(SIG_TOWNS_CDROM_ABSOLUTE_MSF);
					set_status_extra(0x20, msf & 0xff, 0x00, 0x00);
					extra_status = 0;
				}
				break;
			}
		break;
	case CDROM_COMMAND_STOP_CDDA:
		switch(extra_status) {
		case 1:
			set_status_extra(TOWNS_CD_STATUS_STOP_DONE, 0x00, 0x00, 0x00);
			extra_status++;
			break;
		case 2:
			set_status_extra(0x00, TOWNS_CD_ACCEPT_WAIT, 0x00, 0x00);
			extra_status = 0;
			break;
		}
		break;
	case CDROM_COMMAND_PAUSE_CDDA:
		if(extra_status == 1) {
			set_status_extra(TOWNS_CD_STATUS_PAUSE_DONE, 0x00, 0x00, 0x00);
			extra_status = 0;
		}
		break;
	case CDROM_COMMAND_RESUME_CDDA:
		if(extra_status == 1) {
			set_status_extra(TOWNS_CD_STATUS_RESUME_DONE, 0, 0x00, 0x00); // From Tsugaru
			extra_status = 0;
		}
		break;
	}
}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	switch(id) {
	case SIG_TOWNS_CDROM_READ_DATA:
		return data_reg.w;
		break;
	case SIG_TOWNS_CDROM_PLAYING:
		return (cdda_status == CDDA_PLAYING && cdda_interrupt) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CDROM_SAMPLE_L:
		return (uint32_t)abs(cdda_sample_l);
		break;
	case SIG_TOWNS_CDROM_SAMPLE_R:
		return (uint32_t)abs(cdda_sample_r);
		break;
	case SIG_TOWNS_CDROM_IS_MEDIA_INSERTED:
		return ((is_device_ready()) ? 0xffffffff : 0x00000000);
		break;
	case SIG_TOWNS_CDROM_MAX_TRACK:
		if(track_num <= 0) {
			return (uint32_t)(TO_BCD(0x00));
		} else {
			return (uint32_t)(TO_BCD(track_num));
		}
		break;
		/*
	case SIG_TOWNS_CDROM_REACHED_MAX_TRACK:
		if(track_num <= 0) {
			return 0xffffffff;
		} else {
			if(current_track >= track_num) {
				return 0xffffffff;
			} else {
				return 0x00000000;
			}
		}
		break;
		*/
	case SIG_TOWNS_CDROM_CURRENT_TRACK:
		if(current_track > track_num) {
			return 0x00000000;
		} else {
			return TO_BCD(current_track);
		}
		break;
	case SIG_TOWNS_CDROM_START_MSF:
		{
			int trk = stat_track;
			if(trk <= 0) {
				return 0xffffffff;
			}
			if(trk == 0xaa) {
				trk = track_num;
			}
			int index0 = toc_table[trk].index0;
			int index1 = toc_table[trk].index1;
			int pregap = toc_table[trk].pregap;
			//if(pregap > 0) index0 = index0 + pregap;
			//if(index0 < 150) index0 = 150;
			uint32_t lba = (uint32_t)index0;
			uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
			stat_track++;
			return msf;
		}
		break;
	case SIG_TOWNS_CDROM_START_MSF_AA:
		{
			int trk = track_num;
			int index0 = toc_table[trk].index0;
			int index1 = toc_table[trk].index1;
			int pregap = toc_table[trk].pregap;
			uint32_t lba = (uint32_t)index0;
			if(pregap > 0) lba = lba + pregap;
			if(lba < 150) lba = 150;
			uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
			return msf;
		}
		break;
	case SIG_TOWNS_CDROM_RELATIVE_MSF:
		if(!(is_device_ready())) {
			return 0;
		}
		if((current_track <= 0) || (current_track >= track_num)) {
			return 0;
		}
		if(toc_table[current_track].is_audio) {
//			uint32_t index1 = toc_table[current_track].index1;
			uint32_t index1 = cdda_start_frame;
			if(cdda_playing_frame <= cdda_start_frame) {
				return 0;
			}
			if(cdda_end_frame <= cdda_start_frame) {
				return 0;
			}
			uint32_t n = 0;
			uint32_t lba = cdda_playing_frame;
			__UNLIKELY_IF(lba >= cdda_end_frame) {
				if(cdda_repeat_count == 0) {
					lba = 0;
				} else {
					if(cdda_end_frame > index1) {
						lba = cdda_end_frame;
					} else {
						lba = 0;
					}
				}
			}
			 if(lba >= index1) {
				n = lba - index1;
			}
			return lba_to_msf(n);
		} else {
			// Not Audio
			uint32_t index1 = toc_table[current_track].index1;
			uint32_t lba = get_image_cur_position();
			uint32_t n = 0;
			if(lba >= index1) {
				n = lba - index1;
			}
			return lba_to_msf(n);
		}
		break;
	case SIG_TOWNS_CDROM_ABSOLUTE_MSF:
		if(!(is_device_ready())) {
			return 0;
		}
		if((current_track <= 0) || (current_track >= track_num)) {
			return 0;
		}
		{
			uint32_t lba = 0;
			if(toc_table[current_track].is_audio) {
				if((cdda_status == CDDA_PLAYING) || (cdda_status == CDDA_PAUSED)) {
					if(cdda_playing_frame >= max_logical_block) {
						lba = max_logical_block;
					} else {
						lba = cdda_playing_frame;
					}
				} else {
					lba = cdda_end_frame;
				}
			} else {
				lba = get_image_cur_position();
				if(lba > max_logical_block) {
					lba = max_logical_block;
				}
			}
			return lba_to_msf(lba + 75 * 2);
		}
		break;
	case SIG_TOWNS_CDROM_GET_ADR:
		return cdrom_get_adr(stat_track);
		break;
	default:
		// ToDo: Implement master DEV
		//return SCSI_DEV::read_signal(id);
		break;
	}
	return 0; // END TRAM
}

uint32_t TOWNS_CDROM::cdrom_get_adr(int trk)
{
	if(!(is_device_ready())) {
		return 0xffffffff; // OK?
	}
	if(trk == 0xaa) {
		return 0x10; // AUDIO SUBQ
	}
	if(trk > track_num) {
		return 0xffffffff; // OK?
	}
	if(toc_table[trk].is_audio) {
		return 0x10;
	}
	return 0x14; // return as data
}

const int TOWNS_CDROM::physical_block_size()
{
	if(current_track <= 0) return 2352; // PAD
	if(!mounted()) return 2352; // PAD
	if(is_iso) return 2352;
	switch(toc_table[current_track].type) {
	case MODE_AUDIO:
		return 2352;
	case MODE_MODE1_2048:
		return 2048;
	case MODE_MODE1_2352:
	case MODE_MODE2_2352:
	case MODE_CDI_2352:
		return 2352;
	case MODE_MODE2_2336:
	case MODE_CDI_2336:
		return 2336;
	case MODE_CD_G:
		return 2448;
	default:
		break;
	}
	// OK?
	return 2352;
}

const int TOWNS_CDROM::logical_block_size()
{
	if(current_track <= 0) return 2352; // PAD
	if(!mounted()) return 2352; // PAD
	if(is_iso) return 2048;
	switch(toc_table[current_track].type) {
	case MODE_AUDIO:
		return 2352;
	case MODE_MODE1_2048:
	case MODE_MODE1_2352:
		return 2048;
	case MODE_MODE2_2336:
	case MODE_MODE2_2352:
	case MODE_CDI_2336:
	case MODE_CDI_2352:
		return 2336;
	case MODE_CD_G:
		return 2448;
	default:
		break;
	}
	// OK?
	return 2048;
}


void TOWNS_CDROM::event_callback(int event_id, int err)
{
	switch (event_id) {
	case EVENT_CDROM_DELAY_INTERRUPT_ON: // DELAY INTERRUPT ON
		write_mcuint_signals(0xffffffff);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDROM_DELAY_INTERRUPT_OFF: // DELAY INTERRUPT OFF
		write_mcuint_signals(0x0);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDROM_DELAY_READY: // CALL READY TO ACCEPT COMMAND WITH STATUS
		event_delay_ready = -1;
		mcu_ready = true;
		if(stat_reply_intr) {
			set_mcu_intr(true);
		}
		break;
	case EVENT_CDROM_READY_NOSTATUS: // CALL READY TO ACCEPT COMMAND WITHOUT STATUS
		event_delay_ready = -1;
		mcu_ready = true;
		//status_queue->clear();
		if(stat_reply_intr) {
			set_mcu_intr(true);
		}
		break;
	case EVENT_CDROM_READY_EOT:  // CALL END-OF-TRANSFER FROM CDC.
		event_delay_ready = -1;
		mcu_ready = true;
//		if(stat_reply_intr) {
//			set_mcu_intr(true);
//		}
		if(req_status) {
			mcu_intr = true;
			if(!(mcu_intr_mask) && (stat_reply_intr)) {
				write_mcuint_signals(0xffffffff);
			}
		} else {
			mcu_intr = false;
		}
//		set_mcu_intr(true);
		break;
	case EVENT_CDROM_READY_CDDAREPLY: // READY TO ACCEPT A COMMAND FROM CDC.
		event_delay_ready = -1;
		mcu_ready = true;
		mcu_intr = true;
		if(stat_reply_intr) {
			set_mcu_intr(true);
		}
		break;
	case EVENT_CDDA_DELAY_PLAY: // DELAY STARTING TO PLAY CDDA
		status_seek = false;
		if(cdda_status != CDDA_PLAYING) {
			set_cdda_status(CDDA_PLAYING);
		}
		event_cdda_delay_play = -1;
		databuffer->clear();
		send_mcu_ready(); // OK? 20230127 K.O
		if(prefetch_audio_sectors(1) < 1) {
			set_cdda_status(CDDA_OFF);
			set_subq();
			access = false;
			return;
		}
		break;
	case EVENT_CDDA:  // READ A PAIR OF CDDA SAMPLE
		read_a_cdda_sample();
		return;
		break;
	case EVENT_CDDA_DELAY_STOP:  // DELAY STOP
		event_cdda_delay_stop = -1;
		stop_cdda_from_cmd();
		break;
	case EVENT_CDROM_RESTORE: // Restore to LBA 0.
		// Seek0
		event_seek = -1;
		status_seek = true;
		if(next_seek_lba <= 2) {
			next_seek_lba = 2;
		}
		{
//			if(!(seek_relative_frame_in_image(0))) {
//				current_track = get_track(0);
//			}
			double usec = get_seek_time(next_seek_lba);
			if(usec < 10.0) usec = 10.0;
			usec *= 2.0;
			register_event(this,
						   EVENT_CDROM_SEEK,
						   usec, false, &event_seek);
		}
//		if(!(req_status)) {
//			mcu_ready = true;
//			mcu_intr = true;
//			status_queue->clear();
//			if(!(mcu_intr_mask)) {
//				write_mcuint_signals(0xffffffff);
//			}
//		}
		break;
	case EVENT_CDROM_SEEK: // SEEK TO TARGET LBA (only for command 00h)
		event_seek = -1;
//		stat_reply_intr = true;
//		if(!(seek_relative_frame_in_image(next_seek_lba))) {
//			current_track = get_track(next_seek_lba);
//		}
		status_seek = false;
		if(req_status) {
			status_accept(1, 0x00, 0x00);
		}
		if((cdda_status != CDDA_OFF) && (mounted())) {
			if((current_track >= 0) && (current_track < track_num)
			   && (toc_table[current_track].is_audio)) { // OK?

				set_cdda_status(CDDA_OFF);
			}
			set_subq();
		}
		if(!(req_status)) {
			mcu_ready = true;
			mcu_intr = true;
//			status_queue->clear();
			if(!(mcu_intr_mask)) {
				write_mcuint_signals(0xffffffff);
			}
		}

		break;
	case EVENT_CDROM_TIMEOUT:  // CDC TIMEOUT (mostly READ BUFFER OVERRUN)
		status_seek = false;
		event_time_out = -1;
		set_status_immediate(req_status, 0, TOWNS_CD_STATUS_CMD_ABEND, TOWNS_CD_ABEND_RETRY, 0x00, 0x00);
		cdrom_debug_log(_T("READ TIME OUT"));
		break;

	case EVENT_CDROM_SEEK_COMPLETED: // SEEK TO TARGET LBA FOR READING (A) SECTOR(S)
		event_seek_completed = -1;
		status_seek = false;
		//read_pos = 0;
		clear_event(this, event_next_sector);
		clear_event(this, event_time_out);
		// ToDo: Prefetch 20201116
#if 0
//		if((databuffer->left() < logical_block_size()) && (read_length > 0)) {

		if(!(databuffer->empty()) && (read_length > 0)) {
			register_event(this, EVENT_CDROM_SEEK_COMPLETED,
						   (1.0e6 / ((double)transfer_speed * 150.0e3)) *
						   2.0,
						   false,
						   &event_seek_completed);
			break; // EXIT
		}
#endif
		if(read_length > 0) {
			mcu_ready = false;
			bool stat = false;
//			cdrom_debug_log(_T("READ DATA SIZE=%d BUFFER COUNT=%d"), logical_block_size(), databuffer->count());
			/// Below has already resolved! Issue of DMAC.20201114 K.O
			// Note: Still error with reading D000h at TownsOS v1.1L30.
			// Maybe data has changed to 1Fh from 8Eh.
			/// 20200926 K.O
			stat = read_buffer(1);
			if((stat)) {
//				if((event_drq < 0) && (dma_transfer_phase)) {
//					register_event_by_clock(this, EVENT_CDROM_DRQ, 2, true, &event_drq);
//				}
				register_event(this, EVENT_CDROM_NEXT_SECTOR,
							   (1.0e6 / ((double)transfer_speed * 150.0e3)) *
							   ((double)(physical_block_size())) *
//							   0.5,
							   1.0, // OK?
							   false, &event_next_sector);
			}
			//register_event(this, EVENT_CDROM_TIMEOUT, 1000.0e3, false, &event_time_out);
		}// else {
		//	status_read_done(false);
		//}
		break;
	case EVENT_CDROM_NEXT_SECTOR:  // READY TO READ (A) NEXT SECTOR(S).
		event_next_sector = -1;
		clear_event(this, event_seek_completed);
		// BIOS FDDFCh(0FC0h:01FCh)-
		if(pio_transfer) {
			set_status(true, 0, TOWNS_CD_STATUS_CMD_ABEND, 0x00, 0x00, 0x00); // OK?
			set_dma_intr(true);
		} else {
			status_data_ready(true);
			if((event_drq < 0) && (dma_transfer_phase)) {
				register_event_by_clock(this, EVENT_CDROM_DRQ, 2, true, &event_drq);
			}
		}
		// ToDo: Prefetch
		status_seek = true;
		//status_seek = false;
		register_event(this, EVENT_CDROM_SEEK_COMPLETED,
					   (1.0e6 / ((double)transfer_speed * 150.0e3)) *
					   ((double)(physical_block_size())) *
					   0.1,
//					   1.0, // OK?
//					   5.0e3,
					   false,
					   &event_seek_completed);
		break;
	case EVENT_CDROM_DRQ:  // DRQ EVENT
		// ToDo: Buffer OVERFLOW at PIO mode.
		if(dma_transfer_phase) {
			if(!(databuffer->empty())) {
				write_signals(&outputs_drq, 0xffffffff);
			} else if(read_length <= 0){ // READ_LENGTH <= 0 && databuffer->empty)()
				clear_event(this, event_drq);
			}
			// Retry DRQ if buffert empty and remains sectors.
		}
		break;
	case EVENT_CDROM_EOT:  // END OF TRANSFER EVENT
		event_eot = -1;
		clear_event(this, event_time_out);
		if(dma_transfer_phase) {
			do_dma_eot(false);
		}
		break;
	default:
		// ToDo: Another events.
		//SCSI_DEV::event_callback(event_id, err);
		break;
	}
}

int TOWNS_CDROM::read_sectors_image(int sectors, uint32_t& transferred_bytes)
{

	cdimage_buffer_t tmpbuf;
	size_t _read_size;		//!< read size from image file.
	size_t _transfer_size;	//!< transfer size to data buffer.
	size_t _offset = sizeof(cd_data_head_t); //!< Data offset from head of sector.

	uint8_t* startp;
	uint8_t* datap;

	transferred_bytes = 0;

	switch(read_mode) {
	case CDROM_READ_MODE1:
		_transfer_size = 2048;
		if(is_iso) {
			_read_size = 2048;
			startp = &(tmpbuf.mode1.data[0]);
		} else {
			_read_size = 2352;
			startp = (uint8_t*)(&tmpbuf);
		}
		datap = &(tmpbuf.mode1.data[0]);
		break;
	case CDROM_READ_MODE2:
		_transfer_size = 2336;
		if(is_iso) {
			_read_size = 2336;
			startp = &(tmpbuf.mode2.data[0]);
		} else {
			_read_size = 2352;
			startp = (uint8_t*)(&tmpbuf);
		}
		datap = &(tmpbuf.mode2.data[0]);
		break;
	case CDROM_READ_RAW:
	case CDROM_READ_AUDIO:
		_offset = 0;
		_read_size = 2352;
		_transfer_size = 2352;
		startp = (uint8_t*)(&tmpbuf);
		datap = (uint8_t*)(&tmpbuf);
		break;
	default:
		// ToDo: Implement for unexpected type.
		return -1; // None sectors.
		break;
	}
	int seccount = 0;
	while(sectors > 0) {
		//cdrom_debug_log(_T("TRY TO READ SECTOR:LBA=%d"), read_sector);
		memset(&tmpbuf, 0x00, sizeof(tmpbuf));
		int _trk = check_cdda_track_boundary(read_sector);
		if(_trk <= 0) { // END
			status_illegal_lba(0, 0x00, 0x00, 0x00);
			return seccount;
		}
		if(_trk != current_track) { // END
			get_track_by_track_num(_trk);
		}
		if(!(seek_relative_frame_in_image(read_sector))) {
			status_illegal_lba(0, 0x00, 0x00, 0x00);
			return seccount;
		}
		// Phase 1: Check whether buffer remains.
		// Phase 2: Read data from image.
		if(!(fio_img->IsOpened())) {
			status_illegal_lba(0, 0x00, 0x00, 0x00); //<! OK? Maybe read error.
			return seccount;
		}
		// Read data from Image, maybe includes (or doed not include) header and footer.
		if(fio_img->Fread(startp, _read_size, 1) != 1) {
			status_illegal_lba(0, 0x00, 0x00, 0x00);
			return seccount;
		}
		// ToDo: Make pseudo header for ISO images.
		// ToDo: Read or Make sub-channel field.

		// Phase 3: Transfer main data to buffers.
		__UNLIKELY_IF(read_length < _transfer_size) {
			// Buffer over flow
			status_illegal_lba(0, 0x00, 0x00, 0x00);
			return seccount;
		}
		uint32_t tbytes_bak = transferred_bytes;
		int rlen_bak = read_length;

		for(int i = 0; i < _transfer_size; i++) {
			__UNLIKELY_IF(databuffer->full()) {
				// Buffer over flow
				// ToDo: Re-Scheduling data transfer.
				access = false;
				transferred_bytes = tbytes_bak;
				read_length = rlen_bak;
				int _dummy;
				for(int j = 0; j < i; j++) {
					_dummy = databuffer->read();
				}
				if(!(seek_relative_frame_in_image(read_sector))) {
					// Try to restore before sector.
					status_illegal_lba(0, 0x00, 0x00, 0x00);
				}
				return seccount;
			}
			write_a_byte(datap[i]);
			transferred_bytes++;
			read_length--;
		}
		read_sector++; // ToDo: Check boundary.
		seccount++;
		sectors--;
	}
	return seccount; // OK.
}

bool TOWNS_CDROM::read_buffer(int sectors)
{
	if(status_media_changed_or_not_ready(false)) {
		// @note ToDo: Make status interrupted.
		access = false;
		return false;
	}

	uint32_t nbytes; // transferred_bytes
	int rsectors = read_sectors_image(sectors, nbytes);
	if(rsectors > 0) {
		access = true;
	} else {
		access = false;
	}
	if(rsectors != sectors) {
		// ToDo: Partly reading error.
		return false;
	} else {
		return true;
	}
}

int TOWNS_CDROM::dequeue_audio_data(pair16_t& left, pair16_t& right)
{
	uint8_t data[4];
	if(databuffer->count() < 4) return 0;

	for(int i = 0; i < 4; i++) {
		data[i] = (uint8_t)(databuffer->read() & 0xff);
	}
	__UNLIKELY_IF(config.swap_audio_byteorder[0]) {
		left.read_2bytes_be_from(&(data[0]));
		right.read_2bytes_be_from(&(data[2]));
	} else {
		left.read_2bytes_le_from(&(data[0]));
		right.read_2bytes_le_from(&(data[2]));
	}
	return 4;
}

void TOWNS_CDROM::read_a_cdda_sample()
{
	__UNLIKELY_IF(event_cdda_delay_play > -1) {
		// Post process
		if(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
			set_subq();
//			return; // WAIT for SEEK COMPLETED
		}
		return; // WAIT for SEEK COMPLETED
	}
	// read 16bit 2ch samples in the cd-da buffer, called 44100 times/sec
	pair16_t sample_l, sample_r;
	int rlen = dequeue_audio_data(sample_l, sample_r);
	__UNLIKELY_IF(rlen != 4) {
		// May recover buffer.
		if(rlen <= 0) return; // None dequeued.
		// ToDo: Recover queue.
		return; //
	}
	cdda_sample_l = sample_l.sw;
	cdda_sample_r = sample_r.sw;
	cdda_buffer_ptr = cdda_buffer_ptr + 4;
	bool force_seek = false;
	__UNLIKELY_IF((cdda_buffer_ptr % 2352) == 0) {
		// one frame finished
		cdda_playing_frame++;
		cdda_buffer_ptr = 0;

		__UNLIKELY_IF(cdda_playing_frame >= cdda_end_frame) {
			if(cdda_repeat_count < 0) {
				// Infinity Loop (from Towns Linux v2.2.26)
				cdda_playing_frame = cdda_start_frame;
				read_sector = cdda_start_frame;
				force_seek = true;
			} else if(cdda_repeat_count == 0) {
				set_cdda_status(CDDA_ENDED);
				set_subq();
				access = false;
				return;
			} else {
				cdda_playing_frame = cdda_start_frame;
				read_sector = cdda_start_frame;
				force_seek = true;
				cdda_repeat_count--;
				if(cdda_repeat_count == 0) {
					set_cdda_status(CDDA_ENDED);
					set_subq();
					access = false;
					return;
				}
			}
		}
/*		int _ntrk =	_ntrk = check_cdda_track_boundary(read_sector);
		__UNLIKELY_IF((_ntrk != current_track) && (_ntrk < track_num) && (_ntrk > 0)) {
			current_track = get_track(read_sector);
			seek_relative_frame_in_image(read_sector);
		} else */if(force_seek) {
			seek_relative_frame_in_image(read_sector);
		}
		//cdda_playing_frame = read_sector;
		if(databuffer->count() <= sizeof(cd_audio_sector_t)) {
			// Kick prefetch
			//(event_next_sector < 0) {
				// TMP: prefetch 2 sectors
				prefetch_audio_sectors(2);
			//
		}
	}
	// Post process
	__UNLIKELY_IF(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
		set_subq();
	}
}

// -1 = End of sector.
int TOWNS_CDROM::prefetch_audio_sectors(int sectors)
{
	if(sectors < 1) {
		return -1;
	}
	if(status_media_changed_or_not_ready(false)) {
		// @note ToDo: Make status interrupted.
		return -1;
	}
	uint8_t tmpbuf[sectors * 2448 + 8];
	if((read_sector >= cdda_end_frame) || (read_sector >= max_logical_block)) {
		return 0; // OK?
	}
	if((current_track >= track_num) || (current_track <= 0)) {
		return 0;
	}
	if((read_sector + sectors) > cdda_end_frame) {
		sectors = cdda_end_frame - read_sector;
	}
	if((read_sector + sectors) > max_logical_block) {
		sectors = max_logical_block - read_sector;
	}
	__UNLIKELY_IF(databuffer->left() < read_length) {
		int tl = databuffer->left();
		if(tl < sizeof(cd_audio_sector_t)) return 0; // Pending
		int _s = tl / sizeof(cd_audio_sector_t);
		if(_s < 1) return 0; // Pending
		sectors = _s;
	}

	read_length = sectors * sizeof(cd_audio_sector_t); // Hack.
	read_mode = CDROM_READ_AUDIO;

	uint32_t nbytes; // transferred_bytes
	int _sectors = 0;
	while(sectors > 0) {
		int rsectors = read_sectors_image(sectors, nbytes);
		cdrom_debug_log(_T("READ AUDIO SECTOR(s) LBA=%d SECTORS=%d -> %d bytes=%d"),
						read_sector, sectors, rsectors, nbytes);
		if(rsectors <= 0) {
			// Error
			//set_cdda_status(CDDA_OFF);
			set_subq();
			access = false;
			return _sectors; // OK?
		}
		sectors -= rsectors;
		_sectors += rsectors;
	}
	return _sectors;
}

void TOWNS_CDROM::set_cdda_status(uint8_t status)
{
	if(status == CDDA_PLAYING) {
		if(mix_loop_num == 0) {
			if(event_cdda < 0) {
				register_event(this, EVENT_CDDA, 1.0e6 / 44100.0, true, &event_cdda);
			}
		}
		if(cdda_status != CDDA_PLAYING) {
			//// Notify to release bus.
			write_mcuint_signals(0x0);
			if((cdda_status == CDDA_OFF) || (cdda_status == CDDA_ENDED)) {
				//get_track_by_track_num(current_track); // Re-Play
				cdda_playing_frame = cdda_start_frame;
				current_track = get_track(cdda_playing_frame);
				seek_relative_frame_in_image(cdda_playing_frame);
				cdda_buffer_ptr = 0;
				access = false;
			} else if(cdda_status == CDDA_PAUSED) {
				// Unpause
				//access = true;
			}
			touch_sound();
			set_realtime_render(this, true);
			const _TCHAR *pp = get_cdda_status_name(cdda_status);
			cdrom_debug_log(_T("Play CDDA from %s.\n"), pp);
		}
		cdda_stopped = false;
	} else {
		clear_event(this, event_cdda);
		if((cdda_status == CDDA_PLAYING) || (cdda_status == CDDA_ENDED)) {
			// Notify to release bus.
			write_mcuint_signals(0x0);
			if(status == CDDA_OFF) {
				databuffer->clear();
				cdda_buffer_ptr = 0;
				read_sector = 0;
				cdda_repeat_count = -1; // OK?
				get_track_by_track_num(0);
				cdda_stopped = true;
			}
			touch_sound();
			set_realtime_render(this, false);
			const _TCHAR *sp = get_cdda_status_name(status);
			const _TCHAR *pp = get_cdda_status_name(cdda_status);
			cdrom_debug_log(_T("Change CDDA status: %s->%s"), pp, sp);
		}
	}
	cdda_status = status;
}

void TOWNS_CDROM::reset_device()
{
	set_cdda_status(CDDA_OFF);
	memset(subq_buffer, 0x00, sizeof(subq_buffer));
	memset(param_queue, 0x00, sizeof(param_queue));
	memset(w_regs, 0x00, sizeof(w_regs));

	status_seek = false;
	media_changed = false;
	cdrom_prefetch = false;

	param_ptr = 0;
	latest_command = 0x00;
	prev_command = 0x00;
	command_received = false;

	subq_overrun = false;
	stat_track = current_track;
	next_seek_lba = 0;

	extra_status = 0;
	data_reg.w = 0x0000;

	cdda_repeat_count = -1;
	touch_sound();
	clear_event(this, event_cdda);
	clear_event(this, event_cdda_delay_play);
	clear_event(this, event_cdda_delay_stop);
	clear_event(this, event_delay_interrupt);
	clear_event(this, event_seek_completed);
	clear_event(this, event_drq);
	clear_event(this, event_next_sector);
	clear_event(this, event_seek);
	clear_event(this, event_delay_ready);
	clear_event(this, event_time_out);
	clear_event(this, event_eot);

	read_length = 0;

	databuffer->clear();
	status_queue->clear();

	if(is_cue) {
		if(fio_img->IsOpened()) {
			fio_img->Fclose();
		}
	} else {
		if(fio_img->IsOpened()) {
			fio_img->Fseek(0, FILEIO_SEEK_SET);
		}
	}
	current_track = 0;
	cdda_start_frame = 0;
	cdda_end_frame = 150;
	cdda_playing_frame = 0;

	read_sector = 0;

	// Note: Reorder sequence.
	// 20220127 K.O
	mcu_intr = false;
	dma_intr = false;
	dma_transfer_phase = false;
	pio_transfer_phase = false;
	mcu_ready = true;

	req_status = false;
	mcu_intr_mask = false;
	dma_intr_mask = false;

	dma_transfer = false;
	pio_transfer = false;

	stat_reply_intr = false;
	cdda_stopped = false;
	write_signals(&outputs_drq, 0);
	write_signals(&outputs_mcuint, 0x0);
	// Will Implement
}

bool TOWNS_CDROM::is_device_ready()
{
	return mounted();
}


void TOWNS_CDROM::get_track_by_track_num(int track)
{
	if((track <= 0) || (track >= track_num)) {
		if(is_cue) {
			if(fio_img->IsOpened()) fio_img->Fclose();
		}
		current_track = 0;
		return;
	}
	if(is_cue) {
		// ToDo: Apply audio with some codecs.
		if((current_track != track) || !(fio_img->IsOpened())){
			if(fio_img->IsOpened()) {
				fio_img->Fclose();
			}
			cdrom_debug_log(_T("LOAD TRK #%02d from %s\n"), track, track_data_path[track - 1]);

			if((track > 0) && (track < 100) && (track < track_num)) {
				if((strlen(track_data_path[track - 1]) <= 0) ||
				   !(fio_img->Fopen(track_data_path[track - 1], FILEIO_READ_BINARY))) {
					track = 0;
				}
			} else {
				track = 0;
			}
		}
	}
	current_track = track;
}

// Detect only track num.
int TOWNS_CDROM::get_track_noop(uint32_t lba)
{
	int track = 0;
	for(int i = 0; i < track_num; i++) {
		if(lba >= toc_table[i].index0) {
			track = i;
		} else {
			break;
		}
	}
	return track;
}

int TOWNS_CDROM::get_track(uint32_t lba)
{
	int track = 0;
	track = get_track_noop(lba);
	get_track_by_track_num(track);
	return track;
}

double TOWNS_CDROM::get_seek_time(uint32_t lba)
{
	if(fio_img->IsOpened()) {
		uint32_t cur_position = (uint32_t)fio_img->Ftell();
		int distance;
		if(is_cue) {
			int track = 0;
			for(int i = 0; i < track_num; i++) {
				if(lba >= toc_table[i].index0) {
					track = i;
				} else {
					break;
				}
			}
			distance = abs((int)lba - (int)(cur_position / ((is_iso) ? 2048 : physical_block_size()) + toc_table[current_track].index0));
			if(track != current_track) {
				current_track = get_track(lba);
			}
		} else {
			distance = abs((int)lba - (int)(cur_position / ((is_iso) ? 2048 : physical_block_size())));
		}
		if(distance < 0) {
			distance = 0; // Seek penalty.
		}
		double _seek = (double)distance / 333000.0 ; // 333000: sectors in media
		_seek = 400.0e3 *  _seek;
		return _seek;
	} else {
		return 400000; // 400msec
	}
}

uint32_t TOWNS_CDROM::lba_to_msf(uint32_t lba)
{
	uint8_t m = lba / (60 * 75);
	lba -= m * (60 * 75);
	uint8_t s = lba / 75;
	uint8_t f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) | ((s / 10) << 12) | ((s % 10) << 8) | ((f / 10) << 4) | ((f % 10) << 0);
}

uint32_t TOWNS_CDROM::get_image_cur_position()
{
	uint32_t frame = 0;
	int track = current_track;
	if(is_device_ready()) {
		if(fio_img->IsOpened()) {
			uint32_t cur_position = (uint32_t)(fio_img->Ftell());
			frame = (cur_position / ((is_iso) ? 2048 : physical_block_size()));
			if(is_cue) {
				frame = frame + toc_table[track].lba_offset;
			}
		} else {
			frame = 0;
		}
	}
	return frame;
}
uint32_t TOWNS_CDROM::lba_to_msf_alt(uint32_t lba)
{
	uint32_t ret = 0;
	ret |= ((lba / (60 * 75)) & 0xff) << 16;
	ret |= (((lba / 75) % 60) & 0xff) <<  8;
	ret |= ((lba % 75)        & 0xff) <<  0;
	return ret;
}

void TOWNS_CDROM::unpause_cdda_from_cmd()
{
	if(status_media_changed_or_not_ready(false)) {
		return;
	}

	set_cdda_status(CDDA_PLAYING);
	/*!
	 * @note This may solve halt incident of Kyukyoku Tiger, but something are wrong.
	 * @note 20201113 K.O
	 */
	set_subq();
	set_status_cddareply(true, 1, TOWNS_CD_STATUS_ACCEPT, 0x00, 0x00, 0x00);
	return;
}

void TOWNS_CDROM::stop_cdda_from_cmd()
{
	if(status_media_changed_or_not_ready(false)) {
		set_cdda_status(CDDA_PAUSED);
		return;
	}
	/// @note Even make additional status even stop.Workaround for RANCEIII.
	/// @note - 20201110 K.O
	set_cdda_status(CDDA_ENDED);
	set_subq();
	status_accept(1, 0x00, 0x00);
	return;
}

void TOWNS_CDROM::pause_cdda_from_cmd()
{
	if(status_media_changed_or_not_ready(false)) {
		return;
	}
	set_cdda_status(CDDA_PAUSED);
   /*!
	 * @note This may solve halt incident of Kyukyoku Tiger, but something are wrong.
	 * @note 20201113 K.O
	 */
   set_subq();
   set_status_cddareply(true, 1, TOWNS_CD_STATUS_ACCEPT, 0x01, 0x00, 0x00);
   return;
}

bool TOWNS_CDROM::seek_relative_frame_in_image(uint32_t frame_no)
{
	int phys_size = (is_iso) ? 2048 : physical_block_size();
	if(frame_no >= toc_table[current_track].lba_offset) {
		if(fio_img->IsOpened()) {
			if(fio_img->Fseek(
				   (frame_no - toc_table[current_track].lba_offset) * phys_size,
				   FILEIO_SEEK_SET) != 0) {
				return false;
			}
			return true;
		}
	}
	return false;
}


int TOWNS_CDROM::check_cdda_track_boundary(uint32_t frame_no)
{
	if((frame_no >= toc_table[current_track + 1].index0) ||
	   (frame_no < toc_table[current_track].index0)) {
		int trk = get_track_noop(frame_no);
		if((trk <= 0) || (trk >= track_num)) {
			cdrom_debug_log(_T("CDDA: BEYOND OF TRACK BOUNDARY AND TO THE END, FRAME=%d"), frame_no);
			return track_num;
		}
		cdrom_debug_log(_T("CDDA: BEYOND OF TRACK BOUNDARY, FRAME=%d"), frame_no);
		//if(frame_no < toc_table[current_track].index0) {
		//	frame_no = toc_table[current_track].index0;
		//}
		return trk;
	}
	return current_track;
}

void TOWNS_CDROM::play_cdda_from_cmd()
{
	uint8_t m_start      = param_queue[0];
	uint8_t s_start      = param_queue[1];
	uint8_t f_start      = param_queue[2];
	uint8_t m_end        = param_queue[3];
	uint8_t s_end        = param_queue[4];
	uint8_t f_end        = param_queue[5];
	uint8_t is_repeat    = param_queue[6]; // From Towns Linux v1.1/towns_cd.c
	uint8_t repeat_count = param_queue[7];
	cdda_repeat_count = -1;
	if(status_media_changed_or_not_ready(false)) {
		return;
	}
	uint32_t start_tmp = FROM_BCD(f_start) + (FROM_BCD(s_start) + FROM_BCD(m_start) * 60) * 75;
	uint32_t end_tmp = FROM_BCD(f_end)   + (FROM_BCD(s_end)   + FROM_BCD(m_end) * 60) * 75;
	/*!
	 * @note Workaround for command SPAM, i.e. Puyo Puyo.
	 * @note 20201115 K.O
	 */
	{
		int track;
		track = get_track_noop(start_tmp);
		if(start_tmp >= toc_table[track].pregap) {
			start_tmp -= toc_table[track].pregap;
		}
		if(start_tmp < toc_table[track].index1) {
			start_tmp = toc_table[track].index1; // don't play pregap
		} else if(start_tmp >= max_logical_block) {
			start_tmp = max_logical_block - 1;
		}
		/* if(end_tmp >= toc_table[track + 1].index0) {
			end_tmp = toc_table[track + 1].index0 - 1;
		} else */if(end_tmp >= max_logical_block) {
			end_tmp = max_logical_block - 1;
		} else if(end_tmp == 0) { //! Workaround of Puyo Puyo 20201116 K.O
			end_tmp = toc_table[track + 1].index0 - 1;
		}
		if(cdda_status == CDDA_PLAYING) {
			int track_tmp_s = get_track_noop(start_tmp);
			int track_tmp_e = get_track_noop(end_tmp);
			if((start_tmp == cdda_start_frame) && (end_tmp == cdda_end_frame)) {
				// Dummy
				set_status_cddareply(true, 1, TOWNS_CD_STATUS_ACCEPT, 0, 0x00, 0x00);
				return;
			}
			// Workaround for Puyo Puyo, Interval stage.
			if((track_tmp_s != current_track) || (track_tmp_e != current_track)) {
				if(cdda_status == CDDA_PLAYING) {
					set_cdda_status(CDDA_OFF);
				}
				if(!(toc_table[track_tmp_s].is_audio)) {
					// If target LBA is not CDDA, reject command.
					set_status_cddareply(true, 1, TOWNS_CD_STATUS_ACCEPT, 0, 0x00, 0x00);
					return;
				}
			}
		}
		cdda_start_frame = start_tmp;
		cdda_end_frame   = end_tmp;

		track = get_track(cdda_start_frame);
		if(!(toc_table[track].is_audio)) {
			if(cdda_status == CDDA_PLAYING) {
				set_cdda_status(CDDA_OFF);
			}
			status_hardware_error(false); // OK?
//			status_not_accept(0, 0x0, 0x00, 0x00);
			return;
		}
		if(is_repeat == 1) {
			cdda_repeat_count = -1;
		} else {
			// Maybe is_repeat == 9
			cdda_repeat_count = repeat_count;
			cdda_repeat_count++;
		}
		track = current_track;
		cdda_playing_frame = cdda_start_frame;
		read_sector = cdda_start_frame;
		status_seek = false;
		seek_relative_frame_in_image(cdda_playing_frame);
		cdrom_debug_log(_T("PLAY_CDROM TRACK=%d START=%02X:%02X:%02X(%d) END=%02X:%02X:%02X(%d) IS_REPEAT=%d REPEAT_COUNT=%d"),
					  track,
					  m_start, s_start, f_start, cdda_start_frame,
					  m_end, s_end, f_end, cdda_end_frame,
					  is_repeat, repeat_count);
		double usec = get_seek_time(cdda_playing_frame);
		if(usec < 10.0) usec = 10.0;

		set_cdda_status(CDDA_PLAYING);
		force_register_event(this, EVENT_CDDA_DELAY_PLAY, usec, false, event_cdda_delay_play);
	}
	set_subq(); // First
	/*!
	 * @note This may solve halt incident of Kyukyoku Tiger, but something are wrong.
	 * @note 20201113 K.O
	 */
    set_status_cddareply(true, 1, TOWNS_CD_STATUS_ACCEPT, 0, 0x00, 0x00);
//	status_accept(1, 0x00, 0x00);
}

void TOWNS_CDROM::make_bitslice_subc_q(uint8_t *data, int bitwidth)
{
	int nbit = 0;
	if(bitwidth > (sizeof(subq_buffer) / sizeof(SUBC_t))) {
		bitwidth = (sizeof(subq_buffer) / sizeof(SUBC_t));
	} else if(bitwidth < 0) {
		bitwidth = 0;
	}
	// Set Q field
	// Q: IS set SYNC CODE?.
	for(int bp = 0; bp < bitwidth; bp++) {
		subq_buffer[bp].bit.Q =
			((data[nbit >> 3] & (1 << (7 - (nbit & 7)))) != 0) ? 1 : 0;
		nbit++;
	}
}

void TOWNS_CDROM::set_subq(void)
{
	if(is_device_ready()) {
		// create track info
		int track = current_track;
		uint32_t frame;
		uint32_t msf_abs;
		uint32_t msf_rel;
		// ToDo: Process when Foo.sub (for Foo.cue or Foo.ccd), this may be sub-channel data.
		if(toc_table[track].is_audio) { // OK? (or force ERROR) 20181120 K.O
			frame = ((cdda_status == CDDA_OFF) || (cdda_status == CDDA_ENDED)) ? cdda_start_frame : cdda_playing_frame;
		} else { // Data
			frame = get_image_cur_position();
		}
		if(frame > toc_table[track].index1) {
			msf_rel = lba_to_msf_alt(frame - toc_table[track].index1);
		} else {
			msf_rel = lba_to_msf_alt(0);
		}
		if(subq_bitptr < subq_bitwidth) {
			subq_overrun = true;
		}
		msf_abs = lba_to_msf_alt(frame + 75 * 2); // OK?
		uint8_t subq_bytes[12] = {0};
		// ToDo: POINT=0xA0-0xA2
		{
			subq_bytes[0] = ((toc_table[track].is_audio) ? 0x40 : 0x00) | 0x01;			// (CNT << 4) | ADR
			subq_bytes[1] = 0x00;							// TNO
			subq_bytes[2] = TO_BCD(track + 1);				// POINT(Track)
			subq_bytes[3] = TO_BCD((msf_abs >> 16) & 0xff);	// M (absolute)
			subq_bytes[4] = TO_BCD((msf_abs >> 8) & 0xff);	// S (absolute)
			subq_bytes[5] = TO_BCD((msf_abs >> 0) & 0xff);	// F (absolute)
			subq_bytes[6] = 0x00;							// Zero
			subq_bytes[7] = TO_BCD((msf_rel >> 16) & 0xff);	// M (relative)
			subq_bytes[8] = TO_BCD((msf_rel >> 8) & 0xff);	// S (relative)
			subq_bytes[9] = TO_BCD((msf_rel >> 0) & 0xff);	// F (relative)
		}
		// Post Process1: Calculate CRC16
		uint16_t crc = calc_subc_crc16(subq_bytes, 10, 0xffff); // OK?
		subq_bytes[10] = (crc >> 8) & 0xff;
		subq_bytes[11] = (crc >> 0) & 0xff;
		// Post process 2: push bytes to Bit slice
		memset(subq_buffer, 0x00, sizeof(subq_buffer));
		// ToDo: P field;
		make_bitslice_subc_q(subq_bytes, 96);
		// ToDo: R-W field (a.k.a. CD-TEXT/CD-G);
		subq_bitptr = 0;
		subq_bitwidth = 96;
	} else {
		// OK?
		if(subq_bitptr < subq_bitwidth) {
			subq_overrun = true; // !:Not Needed?
		}
		memset(subq_buffer, 0x00, sizeof(subq_buffer));
		uint8_t subq_bytes[12] = {0};
		uint16_t crc = calc_subc_crc16(subq_bytes, 10, 0xffff); // OK?
		subq_bytes[10] = (crc >> 8) & 0xff;
		subq_bytes[11] = (crc >> 0) & 0xff;
		// Post process 2: push bytes to Bit slice
		memset(subq_buffer, 0x00, sizeof(subq_buffer));
		// ToDo: P field;
		make_bitslice_subc_q(subq_bytes, 96);
		// ToDo: R-W field (a.k.a. CD-TEXT/CD-G);
		subq_bitptr = 0;
		subq_bitwidth = 96;
	}
//	mcu_ready = true;
	return;
}

uint8_t TOWNS_CDROM::get_subq_status()
{
	uint8_t val = 0x00;
	val = val | ((subq_bitwidth > 0) ? 0x01 : 0x00);
	val = val | ((subq_overrun) ? 0x02 : 0x00);
	return val;
}

uint8_t TOWNS_CDROM::read_subq()
{
	uint8_t val = 0x00;
	if(subq_bitptr < subq_bitwidth) {
		val = subq_buffer[subq_bitptr].byte;
		subq_bitptr++;
	}
	return val;
}

int TOWNS_CDROM::get_frames_from_msf(const char *s)
{
	const char *ptr = s;
	int frames[3] = {0};
	int index = 0;

	while(1) {
		if(*ptr >= '0' && *ptr <= '9') {
			frames[index] = frames[index] * 10 + (*ptr - '0');
		} else if(*ptr == ':') {
			if(++index == 3) {
				// abnormal data
				break;
			}
		} else if(*ptr == '\r' || *ptr == '\n' || *ptr == '\0') {
			// end of line
			break;
		}
		ptr++;
	}
	return (frames[0] * 60 + frames[1]) * 75 + frames[2]; // 75frames/sec
}

int64_t TOWNS_CDROM::hexatoi(const char *s)
{
	const char *ptr = s;
	int64_t value = 0;

	while(1) {
		if(*ptr >= '0' && *ptr <= '9') {
			value = value * 16 + (*ptr - '0');
		} else if(*ptr >= 'a' && *ptr <= 'f') {
			value = value * 16 + (*ptr - 'a' + 10);
		} else if(*ptr >= 'A' && *ptr <= 'F') {
			value = value * 16 + (*ptr - 'A' + 10);
		} else if(*ptr == '\r' || *ptr == '\n' || *ptr == '\0') {
			break;
		}
		ptr++;
	}
	return value;
}

#include <string>
#include <map>

int64_t TOWNS_CDROM::string_to_numeric(std::string s)
{
	int64_t _ret = 0;
	if(!(s.empty())) {
		std::string head;
		try {
			head = s.substr(0, 2);
		} catch (std::out_of_range &e) {
			head = s;
		}
		std::transform(head.begin(), head.end(), head.begin(),
					   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

		if(head == "0X") {
			std::string __val;
			try {
				__val = s.substr(2, s.size());
			} catch (std::out_of_range &e) {
				__val = "";
			}
			_ret = hexatoi(__val.c_str());
		} else {
			_ret = atoll(s.c_str());
		}

	}
	return _ret;
}

void TOWNS_CDROM::open(const _TCHAR* file_path)
{
	media_changed = true;
	open_from_cmd(file_path);
//	set_status(true, 0, TOWNS_CD_STATUS_DOOR_CLOSE_DONE, 0x00, 0x00, 0x00);
//	status_accept(0, 0x09, 0x00, 0x00); // Disc changed
}

void TOWNS_CDROM::open_from_cmd(const _TCHAR* file_path)
{
	_TCHAR img_file_path[_MAX_PATH] = {0};
	memset(img_file_path_bak, 0x00, sizeof(img_file_path_bak));

	close_from_cmd();
	access = false;

	if(check_file_extension(file_path, _T(".cue"))) {
		is_cue = false;
		current_track = 0;
		if(open_cue_file(file_path)) {
			strncpy(img_file_path_bak, file_path, _MAX_PATH - 1);
		}
	} else if(check_file_extension(file_path, _T(".iso"))) {
		is_cue = false;
		current_track = 0;
		if(open_iso_file(file_path)) {
			strncpy(img_file_path, file_path, _MAX_PATH - 1);
			strncpy(img_file_path_bak, file_path, _MAX_PATH - 1);
		}
		if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
			is_cue = false;
			current_track = 0;
			is_iso = true;
		}
	} else if(check_file_extension(file_path, _T(".ccd"))) {
		// get image file name
		if(open_ccd_file(file_path, img_file_path)) {
			strncpy(img_file_path_bak, img_file_path, _MAX_PATH - 1);
//			strncpy(img_file_path_bak, file_path, _MAX_PATH - 1);
		}
	}

	if(mounted() /*&& (__SCSI_DEBUG_LOG)*/) {
		for(int i = 1; i < track_num + 1; i++) {
			uint32_t idx0_msf = lba_to_msf(toc_table[i].index0);
			uint32_t idx1_msf = lba_to_msf(toc_table[i].index1);
			uint32_t pgap_msf = lba_to_msf(toc_table[i].pregap);
			this->cdrom_debug_log(_T("Track%02d: Index0=%02x:%02x:%02x Index1=%02x:%02x:%02x PreGap=%02x:%02x:%02x\n"), i,
			(idx0_msf >> 16) & 0xff, (idx0_msf >> 8) & 0xff, idx0_msf & 0xff,
			(idx1_msf >> 16) & 0xff, (idx1_msf >> 8) & 0xff, idx1_msf & 0xff,
			(pgap_msf >> 16) & 0xff, (pgap_msf >> 8) & 0xff, pgap_msf & 0xff);
		}
	}
}

void TOWNS_CDROM::close()
{
//	status_accept(0, 0x09, 0x00, 0x00, 0x00); // Disc changed
//	req_status = true;
//	set_status(true, 0, TOWNS_CD_STATUS_DOOR_OPEN_DONE, 0x00, 0x00, 0x00);
//	media_changed = true;
	close_from_cmd();
}

void TOWNS_CDROM::close_from_cmd()
{
	if(fio_img->IsOpened()) {
		fio_img->Fclose();
	}
	memset(toc_table, 0, sizeof(toc_table));
	memset(img_file_path_bak, 0x00, sizeof(img_file_path_bak));
	track_num = 0;
	is_cue = false;
	current_track = 0;
	set_cdda_status(CDDA_OFF);
	is_iso = false;
	read_mode = CDROM_READ_NONE;
}

bool TOWNS_CDROM::mounted()
{
	if(is_cue) return true;
	return fio_img->IsOpened();
}

bool TOWNS_CDROM::accessed()
{
	bool value = access;
	access = false;
	return value;
}

void TOWNS_CDROM::mix(int32_t* buffer, int cnt)
{
	if(cdda_status == CDDA_PLAYING) {
		if(mix_loop_num != 0) {
			int tmp_l = 0, tmp_r = 0;
			for(int i = 0; i < mix_loop_num; i++) {
				event_callback(EVENT_CDDA, 0);
				tmp_l += cdda_sample_l;
				tmp_r += cdda_sample_r;
			}
			cdda_sample_l = tmp_l / mix_loop_num;
			cdda_sample_r = tmp_r / mix_loop_num;
		}
//		int32_t val_l = apply_volume(apply_volume(cdda_sample_l, volume_m), volume_l);
//		int32_t val_r = apply_volume(apply_volume(cdda_sample_r, volume_m), volume_r);
		int32_t val_l = (mute_left) ? 0 : apply_volume(cdda_sample_l, volume_l);
		int32_t val_r = (mute_right) ? 0 : apply_volume(cdda_sample_r, volume_r);

		for(int i = 0; i < cnt; i++) {
			*buffer++ += val_l; // L
			*buffer++ += val_r; // R
		}
	}
}

void TOWNS_CDROM::set_volume(int ch, int decibel_l, int decibel_r)
{
	_decibel_l = decibel_l;
	_decibel_r = decibel_r;
	volume_l = decibel_to_volume(_decibel_l + 6.0);
	volume_r = decibel_to_volume(_decibel_r + 6.0);
}

void TOWNS_CDROM::get_volume(int ch, int& decibel_l, int& decibel_r)
{
	decibel_l = _decibel_l;
	decibel_r = _decibel_r;
//	decibel_l = volume_l;
//	decibel_r = volume_r;
}


uint32_t TOWNS_CDROM::read_io8w(uint32_t addr, int *wait)
{
	/*
	 * 04C0h : Master status
	 * 04C2h : CDC status
	 * 04C4h : DATA
	 * 04CCh : SUBQ CODE
	 * 04CDh : SUBQ STATUS
	 */
	*wait = 6; // Temporally.
	uint32_t val = 0xff;
	switch(addr & 0x0f) {
	case 0x00:
		val = 0x00;
		val = val | ((mcu_intr)					? 0x80 : 0x00);
		val = val | ((dma_intr)					? 0x40 : 0x00);
		val = val | ((pio_transfer_phase)		? 0x20 : 0x00);
		val = val | ((dma_transfer_phase)		? 0x10 : 0x00); // USING DMAC ch.3
		// ToDo: FIX to lack queueing status with some commands
		// 20230207 K.O
		val = val | (!(status_queue->empty())	? 0x02 : 0x00);
//		val = val | ((has_status)				? 0x02 : 0x00);
		val = val | ((mcu_ready)				? 0x01 : 0x00);
//		if((mcu_intr) || (dma_intr)) {
//			mcu_intr = false;
//			dma_intr = false;
//			write_signals(&outputs_mcuint, 0x00000000);
//			cdrom_debug_log(_T("FALL DOWN INTs@04C0h"));
//		}
		break;
	case 0x02:
		val = read_status();
		break;
	case 0x04:
		if((pio_transfer_phase) && (pio_transfer)) {
			if(databuffer->left() == logical_block_size()) {
				cdrom_debug_log(_T("PIO READ START FROM 04C4h"));
			}
			fetch_datareg_8();
			val = data_reg.b.l;
			if((read_length <= 0) && (databuffer->empty())) {
				pio_transfer_phase = false;
				mcu_ready = false;
				clear_event(this, event_next_sector);
				clear_event(this, event_seek_completed);
				clear_event(this, event_time_out);
				cdrom_debug_log(_T("EOT(PIO)"));
				status_read_done(false);
				if((stat_reply_intr) || !(dma_intr_mask)) {
					//if((stat_reply_intr) && !(dma_intr_mask)) {
					write_mcuint_signals(0xffffffff);
				}
			} else if(databuffer->empty()) {
				pio_transfer_phase = false;
				mcu_ready = false;
				clear_event(this, event_time_out);
				cdrom_debug_log(_T("NEXT(PIO)"));
				if((stat_reply_intr) || !(dma_intr_mask)) {
					//if((stat_reply_intr) && !(dma_intr_mask)) {
					write_mcuint_signals(0xffffffff);
				}
			}
		}
		break;
	case 0x0c: // Subq code
		val = read_subq();
		break;
	case 0x0d: // Subq status
		val = get_subq_status();
		break;
	}
	//cdrom_debug_log(_T("READ IO8: %04X %02X"), addr, val);
	return val;
}
/*
uint32_t TOWNS_CDROM::read_io16w(uint32_t addr, int *wait)
{
	pair32_t v;
	v.d = 0xffffffff;
	v.b.l = read_io8w(addr & 0xfffe, wait);
	return v.d;
}
*/
void TOWNS_CDROM::write_io8w(uint32_t addr, uint32_t data, int *wait)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
	//cdrom_debug_log(_T("WRITE IO8: %04X %02X"), addr, data);
	*wait = 6; // temporally
	w_regs[addr & 0x0f] = data;
	switch(addr & 0x0f) {
	case 0x00: // Master control register
		// Note: Sync with TSUGARU.
		// 20220127 K.O
		//cdrom_debug_log(_T("PORT 04C0h <- %02X"), data);
		if((data & 0x80) != 0) {
			mcu_intr = false;
			if(!(dma_intr)) {
				write_mcuint_signals(0); // Reset interrupt request to PIC.
			}
		}
		if((data & 0x40) != 0) {
			dma_intr = false;
			if(!(mcu_intr)) {
				write_mcuint_signals(0); // Reset interrupt request to PIC.
			}
		}
		if((data & 0x04) != 0) {
			cdrom_debug_log(_T("RESET FROM CMDREG: 04C0h"));
			reset_device();
		}
		mcu_intr_mask = ((data & 0x02) == 0) ? true : false;
		dma_intr_mask = ((data & 0x01) == 0) ? true : false;
		break;
	case 0x02: // Command
		//cdrom_debug_log(_T("PORT 04C2h <- %02X"), data);
		// Note: Sync with TSUGARU.
		// 20220127 K.O
		//if(mcu_intr) {
		//	data = data | (latest_command & 0x60); // For FRACTAL ENGINE DEMO (from TSUGARU).
		//}
		if(mcu_ready) {
			//dma_transfer_phase = false;
			//pio_transfer_phase = false;
			mcu_ready = false;
			prev_command = latest_command;
			command_received = true;
			latest_command = data;
			execute_command(data);
		}
		break;
	case 0x04: // Param
		if(param_ptr >= 8) {
			// Rotate queue
			for(int xx = 0; xx < 7; xx++) {
				param_queue[xx] = param_queue[xx + 1];
			}
			param_ptr = 7;
		}
		param_queue[param_ptr] = data;
		param_ptr++;
		break;
	case 0x06:
		dma_transfer = ((data & 0x10) != 0) ? true : false;
		pio_transfer = ((data & 0x08) != 0) ? true : false;
		if(dma_transfer) {
			if(!(dma_transfer_phase)) {
				dma_transfer_phase = true;
				if((event_drq < 0) && !(databuffer->empty())) {
					register_event_by_clock(this, EVENT_CDROM_DRQ, 2, true, &event_drq);
				}
			}
		} else if(pio_transfer) {
			if(!(pio_transfer_phase)) {
				pio_transfer_phase = true;
			}
		}
//		cdrom_debug_log(_T("SET TRANSFER MODE to %02X"), data);
//		cdrom_debug_log(_T("SET TRANSFER MODE to %02X"), data);
//		}
		break;
	}
#if 0
	if((command_received) && (param_ptr >= 8)) {
		_TCHAR param_str[128] = {0};
		_TCHAR tstr[8];
		for(int i = 0; i < 8; i++) {
			memset(tstr, 0x00, sizeof(tstr));
			my_stprintf_s(tstr, 7, _T("%02X "), param_queue[i]);
			my_tcscat_s(param_str, 127, tstr);
		}
		_TCHAR _pcstr[128] = {0};
		if(d_cpu == NULL) {
			my_stprintf_s(_pcstr, sizeof(_pcstr) - 1, _T("CMD=%02X"),  latest_command);
		} else {
			//cdrom_debug_log(_T("CMD=%02X from PC=%08X"), data, d_cpu->get_pc());
			my_stprintf_s(_pcstr, sizeof(_pcstr) - 1, _T("CMD=%02X from PC=%08X"), latest_command, d_cpu->get_pc());
		}
		cdrom_debug_log(_T("REGISTER COMMAND %s PARAM=%s"), _pcstr, param_str);
		execute_command(latest_command);
	}
#endif
}
/*
void TOWNS_CDROM::write_io16w(uint32_t addr, uint32_t data, int *wait)
{
	pair32_t v;
	v.d = data;
	write_io8w(addr & 0xfffe, v.b.l, wait);
}
*/
void TOWNS_CDROM::write_debug_data8(uint32_t addr, uint32_t data)
{
	databuffer->write_not_push(addr % max_fifo_length, data & 0xff);
}

uint32_t TOWNS_CDROM::read_debug_data8(uint32_t addr)
{
	return databuffer->read_not_remove(addr % max_fifo_length) & 0xff;
}


bool TOWNS_CDROM::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}


bool TOWNS_CDROM::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == NULL) return false;
	_TCHAR regs[256] = {0};
	for(int i = 0; i < 16; i += 2) {
		_TCHAR tmps[16] = {0};
		my_stprintf_s(tmps, 16, _T("%02X "), w_regs[i]);
		my_tcscat_s(regs, sizeof(regs) / sizeof(_TCHAR), tmps);
	}
	_TCHAR stat[256] = {0};
	for(int i = 0; i < 4; i++) {
		_TCHAR tmps[16] = {0};
		my_stprintf_s(tmps, 16, _T("%02X "), status_queue->read_not_remove(i) & 0xff);
		my_tcscat_s(stat, sizeof(regs) / sizeof(_TCHAR), tmps);
	}
	_TCHAR param[256] = {0};
	for(int i = 0; i < 8; i++) {
		_TCHAR tmps[16] = {0};
		my_stprintf_s(tmps, 16, _T("%02X "), param_queue[i]);
		my_tcscat_s(param, sizeof(param) / sizeof(_TCHAR), tmps);
	}
	bool is_audio = false;
	bool in_track = ((current_track > 0) && (current_track < track_num));
	uint32_t index0 = 0;
	uint32_t index1 = 0;
	uint32_t lba_size = 0;
	uint32_t lba_offset = 0;
	uint32_t pregap = 150;
	if(in_track) {
		is_audio = toc_table[current_track].is_audio;
		index0 = toc_table[current_track].index0;
		index1 = toc_table[current_track].index1;
		pregap = toc_table[current_track].pregap;
		lba_size = toc_table[current_track].lba_size;
		lba_offset = toc_table[current_track].lba_offset;
	}
	_TCHAR moreinfo[512] = {0};
	if(is_audio) {
		const _TCHAR* playstatus = get_cdda_status_name(cdda_status);
		my_stprintf_s(moreinfo, sizeof(moreinfo) - 1,
					  _T("TYPE=AUDIO: \n")
					  _T("STATUS=%s LOOP COUNT=%d \n")
					  _T("START=%d END=%d CURRENT_PLAY=%d NEXT_READ=%d \n")
					  ,
					  playstatus, cdda_repeat_count,
					  cdda_start_frame, cdda_end_frame,
					  cdda_playing_frame, read_sector
			);
	} else {
		my_stprintf_s(moreinfo, sizeof(moreinfo) - 1,
					  _T("TYPE=DATA: TRANSFER MODE=%s %s\n")
					  , (pio_transfer) ? _T("PIO") : _T("   ")
					  , (dma_transfer) ? _T("DMA") : _T("   ")
			);
	}
	const _TCHAR* cmdname = get_command_name_from_command(latest_command);
	const _TCHAR* prev_cmdname = get_command_name_from_command(prev_command);
	my_stprintf_s(buffer, buffer_len,
				  _T("\n%s")
				  _T("MCU INT=%s DMA INT=%s TRANSFER PHASE:%s %s HAS_STATUS=%s MCU=%s\n")
				  _T("TRACK=%d INDEX0=%d INDEX1=%d PREGAP=%d LBA_OFFSET=%d LBA_SIZE=%d\n")
				  _T("LBA=%d READ LENGTH=%d DATA QUEUE=%d\n")
				  _T("CMD=%02X(%s) PARAM=%s PTR=%d PREV_COMMAND=%02X(%s)\n")
				  _T("EXTRA STATUS=%d STATUS COUNT=%d QUEUE_VALUE=%s\n")
				  _T("REGS RAW VALUES=%s\n")
				  , moreinfo
				  , (mcu_intr) ? _T("ON ") : _T("OFF"), (dma_intr) ? _T("ON ") : _T("OFF")
				  , (pio_transfer_phase) ? _T("PIO") : _T("   ")
				  , (dma_transfer_phase) ? _T("DMA") : _T("   ")
				  , (!(status_queue->empty())) ? _T("ON ") : _T("OFF"), (mcu_ready) ? _T("ON ") : _T("OFF")
				  , current_track, index0, index1, pregap, lba_size, lba_offset
				  , read_sector, read_length, databuffer->count()
				  , latest_command, cmdname,  param, param_ptr, prev_command, prev_cmdname
				  , extra_status, status_queue->count(), stat
				  , regs
		);
	return true;
}


/*
 * Note: 20200428 K.O: DO NOT USE STATE SAVE, STILL don't implement completely yet.
 */
#define STATE_VERSION	24

bool TOWNS_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(databuffer->process_state((void *)state_fio, loading))) {
		return false;
	}
	if(!(status_queue->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateArray(w_regs, sizeof(w_regs), 1);
	for(int i = 0; i < (sizeof(subq_buffer) / sizeof(SUBC_t)); i++) {
		state_fio->StateValue(subq_buffer[i].byte);
	}
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);
	state_fio->StateValue(max_fifo_length);
	state_fio->StateValue(fifo_length);

	state_fio->StateValue(data_reg);
	state_fio->StateValue(req_status);
	state_fio->StateValue(stat_reply_intr);
	state_fio->StateValue(latest_command);

	state_fio->StateValue(mcu_intr);
	state_fio->StateValue(dma_intr);
	state_fio->StateValue(pio_transfer);
	state_fio->StateValue(dma_transfer);
	state_fio->StateValue(pio_transfer_phase);
	state_fio->StateValue(dma_transfer_phase);
	state_fio->StateValue(mcu_ready);
	state_fio->StateValue(mcu_intr_mask);
	state_fio->StateValue(dma_intr_mask);
	state_fio->StateValue(transfer_speed);
	state_fio->StateValue(read_length);

	state_fio->StateValue(next_seek_lba);

	state_fio->StateValue(param_ptr);
	state_fio->StateArray(param_queue, sizeof(param_queue), 1);
	state_fio->StateValue(extra_status);

	state_fio->StateValue(subq_bitwidth);
	state_fio->StateValue(subq_bitptr);
	state_fio->StateValue(subq_overrun);
	state_fio->StateValue(stat_track);
	state_fio->StateValue(media_changed);
	state_fio->StateValue(command_received);


	state_fio->StateValue(force_logging);
	// SCSI_CDROM
	uint32_t offset = 0;
	state_fio->StateValue(read_sector);
	state_fio->StateValue(mix_loop_num);

	state_fio->StateArray(img_file_path_bak, sizeof(img_file_path_bak), 1);
	state_fio->StateValue(is_cue);
	state_fio->StateValue(current_track);
	state_fio->StateValue(track_num);
	state_fio->StateValue(status_seek);

	state_fio->StateValue(cdrom_prefetch);

	state_fio->StateValue(cdda_start_frame);
	state_fio->StateValue(cdda_end_frame);
	state_fio->StateValue(cdda_playing_frame);
	state_fio->StateValue(cdda_status);
	state_fio->StateValue(cdda_repeat_count);
	state_fio->StateValue(cdda_interrupt);
	state_fio->StateValue(cdda_buffer_ptr);
	state_fio->StateValue(cdda_sample_l);
	state_fio->StateValue(cdda_sample_r);
	state_fio->StateValue(cdda_stopped);

	state_fio->StateValue(_decibel_l);
	state_fio->StateValue(_decibel_r);

	state_fio->StateValue(mute_left);
	state_fio->StateValue(mute_right);

	if(loading) {
		offset = state_fio->FgetUint32_LE();
	} else {
		if(fio_img->IsOpened()) {
			offset = fio_img->Ftell();
		}
		state_fio->FputUint32_LE(offset);
	}
	// ToDo: Re-Open Image.20181118 K.O
 	// post process
	if(loading) {
		if(fio_img->IsOpened()) {
			close_from_cmd();
		}
		bool is_cue_bak = is_cue;
		int track_num_bak = track_num;
		if(strlen(img_file_path_bak) > 0) {
			open_from_cmd(img_file_path_bak);
		}
		if((is_cue_bak == is_cue) && (track_num_bak == track_num)) {
			if((current_track > 0) && (current_track < 100)) {
				get_track_by_track_num(current_track); // Re-Play
			}
			if(fio_img->IsOpened()) {
				fio_img->Fseek(offset, FILEIO_SEEK_SET);
			}
		} else {
			close_from_cmd();
		}

		volume_l = decibel_to_volume(_decibel_l + 6.0);
		volume_r = decibel_to_volume(_decibel_r + 6.0);

 	}
	state_fio->StateValue(event_seek);
	state_fio->StateValue(event_cdda);
	state_fio->StateValue(event_cdda_delay_play);
	state_fio->StateValue(event_delay_interrupt);
	state_fio->StateValue(event_drq);
	state_fio->StateValue(event_next_sector);
	state_fio->StateValue(event_seek_completed);
	state_fio->StateValue(event_delay_ready);
	state_fio->StateValue(event_time_out);
	state_fio->StateValue(event_eot);

	// SCSI_DEV
 	return true;
}



}
