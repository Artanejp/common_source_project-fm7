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
#include "./towns_cdrom.h"
#include "../../fifo.h"
#include "../../fileio.h"
#include "../scsi_host.h"

// SAME AS SCSI_CDROM::
#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA						100
#define EVENT_CDDA_DELAY_PLAY			101
#define EVENT_CDROM_SEEK_SCSI			102
#define EVENT_CDROM_DELAY_INTERRUPT_ON	103
#define EVENT_CDROM_DELAY_INTERRUPT_OFF	104
#define EVENT_CDDA_DELAY_STOP			105

#define _SCSI_DEBUG_LOG
#define _CDROM_DEBUG_LOG

// Event must be larger than 116.

namespace FMTOWNS {

#undef __NOT_SCSI_CDROM
#if defined(__NOT_SCSI_CDROM)
	// Still not be real implement.
/*
 * Note: 20200411 K.O
 *
 * 1. MCU of CDC is driven by 8MHz clock maximum, 1.5us per insn (?).
 * 2. DATA QUEUE RAM may be 8KiB.
 * 3. CDC seems not to be SCSI, raw davice (?)
 *
 * *NOTE IN NOTE:
 *   - SECTOR SIZE OF DATA TRACK maybe 2048 bytes (excepts SUB TRACK and CRC).
 *     WHEN READ A SECTOR, THEN START DRQ.
 *   - ToDo: Will implement MODE2/2352. 
 *   - ALL OF SEQUENCES are EVENT-DRIVEN.
 *   - SUB MPU (MCU) INTERRUPT HAPPENED AT SOME ERRORs and COMPLETED ?
 *   - DMA END INTERRUPT HAPPENED AT COMPLETED TO TRANSTER ALL OF DATA.
 *   - MASTER STATUS REGISTER (04C0h:R) WILL SHOW SOME STATUS.
 *   - IF SET 04C2h:W:BIT5, WILL INTERRUPT AT SOME **END.
 *   - PARAMETER REGISTER(04C4h:W) HAS 8BYTES DEPTH FIFO,
 *     ALL OF COMMAND-PARAMETERS MUST HAVE 8 BYTES LONG.
 *   - STATUS REGISTER (04C2h:R) HAS 4 BYTES DEPTH FIFO,
 *     ALL OF STATUSS MUST HAVE 4 BYTES LONG.
 *   - 04C6h:W WILL SET DMA TRANSFER or PIO TRANSTER,
 *     DATA REGISTER (04C4h:R) WILL CONNECT TO DATA RAM,
 *     WHEN PIO TRANSFER.

https://www.wdic.org/w/TECH/サブコード
 *
 * REGISTERS:
 * 04C0h:W : MASTER CONTROL REGISTER.
 *         BIT7: SMIC      : IF WRITE '1', CLEAR SUB MCU IRQ.
 *         BIT6: DEIC      : IF WRITE '1', CLEAR DMA END IRQ.
 *         ...
 *         BIT2: SRST      : IF WRITE '1', RESET SUB MCU (MAY DISCARD SOME DATA).
 *         BIT1: SMIM      : IF WRITE '1', ALLOW SUB MCU IRQ, '0', NOT ALLOW SUB MCU IRQ. 
 *         BIT2: DEIM      : IF WRITE '1', ALLOW DMA END IRQ, '0', NOT ALLOW DMA END IRQ. 
 *
 * 04C2h:W : COMMAND REGISTER.
 *         BIT7: TYPE      : COMMAND TYPE; '1' IS AROUND PLAYING, '0' IS AROUND STATE.
 *         BIT6: IRQ       : IF WRITE '1', DO MCU INTERRUPT WHEN STATUS PHASE.
 *         BIT5: STATUS    : IF WRITE '1', REPLY STATUS FROM MCU, '0', NOT REPLY (excepts IRQ?) 
 *         BIT4-0: CMD     : COMMAND CODE.
 *
 * 04C4h:W : PARAMETER FIFO.DEPTH IS 8 BYTES.
 *
 * 04C6h:W : TRANSFER CONTROL REGISTER.
 *         BIT4: DTS       :  WRITE '1' IS DMA TRANSFER MODE, '0' IS PIO TRANSFER MODE.
 *         BIT3: STS       : IF PIO TRANSFER MODE AND WRITE TO '1', START TRANSFERRING.
 * 
 * 04C0h:R : MASTER STATUS REGISTER.
 *         BIT7: SIRQ      : IF '1', INTERRUPTS FROM SUB MCU.
 *         BIT6: DEI       : IF '1', INTERRUPTS FROM DMA BY ENDED TO TRANSFER.
 *         BIT5: STSF      : IF '1', END OF PIO TRANSFER.
 *         BIT4: DTSF      : IF '1', STILL IN DMA TRANSFER, '0' IS NOT.
 *         ...
 *         BIT1: SRQ       : IF '1', SUB MCU HAS STATUS CODES AFTER COMPLETED.
 *         BIT0: DRY       : IF '1', SUB MCU ACCEPTS ANY COMMAND, '0' IS NOT.
 * 
 * 04C2h:R : STATUS REGISTER.HAS 4 BYTES DEPTH FIFO.
 * 
 * 04C4h:R : DATA REGISTER.THIS IS NOT MEANFUL AT DMA TRANSFERRING.
 * 
 * 04CCh:R : CD SUBCODE REGISTER.
 *         BIT1: OVER RUN  : IF '1', BEFORE SUB CODE DATA IS NOT READ AT BEFORE CYCLE.
 *         BIT0: SUBC DATR : IF '1', SUB CODE HAS ANY DATA, SHOULD READ FROM 04CDh.
 *
 * 04CDh:R :CD SUB CODE REGISTER.
 *         BIT7: SUBC P-DATA : IF '0', IN GAP (NO SOUND)
 *         BIT6: SUBC Q-DATA : BIT SLICE OF SUBQ DATA (98bits=2bits+12bytes). i.e. TOC 
 *         BIT5: SUBC R-DATA : R-W IS 6bits data of,
 *         BIT4: SUBC S-DATA :   READ IN  : CD TEXT
 *         BIT3: SUBC T-DATA :   DATA     : CD-G OR CD-MIDI
 *         BIT2: SUBC U-DATA :   READ OUT : UNUSED.
 *         BIT1: SUBC V-DATA :
 *         BIT0: SUBC W-DATA :
 *
 * SEE, https://www.wdic.org/w/TECH/TOC .
 *       ONE SECTOR HAS ONE SUB FRAME, 98BYTES.
 * 
 * SUBQ IS MOSTLY TIMING:
 * BIT +0        : S0 BIT
 *     +1        : S1 BIT
 *     +2  - +5  : CNT : TYPE OF TRACK
 *     +6  - +9  : ADR : 1 = TIME / 2 = CATALOGUE DATA / 3 = ISR CODE
 *     +10 - +81 : DATA-Q : SEE *TOC_DATA
 *     +82 - +97 : CRC16 
 *
 * *TOC_DATA IN SUBQ: 
 * BYTE +0 : TNO          : MOSTLY 00
 *      +1 : POINT        : TRACK NUMBER or A0h - A2h
 *      +2 : MIN   (BCD)  : START ABS MIN of THIS TRACK
 *      +3 : SEC   (BCD)  : START ABS SEC of THIS TRACK
 *      +4 : FRAME (BCD)  : START ABS FRAME of THIS TRACK. 1 FRAME IS 1/75 sec.
 *      +5 : ZERO         : MOSTLY 00
 *      +6 : PMIN   (BCD) : START REL MIN of THIS TRACK
 *      +7 : PSEC   (BCD) : START REL SEC of THIS TRACK
 *      +8 : PFRAME (BCD) : START REL FRAME of THIS TRACK. 1 FRAME IS 1/75 sec.
 *
 * *POINT = A0h: FOR FIRST TRACK
 * BYTE +0 : TNO          : MOSTLY 00
 *      +1 : POINT        : A0h
 *      +2 : MIN   (BCD)  : START ABS MIN of FIRST TRACK
 *      +3 : SEC   (BCD)  : START ABS SEC of FIRST TRACK
 *      +4 : FRAME (BCD)  : START ABS FRAME of FIRST TRACK. 1 FRAME IS 1/75 sec.
 *      +5 : ZERO         : MOSTLY 00
 *      +6 : PMIN   (BCD) : FIRST TRACK NUMBER
 *      +7 : PSEC   (BCD) : DISC TYPE, MOSTLY 00
 *      +8 : PFRAME (BCD) : MOSTLY 00
 *      
 * *POINT = A1h: FOR LAST TRACK
 * BYTE +0 : TNO          : MOSTLY 00
 *      +1 : POINT        : A1h
 *      +2 : MIN   (BCD)  : START ABS MIN of LAST TRACK
 *      +3 : SEC   (BCD)  : START ABS SEC of LAST TRACK
 *      +4 : FRAME (BCD)  : START ABS FRAME of LAST TRACK. 1 FRAME IS 1/75 sec.
 *      +5 : ZERO         : MOSTLY 00
 *      +6 : PMIN   (BCD) : LAST TRACK NUMBER
 *      +7 : PSEC   (BCD) : MOSTLY 00
 *      +8 : PFRAME (BCD) : MOSTLY 00
 *
 * *POINT = A2h: FOR READOUT
 * BYTE +0 : TNO          : MOSTLY 00
 *      +1 : POINT        : A2h
 *      +2 : MIN   (BCD)  : START ABS MIN of READOUT
 *      +3 : SEC   (BCD)  : START ABS SEC of READOUT
 *      +4 : FRAME (BCD)  : START ABS FRAME of READOUT. 1 FRAME IS 1/75 sec.
 *      +5 : ZERO         : MOSTLY 00
 *      +6 : PMIN   (BCD) : START REL MIN of READOUT
 *      +7 : PSEC   (BCD) : START REL SEC of READOUT
 *      +8 : PFRAME (BCD) : START REL FRAME of READOUT
 *
 * *DMA DATA TRANSFER FLOW:
 * HOST                      CDC/DRIVE
 * POLL READY                [DO COMMAND]
 *            <------------- CMD READY
 * SEEK CMD   -------------> 
 *            <------------- ACCEPT_NORMAL
 *                           [CHECK READY]
 *                           IF ERROR THEN 
 *                              GOTO *NOTREADY_1
 *                           FI
 *                           [CHECK LBA]
 *                           IF ERROR THEN 
 *                              GOTO *SEEK_ERROR_1
 *                           FI
 *                           [WAIT FOR SEEK]
 *                           [COMPLETED]
 * [POLL READY]
 *            <-------------- SEND STATUS (SEEK COMPLETED)
 * [PREPARE DMA]
 * READ CMD(DMA,BLOCKS)-----> 
 *                            // ToDo: DATA READ FROM CDDA TRACK.
 *                            // THIS SEQUENCE STILL BE FOR DATA TRACK MAINLY.
 *            <-------------- ACCEPT_DMA
 * [DMA START]
 *                            **LOOP_READ: 
 *                            [READ A SECTOR]
 *                            [ENQUEUE 1 BLOCK to BUFFER]
 * [START DMA] <------------- [1St DRQ]
 * [DMAACK   ] ------------->
 *                            IF END OF A SECTOR THEN
 *             <------------    [NEXT-SECTOR INTERRUPT]
 *                            FI
 *                            IF REMAIN BLOCKS THEN
 *                                GOTO *LOOP_READ
 *                            ELSE [IF ALL OF SECTORS ARE READ THEN]
 *             <-------------    STATUS END OF TRANSFER
 *                            FI
 * POLL READY 
 * IF DMA COMPLETED
 * THEN REPLY EOT ----------> 
 *                <---------- SEND STATUS (OK)
 * IF ABORT THEN BUS ABORT ->
 *                <---------- SEND STATUS (OK)
 * **END.
 *
 *                           **NOTREADY_1:
 *                <---------- SEND STATUS (NOT READY)
 * **END.
 *
 *                           **SEEK_ERROR_1:
 *                <---------- SEND STATUS (SEEK ERROR)
 * **END.
 *
 * *CDDA PLAYING FLOW:
 * HOST                      CDC/DRIVE
 * POLL READY                [DO COMMAND]
 *            <------------- CMD READY
 * SEEK CMD   -------------> 
 *            <------------- ACCEPT_NORMAL
 *                           **LOOP_CDDA:
 *                           [CHECK READY]
 *                           IF ERROR 
 *                             GOTO *NOTREADY_1
 *                           FI
 *                           [CHECK LBA]
 *                           IF ERROR THEN
 *                              GOTO *SEEK_ERROR_1
 *                           FI
 *                           [WAIT FOR SEEK]
 *                           [COMPLETED]
 * [POLL READY]
 *            <-------------- SEND STATUS (SEEK COMPLETED)
 * CMD PLAY CDDA ------------> 
 *                            IF TRACK IS DATA THEN 
 *                                GOTO *ERROR_DATA_TRACK_PLAY
 *                            FI
 *                            [WAIT FOR START PLAYING]
 * [POLL READY]
 *            <-------------- SEND STATUS (PLAY STARTED)
 * [POLL READY]
 * [ENQUEUE ANOTHER CMDs]
 * [POLL READY]
 *                            AT END OF TRACK,
 *           <---------------    SEND STATUS (END OF TRACK)
 *                               IF NOT LOOP THEN 
 *                                  GOTO *COMPLETED_CDDA;
 *                               ELSE 
 *                                  SEEK TO HEAD OF TRACK ;
 *                                  GOTO *LOOP_CDDA ;
 *                               FI
 *                            END AT
 *                            **COMPLETED_CDDA:
 *           <--------------- SEND STATUS (CDDA STOPPED)
 * **END. 
  *                            **ERROR_DATA_TRACK_PLAY:
 *           <--------------- SEND STATUS (NOT CDDA TRACK)
 * **END.
 *
 * *CDDA PAUSING FLOW:
 * HOST                       CDC/DRIVE
 * [POLL READY]
 * PAUSE CMD(ON) ----------->
 * [POLL READY]
 *                            IF TRACK IS DATA THEN 
 *                               GOTO *ERROR_DATA_TRACK_PAUSE1
 *                            FI
 *                            IF ALREADY PAUSED THEN
 *               <----------    SEND STATUS (ALREADY PAUSED)
 *                            ELSE IF STOPPED THEN
 *               <----------    SEND STATUS (STOPPED)
 *                            ELSE
 *               <----------    SEND STATUS (PAUSING SUCCEEDED)
 *                            FI
 *                            **ERROR_DATA_TRACK_PAUSE1:
 *               <----------   SEND STATUS (NOT CDDA TRACK)
 * **END.
 *
 * [POLL READY]
 * PAUSE CMD(OFF)---------->
 * [POLL READY]
 *                            IF TRACK IS DATA THEN 
 *                              GOTO *ERROR_DATA_TRACK_PAUSE2
 *                            FI
 *                            IF ALREADY PAUSED THEN
 *               <----------    SEND STATUS (UNPAUSING SUCCEEDED)
 *                            ELSE IF STOPPED THEN
 *               <----------    SEND STATUS (STOPPED)
 *                            ELSE
 *               <----------    SEND STATUS (ALREADY PAUSED)
 *                            FI
 *                            **ERROR_DATA_TRACK_PAUSE2:
 *               <----------   SEND STATUS (NOT CDDA TRACK)
 * **END.
 *      
 * *CDDA STOPPING FLOW:
 * ToDo: CD-DA AUDIO, Initialize etc.
 * STOP CMD      ---------->
 * [POLL READY]
 *                            IF TRACK IS DATA THEN
 *                              GOTO *ERROR_DATA_TRACK_STOP
 *                            FI
 *                            IF NOT READY THEN
 *               <----------    SEND STATUS (NOT READY)
 *                            ELSE IF STOPPED THEN
 *               <----------    SEND STATUS (ALREADY STOPPED)
 *                            ELSE
 *                              SEEK TO TRACK0, BLOCK0 (LBA0) ;
 *                            FI
 *               <----------    SEND STATUS (OK)
 *                            **ERROR_DATA_TRACK_STOP:
 *               <----------   SEND STATUS (NOT CDDA TRACK)
 * **END.
 *
 * // ToDo: Implement TOC, SUBC...
 * **SUBC SEQUENCE (TEMPORALLY):
 *      WHEN READING SECTOR OR PLAYING CD-DA:
 *           AT BEGIN OF SECTOR: 
 *              IF (SUBC DATA (98 BYTES) HAVEN'T READ COMPLETLY) AND (FLAG OF SUBC DATA IN) THEN
 *                 SET BIT1 OF 04CCh:R TO '1';
 *              ELSE
 *                 SET BIT1 OF 04CCh:R TO '0';
 *              FI
 *              RESET FLAG OF SUBC DATA IN;
 *              CALCURATE SUBQ FIELD ;
 *              SET SUBP FIELD TO '1' (FIXED) ;
 *              SET SUBR - SUBW TO '0' (TEMPORALLY, WILL IMPLEMENT CD-TEXT etc) ;
 *              SET FLAG OF SUBC DATA IN;
 *              SET BYTECOUNT TO 98;
 *              SET BIT0 OF 04CCH:R TO '1';
 *           END AT
 *      GOTO *END_OF_SUBC_1;
 *
 *      WHEN NOT READING SECTOR AND NOT PLAYING CD-DA:
 *              IF (SUBC DATA (98 BYTES) HAVEN'T READ COMPLETLY) AND (FLAG OF SUBC DATA IN) THEN
 *                 SET BIT1 OF 04CCh:R TO '1';
 *              ELSE
 *                 SET BIT1 OF 04CCh:R TO '0';
 *              FI
 *              RESET FLAG OF SUBC DATA IN;
 *              IF (LAST TRACK),
 *                 THEN CALCURATE SUBQ FIELD ;
 *              ELSEIF (SEEKED TO FIRST TRACK),
 *                 THEN CALCURATE SUBQ FIELD ;
 *              ELSE // (STOPPED),
 *                 THEN CLEAR SUBQ FIELD;
 *                 SET SUBP FIELD TO '0' (FIXED) ;
 *                 SET SUBR - SUBW TO '0' (TEMPORALLY, WILL IMPLEMENT CD-TEXT etc) ;
 *                 SET FLAG OF SUBC DATA IN;
 *                 SET BYTECOUNT TO 0;
 *                 SET BIT0 OF 04CCH:R TO '0';
 *                 GOTO *END_OF_SUBC_1;
 *              FI
 *              SET SUBP FIELD TO '0' (FIXED) ;
 *              SET SUBR - SUBW TO '0' (TEMPORALLY, WILL IMPLEMENT CD-TEXT etc) ;
 *              SET BYTECOUNT TO 98;
 *              SET FLAG OF SUBC DATA IN;
 *              SET BIT0 OF 04CCH:R TO '1';
 *  **END_OF_SUBC_1:
 *  **END.              
 *  
 *   WHEN (FLAG OF SUBC DATA IN) IS SET ;
 *        IF BYTECOUNT > 0 THEN
 *             WHEN 04CDh:R has READ,
 *                 BYTECOUNT--;
 *                 IF BYTECOUNT <= 0 THEN
 *                    RESET FLAG OF SUBC DATA IN;
 *                    CLEAR 04CDh:R TO 0
 *                    SET BIT1 OF 04CCh:R TO '0'
 *                    SET BIT0 OF 04CCh:R TO '0'
 *                 ELSE
 *                    SET NEXT BITS TO 04CDh:R
 *                    SET BIT1 OF 04CCh:R TO '0'
 *                    SET BIT0 OF 04CCh:R TO '1'
 *                 FI
 *              END WHEN
 *        FI
 *   END WHEN
 * 
 */

void TOWNS_CDROM::initialize()
{
	subq_buffer = new FIFO(32); // OK?
	subq_overrun = false;
	stat_track = 0;
	
//   	SCSI_DEV::initialize();
	// ToDo: MasterDevice::initialize()
	fio_img = new FILEIO();
	__CDROM_DEBUG_LOG = osd->check_feature(_T("_CDROM_DEBUG_LOG"));
	
	if(44100 % emu->get_sound_rate() == 0) {
		mix_loop_num = 44100 / emu->get_sound_rate();
	} else {
		mix_loop_num = 0;
	}
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	cdda_status = CDDA_OFF;
	is_cue = false;
	current_track = 0;
	read_sectors = 0;
	for(int i = 0; i < 99; i++) {
		memset(track_data_path[i], 0x00, _MAX_PATH * sizeof(_TCHAR));
	}
}

void SCSI_CDROM::out_debug_log(const _TCHAR *format, ...)
{
	if(!(__CDROM_DEBUG_LOG) && !(_OUT_DEBUG_LOG)) return;
	va_list args;
	_TCHAR _tmps[4096] = {0};
	_TCHAR _domain[256] = {0};
	my_sprintf_s(_domain, sizeof(_domain) / sizeof(_TCHAR), _T("[SCSI_CDROM:ID=%d]"), scsi_id);
	va_start(args, format);
	vsnprintf(_tmps, sizeof(_tmps) / sizeof(_TCHAR), format, args);
	va_end(args);
	DEVICE::out_debug_log(_T("%s %s"), _domain, _tmps);
}

void SCSI_CDROM::release()
{
	if(subq_buffer != NULL) {
		subq_buffer->release();
		delete subq_buffer;
		subq_buffer = NULL;
	}
	
	if(fio_img->IsOpened()) {
		fio_img->Fclose();
	}
	delete fio_img;
	// ToDo: release from master device
//	SCSI_DEV::release();
}

void TOWNS_CDROM::reset()
{
	subq_buffer->clear();
	subq_overrun = false;
	stat_track = current_track;
	out_debug_log("RESET");
	touch_sound();
	if(event_delay_interrupt != -1) cancel_event(this, event_delay_interrupt);
	if(event_cdda_delay_play != -1) cancel_event(this, event_cdda_delay_play);
	if(event_cdda != -1) cancel_event(this, event_cdda);
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	// ToDo: Implement master device.
//	SCSI_DEV::reset();

	read_mode = false;
	set_cdda_status(CDDA_OFF);
	read_sectors = 0;
	// Q: Does not seek to track 0? 20181118 K.O
	//current_track = 0;
//	SCSI_CDROM::reset();
}


void TOWNS_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool _b = ((data & mask) != 0);
	
	switch(id) {
	case SIG_SCSI_CDROM_CDDA_STOP:
		if(cdda_status != CDDA_OFF) {
			if(_b) set_cdda_status(CDDA_OFF);
		}
		break;
	case SIG_SCSI_CDROM_CDDA_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			if(_b) set_cdda_status(CDDA_PLAYING);
		}
		break;
	case SIG_SCSI_CDROM_CDDA_PAUSE:
		if(cdda_status != CDDA_PAUSED) {
			if(_b) set_cdda_status(CDDA_PAUSED);
		}
		break;
	case SIG_TOWNS_CDROM_SET_TRACK:
		if(((data < 100) && (data >= 0)) || (data == 0xaa)) {
			stat_track = data;
		}
		break;
	default:
//		SCSI_DEV::write_signal(id, data, mask);
		// ToDo: Implement master devices.
		break;
	}

}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	switch(id) {
	case SIG_SCSI_CDROM_PLAYING:
		return (cdda_status == CDDA_PLAYING && cdda_interrupt) ? 0xffffffff : 0;
		break;
	case SIG_SCSI_CDROM_SAMPLE_L:
		return (uint32_t)abs(cdda_sample_l);
		break;
	case SIG_SCSI_CDROM_SAMPLE_R:
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
			uint32_t lba = (uint32_t)index0;
			if(pregap > 0) lba = lba - pregap;
			if(lba < 150) lba = 150;
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
			if(pregap > 0) lba = lba - pregap;
			if(lba < 150) lba = 150;
			uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
			return msf;
		}
		break;
	case SIG_TOWNS_CDROM_RELATIVE_MSF:
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			if(cdda_playing_frame <= cdda_start_frame) {
				return 0;
			}
			uint32_t msf;
			if(cdda_playing_frame >= cdda_end_frame) {
				if(cdda_repeat) {
					return 0;
				} else {
					msf = lba_to_msf(cdda_end_frame - cdda_start_frame);
					return msf;
				}
			}
			msf = lba_to_msf(cdda_playing_frame - cdda_start_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position);
				return msf;
			}
			return 0;
		}
		break;
	case SIG_TOWNS_CDROM_ABSOLUTE_MSF:
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			uint32_t msf;
			msf = lba_to_msf(cdda_playing_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position + toc_table[current_track].lba_offset);
				return msf;
			}
			return 0;
		}
		break;
	case SIG_TOWNS_CDROM_GET_ADR:
		{
			int trk = stat_track;
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
		break;
	default:
		// ToDo: Implement master DEV
		//return SCSI_DEV::read_signal(id);
		break;
	}
	return 0; // END TRAM
}

void TOWNS_CDROM::event_callback(int event_id, int err)
{
	switch (event_id) {
	case EVENT_CDROM_DELAY_INTERRUPT_ON:
		write_signals(&outputs_done, 0xffffffff);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDROM_DELAY_INTERRUPT_OFF:
		write_signals(&outputs_done, 0x00000000);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDDA_DELAY_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			set_cdda_status(CDDA_PLAYING);
		}
		event_cdda_delay_play = -1;
		break;
	case EVENT_CDROM_SEEK_SCSI:
		seek_time = 10.0;
		event_cdda_delay_play = -1;
		// WILL Implement MASTER DEVICE.
//		SCSI_DEV::start_command();
		break;
	case EVENT_CDDA:
		if(event_cdda_delay_play > -1) {
			// Post process
			if(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
				set_subq();
			}
			return; // WAIT for SEEK COMPLETED
		}
		// read 16bit 2ch samples in the cd-da buffer, called 44100 times/sec
		
		cdda_sample_l = (int)(int16_t)(cdda_buffer[cdda_buffer_ptr + 0] + cdda_buffer[cdda_buffer_ptr + 1] * 0x100);
		cdda_sample_r = (int)(int16_t)(cdda_buffer[cdda_buffer_ptr + 2] + cdda_buffer[cdda_buffer_ptr + 3] * 0x100);
		// ToDo: CLEAR IRQ Line (for PCE)
		if((cdda_buffer_ptr += 4) % 2352 == 0) {
			// one frame finished
			cdda_playing_frame++;
			if((is_cue) && (cdda_playing_frame != cdda_end_frame)) {
				if((cdda_playing_frame >= toc_table[current_track + 1].index0) && (track_num > (current_track + 1))) {
					get_track_by_track_num(current_track + 1); // Note: Increment current track
					if(fio_img->IsOpened()) {
						//fio_img->Fseek(0, FILEIO_SEEK_SET);
						read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					} else {
						// Seek error (maybe end of disc)
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
					cdda_buffer_ptr = 0;
					access = false;
				}
			}
			if(cdda_playing_frame == cdda_end_frame) {
				out_debug_log(_T("Reaches to the end of track.(FRAME %d). START_FRAME=%d END_FRAME=%d REPEAT=%s INTERRUPT=%s\n"),
							  cdda_playing_frame, cdda_start_frame, cdda_end_frame, 
							  (cdda_repeat) ? _T("YES") : _T("NO"),
							  (cdda_interrupt) ? _T("YES") : _T("NO"));
				// reached to end frame
				if(cdda_repeat) {
					// reload buffer
					// Restart.
					if(is_cue) {
						int trk = get_track(cdda_start_frame);
						if(fio_img->IsOpened()) {
							fio_img->Fseek((cdda_start_frame - toc_table[trk].lba_offset) * 2352, FILEIO_SEEK_SET);
						}
//						fio_img->Fclose();
						//current_track = 0;
						//int trk = get_track(cdda_start_frame);
//						int trk = current_track;
//						fio_img->Fseek((cdda_start_frame - toc_table[trk].lba_offset) * 2352, FILEIO_SEEK_SET);
					} else {
						fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
					}
					if(fio_img->IsOpened()) {
						read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t) , array_length(cdda_buffer) / 2352);
					} else {
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
//					read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t) , array_length(cdda_buffer) / 2352);
					cdda_buffer_ptr = 0;
					cdda_playing_frame = cdda_start_frame;
					access = true;
				} else {
					// Stop
					if(event_cdda_delay_play >= 0) cancel_event(this, event_cdda_delay_play);
					memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					register_event(this, EVENT_CDDA_DELAY_STOP, 1000.0, false, &event_cdda_delay_play);

					//set_cdda_status(CDDA_OFF);
					//register_event(this, EVENT_CDDA_DELAY_STOP, 1000.0, false, &event_cdda_delay_play);
					access = false;
				}
			} else if((cdda_buffer_ptr % 2352) == 0) {
				// refresh buffer
				read_sectors--;
				if(read_sectors <= 0) {
					if(fio_img->IsOpened()) {
						read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					} else {
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
//					read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					cdda_buffer_ptr = 0;
					access = false;
				} else {
				}
			}
		}
		// Post process
		if(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
			set_subq();
		}
		return;
		break;
	case EVENT_CDDA_DELAY_STOP:
		if(cdda_interrupt) {
			write_signals(&outputs_done, 0xffffffff);
		}
		set_cdda_status(CDDA_OFF);
		event_cdda_delay_play = -1;
		break;			
	default:
		// ToDo: Another events.
		//SCSI_DEV::event_callback(event_id, err);
		break;
	}
}
void TOWNS_CDROM::set_cdda_status(uint8_t status)
{
	if(status == CDDA_PLAYING) {
		if(mix_loop_num == 0) {
			if(event_cdda == -1) {
				register_event(this, EVENT_CDDA, 1000000.0 / 44100.0, true, &event_cdda);
			}
		}
		if(cdda_status != CDDA_PLAYING) {
			//// Notify to release bus.
			write_signals(&outputs_done, 0x00000000);
			if(cdda_status == CDDA_OFF) {
				//get_track_by_track_num(current_track); // Re-Play
				//memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
				cdda_playing_frame = cdda_start_frame;
				get_track(cdda_playing_frame);
				cdda_buffer_ptr = 0;
				access = false;
			} else if(cdda_status == CDDA_PAUSED) {
				// Unpause
				access = true;
			}
			touch_sound();
			set_realtime_render(this, true);
			out_debug_log(_T("Play CDDA from %s.\n"), (cdda_status == CDDA_PAUSED) ? _T("PAUSED") : _T("STOPPED"));
		}
	} else {
		if(event_cdda != -1) {
			cancel_event(this, event_cdda);
			event_cdda = -1;
		}
		if(cdda_status == CDDA_PLAYING) {
			// Notify to release bus.
			write_signals(&outputs_done, 0x00000000);
			//if(event_delay_interrupt >= 0) cancel_event(this, event_delay_interrupt);
			//register_event(this, EVENT_CDROM_DELAY_INTERRUPT_OFF, 1.0e6 / (44100.0 * 2352), false, &event_delay_interrupt);
			if(status == CDDA_OFF) {
				memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
				cdda_buffer_ptr = 0;
				read_sectors = 0;
				cdda_repeat = false; // OK?
				//if(is_cue) {
				//	if(fio_img->IsOpened()) fio_img->Fclose();
				//}
				//current_track = 0;
			}
			touch_sound();
			set_realtime_render(this, false);
			out_debug_log(_T("%s playing CDDA.\n"), (status == CDDA_PAUSED) ? _T("PAUSE") : _T("STOP"));
		}
	}
	cdda_status = status;
}

void TOWNS_CDROM::reset_device()
{
	set_cdda_status(CDDA_OFF);
//	SCSI_DEV::reset_device();
	// Will Implement
}

bool TOWNS_CDROM::is_device_ready()
{
	return mounted();
}

int TOWNS_CDROM::get_command_length(int value)
{
	switch(value) {
	case TOWNS_CDROM_CDDA_PLAY:
		return 10;
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_STOP:
		return 4;
		break;
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdd:
	case 0xde:
		// ToDo: These commands are PCE's extend.Will implement.
		return 10;
		break;
	default:
		// ToDo: Will implement master command.
//		return SCSI_CDROM::get_command_length(value);
		break;
	}
	return 0;
}
void SCSI_CDROM::get_track_by_track_num(int track)
{
	if((track <= 0) || (track >= track_num)) {
		if(is_cue) {
			if(fio_img->IsOpened()) fio_img->Fclose();
		}
		//if(track <= 0) current_track = 0;
		//if(track >= track_num)current_track = track_num;
		current_track = 0;
		return;
	}
	if(is_cue) {
		// ToDo: Apply audio with some codecs.
		if((current_track != track) || !(fio_img->IsOpened())){
			if(fio_img->IsOpened()) {
				fio_img->Fclose();
			}
			out_debug_log(_T("LOAD TRK #%02d from %s\n"), track, track_data_path[track - 1]);
			
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
int SCSI_CDROM::get_track_noop(uint32_t lba)
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

int SCSI_CDROM::get_track(uint32_t lba)
{
	int track = 0;
	track = get_track_noop(lba);
	if(is_cue) {
		get_track_by_track_num(track);
	} else {
		current_track = track;
	}
	return track;
}

double SCSI_CDROM::get_seek_time(uint32_t lba)
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
			distance = abs((int)(lba * physical_block_size()) - (int)(cur_position + toc_table[current_track].lba_offset * physical_block_size()));
			if(track != current_track) {
				current_track = get_track(lba);
			}
		} else {
			distance = abs((int)(lba * physical_block_size()) - (int)cur_position);
		}
		double ratio = ((double)distance / 333000.0) / physical_block_size(); // 333000: sectors in media
		return max(10, (int)(400000 * 2 * ratio));
		//double ratio = (double)distance  / 150.0e3; // 150KB/sec sectors in media
		//return max(10, (int)(400000 * 2 * ratio));
	} else {
		return 400000; // 400msec
	}
}

uint32_t SCSI_CDROM::lba_to_msf(uint32_t lba)
{
	uint8_t m = lba / (60 * 75);
	lba -= m * (60 * 75);
	uint8_t s = lba / 75;
	uint8_t f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) | ((s / 10) << 12) | ((s % 10) << 8) | ((f / 10) << 4) | ((f % 10) << 0);
}

uint32_t SCSI_CDROM::lba_to_msf_alt(uint32_t lba)
{
	uint32_t ret = 0;
	ret |= ((lba / (60 * 75)) & 0xff) << 16;
	ret |= (((lba / 75) % 60) & 0xff) <<  8;
	ret |= ((lba % 75)        & 0xff) <<  0;
	return ret;
}


void TOWNS_CDROM::start_command()
{
	touch_sound();
	switch(command[0]) {
	case TOWNS_CDROM_CDDA_PLAY:
		play_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		pause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		unpause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_STOP:
		stop_cdda_from_cmd();
		break;
	case SCSI_CMD_TST_U_RDY:
	case SCSI_CMD_INQUIRY:
	case SCSI_CMD_REQ_SENSE:
	case SCSI_CMD_RD_DEFECT:
	case SCSI_CMD_RD_CAPAC:
	case SCSI_CMD_MODE_SEL6: // OK?
//	case SCSI_CMD_READ6:
//	case SCSI_CMD_READ10:
	case SCSI_CMD_READ12:
//		SCSI_CDROM::start_command();
		// ToDo: Implement READ12.
		break;
	case 0xff:
		// End of List
//		set_dat(SCSI_STATUS_CHKCOND);
		break;
	default:
		out_debug_log(_T("Command: Unknown %02X\n"), command[0]);
//		set_dat(SCSI_STATUS_GOOD);
//		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
}

	
// From MAME 0203's fmtowns.cpp .	
void TOWNS_CDROM::pause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PLAYING) {
		set_cdda_status(CDDA_PAUSED);
		//set_subq();
	}
}

void TOWNS_CDROM::unpause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PAUSED) {
		set_cdda_status(CDDA_PLAYING);
		//set_subq();
	}
}

void TOWNS_CDROM::stop_cdda_from_cmd()
{
	set_cdda_status(CDDA_OFF);
	set_subq();
}

void TOWNS_CDROM::play_cdda_from_cmd()
{
	uint8_t m_start = command[3]; 
	uint8_t s_start = command[4];
	uint8_t f_start = command[5];
	uint8_t m_end   = command[7];
	uint8_t s_end   = command[8];
	uint8_t f_end   = command[9];
	if(is_device_ready()) {
		cdda_start_frame = FROM_BCD(f_start) + (FROM_BCD(s_start) + FROM_BCD(m_start) * 60) * 75;
		cdda_end_frame   = FROM_BCD(f_end)   + (FROM_BCD(s_end)   + FROM_BCD(m_end) * 60) * 75;
		int track = get_track(cdda_start_frame);
		if(cdda_start_frame >= toc_table[track].pregap) {
			cdda_start_frame -= toc_table[track].pregap;
		}
		if(cdda_start_frame < toc_table[track].index0) {
			cdda_start_frame = toc_table[track].index0; // don't play pregap
		} else if(cdda_start_frame > max_logical_block) {
			cdda_start_frame = 0;
		}
		track = current_track;
		cdda_playing_frame = cdda_start_frame;
		if(cdda_end_frame > toc_table[track + 1].index1 && (cdda_end_frame - toc_table[track].pregap) <= toc_table[track + 1].index1) {
			//auto_increment_track = true;
		}
		if(event_cdda_delay_play >= 0) {
			cancel_event(this, event_cdda_delay_play);
			event_cdda_delay_play = -1;
		}
		register_event(this, EVENT_CDDA_DELAY_PLAY, 10.0, false, &event_cdda_delay_play);
		
	}
	set_subq(); // First
}
	
void TOWNS_CDROM::set_subq(void)
{
	if(is_device_ready()) {
		// create track info
		int track = current_track;
		uint32_t frame;
		uint32_t msf_abs;
		uint32_t msf_rel;
		if(toc_table[track].is_audio) { // OK? (or force ERROR) 20181120 K.O
			frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
			msf_rel = lba_to_msf_alt(frame - toc_table[track].index0);
		} else { // Data
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)(fio_img->Ftell());
				if(is_cue) {
					frame = (cur_position / physical_block_size()) + toc_table[track].lba_offset;
					msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
				} else {
					frame = cur_position / physical_block_size();
					if(frame > toc_table[track].lba_offset) {
						msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
					} else {
						msf_rel = lba_to_msf_alt(0);
					}
				}
			} else {
				frame = toc_table[track].lba_offset;
				msf_rel = 0;
			}
		}
		msf_abs = lba_to_msf_alt(frame);
		subq_overrun = !(subq_buffer->empty());
		subq_buffer->clear();
		// http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-130.pdf
		subq_buffer->write(0x01 | (toc_table[track].is_audio ? 0x00 : 0x40));
		
		subq_buffer->write(TO_BCD(track + 1));		// TNO
		subq_buffer->write(TO_BCD((cdda_status == CDDA_PLAYING) ? 0x00 : ((cdda_status == CDDA_PAUSED) ? 0x00 : 0x01))); // INDEX
		subq_buffer->write(TO_BCD((msf_rel >> 16) & 0xff));	// M (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  8) & 0xff));	// S (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  0) & 0xff));	// F (relative)
		subq_buffer->write(TO_BCD(0x00));	// Zero (relative)
		subq_buffer->write(TO_BCD((msf_abs >> 16) & 0xff));	// M (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  8) & 0xff));	// S (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  0) & 0xff));	// F (absolute)
		// transfer length
		//remain = subq_buffer->count();
		// set first data
		// change to data in phase
		//set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
	} else {
		//write_signals(&output_subq_overrun, (subq_buffer->empty()) ? 0x00000000 : 0xffffffff); // OK?
		subq_buffer->clear();
		// transfer length
		//remain = subq_buffer->count();
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
	return;
}

uint8_t TOWNS_CDROM::get_subq_status()
{
	uint8_t val = 0x00;
	val = val | ((subq_buffer->empty()) ? 0x00 : 0x01);
	val = val | ((subq_overrun) ? 0x02 : 0x00);
	return val;
}

uint8_t TOWNS_CDROM::read_subq()
{
	uint8_t val;
//	if(subq_buffer->empty()) {
//		set_subq();
//	}
	val = (uint8_t)(subq_buffer->read() & 0xff);
	return val;
}

#define STATE_VERSION	1

bool TOWNS_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(subq_buffer->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(subq_overrun);
	state_fio->StateValue(stat_track);
	
	return SCSI_CDROM::process_state(state_fio, loading);
}
	

#else /* __NOT_SCSI_CDROM */	

void TOWNS_CDROM::initialize()
{
	subq_buffer = new FIFO(32); // OK?
	subq_overrun = false;
	stat_track = 0;
	SCSI_CDROM::initialize();
}

void TOWNS_CDROM::release()
{
	subq_buffer->release();
	delete subq_buffer;
	SCSI_CDROM::release();
}

void TOWNS_CDROM::reset()
{
	subq_buffer->clear();
	subq_overrun = false;
	stat_track = current_track;
	out_debug_log("RESET");
	SCSI_CDROM::reset();
}
	
void TOWNS_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TOWNS_CDROM_SET_TRACK) {
		if(((data < 100) && (data >= 0)) || (data == 0xaa)) {
			stat_track = data;
		}
		return;
	}
	SCSI_CDROM::write_signal(id, data, mask);
}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	if(id == SIG_TOWNS_CDROM_IS_MEDIA_INSERTED) {
		return ((is_device_ready()) ? 0xffffffff : 0x00000000);
	} else if(id == SIG_TOWNS_CDROM_MAX_TRACK) {
		if(track_num <= 0) {
			return (uint32_t)(TO_BCD(0x00));
		} else {
			return (uint32_t)(TO_BCD(track_num));
		}
	} else if(id == SIG_TOWNS_CDROM_REACHED_MAX_TRACK) {
		if(track_num <= 0) {
			return 0xffffffff;
		} else {
			if(current_track >= track_num) {
				return 0xffffffff;
			} else {
				return 0x00000000;
			}
		}
	} else if(id == SIG_TOWNS_CDROM_CURRENT_TRACK) {
		if(current_track > track_num) {
			return 0x00000000;
		} else {
			return TO_BCD(current_track);
		}
	} else if(id == SIG_TOWNS_CDROM_START_MSF) {
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
		uint32_t lba = (uint32_t)index0;
		if(pregap > 0) lba = lba - pregap;
		if(lba < 150) lba = 150;
		uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
		stat_track++;
		return msf;
	} else if(id == SIG_TOWNS_CDROM_START_MSF_AA) {
		int trk = track_num;
		int index0 = toc_table[trk].index0;
		int index1 = toc_table[trk].index1;
		int pregap = toc_table[trk].pregap;
		uint32_t lba = (uint32_t)index0;
		if(pregap > 0) lba = lba - pregap;
		if(lba < 150) lba = 150;
		uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
		return msf;
	} else if(id == SIG_TOWNS_CDROM_RELATIVE_MSF) {
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			if(cdda_playing_frame <= cdda_start_frame) {
				return 0;
			}
			uint32_t msf;
			if(cdda_playing_frame >= cdda_end_frame) {
				if(cdda_repeat) {
					return 0;
				} else {
					msf = lba_to_msf(cdda_end_frame - cdda_start_frame);
					return msf;
				}
			}
			msf = lba_to_msf(cdda_playing_frame - cdda_start_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position);
				return msf;
			}
			return 0;
		}
	}  else if(id == SIG_TOWNS_CDROM_ABSOLUTE_MSF) {
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			uint32_t msf;
			msf = lba_to_msf(cdda_playing_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position + toc_table[current_track].lba_offset);
				return msf;
			}
			return 0;
		}
	} else if(id == SIG_TOWNS_CDROM_GET_ADR) {
		int trk = stat_track;
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
	return SCSI_CDROM::read_signal(id);
}

void TOWNS_CDROM::event_callback(int event_id, int err)
{
	SCSI_CDROM::event_callback(event_id, err);
	if(event_id == EVENT_CDDA) {
		// Post process
		if(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
			set_subq();
		}
	}
}

int TOWNS_CDROM::get_command_length(int value)
{
	switch(value) {
	case TOWNS_CDROM_CDDA_PLAY:
		return 10;
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_STOP:
		return 4;
		break;
	}
		
	return SCSI_CDROM::get_command_length(value);
}

void TOWNS_CDROM::start_command()
{
	touch_sound();
	switch(command[0]) {
	case TOWNS_CDROM_CDDA_PLAY:
		play_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		pause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		unpause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_STOP:
		stop_cdda_from_cmd();
		break;
	case SCSI_CMD_TST_U_RDY:
	case SCSI_CMD_INQUIRY:
	case SCSI_CMD_REQ_SENSE:
	case SCSI_CMD_RD_DEFECT:
	case SCSI_CMD_RD_CAPAC:
	case SCSI_CMD_MODE_SEL6: // OK?
	case SCSI_CMD_READ6:
	case SCSI_CMD_READ10:
	case SCSI_CMD_READ12:
		SCSI_CDROM::start_command();
		break;
	case 0xff:
		// End of List
		set_dat(SCSI_STATUS_CHKCOND);
		break;
	default:
		out_debug_log(_T("Command: Unknown %02X\n"), command[0]);
		set_dat(SCSI_STATUS_GOOD);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
}

	
// From MAME 0203's fmtowns.cpp .	
void TOWNS_CDROM::pause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PLAYING) {
		set_cdda_status(CDDA_PAUSED);
		//set_subq();
	}
}

void TOWNS_CDROM::unpause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PAUSED) {
		set_cdda_status(CDDA_PLAYING);
		//set_subq();
	}
}

void TOWNS_CDROM::stop_cdda_from_cmd()
{
	set_cdda_status(CDDA_OFF);
	set_subq();
}

void TOWNS_CDROM::play_cdda_from_cmd()
{
	uint8_t m_start = command[3]; 
	uint8_t s_start = command[4];
	uint8_t f_start = command[5];
	uint8_t m_end   = command[7];
	uint8_t s_end   = command[8];
	uint8_t f_end   = command[9];
	if(is_device_ready()) {
		cdda_start_frame = FROM_BCD(f_start) + (FROM_BCD(s_start) + FROM_BCD(m_start) * 60) * 75;
		cdda_end_frame   = FROM_BCD(f_end)   + (FROM_BCD(s_end)   + FROM_BCD(m_end) * 60) * 75;
		int track = get_track(cdda_start_frame);
		if(cdda_start_frame >= toc_table[track].pregap) {
			cdda_start_frame -= toc_table[track].pregap;
		}
		if(cdda_start_frame < toc_table[track].index0) {
			cdda_start_frame = toc_table[track].index0; // don't play pregap
		} else if(cdda_start_frame > max_logical_block) {
			cdda_start_frame = 0;
		}
		track = current_track;
		cdda_playing_frame = cdda_start_frame;
		if(cdda_end_frame > toc_table[track + 1].index1 && (cdda_end_frame - toc_table[track].pregap) <= toc_table[track + 1].index1) {
			//auto_increment_track = true;
		}
		if(event_cdda_delay_play >= 0) {
			cancel_event(this, event_cdda_delay_play);
			event_cdda_delay_play = -1;
		}
		register_event(this, EVENT_CDDA_DELAY_PLAY, 10.0, false, &event_cdda_delay_play);
		
	}
	set_subq(); // First
}
	
void TOWNS_CDROM::set_subq(void)
{
	if(is_device_ready()) {
		// create track info
		int track = current_track;
		uint32_t frame;
		uint32_t msf_abs;
		uint32_t msf_rel;
		if(toc_table[track].is_audio) { // OK? (or force ERROR) 20181120 K.O
			frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
			msf_rel = lba_to_msf_alt(frame - toc_table[track].index0);
		} else { // Data
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)(fio_img->Ftell());
				if(is_cue) {
					frame = (cur_position / physical_block_size()) + toc_table[track].lba_offset;
					msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
				} else {
					frame = cur_position / physical_block_size();
					if(frame > toc_table[track].lba_offset) {
						msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
					} else {
						msf_rel = lba_to_msf_alt(0);
					}
				}
			} else {
				frame = toc_table[track].lba_offset;
				msf_rel = 0;
			}
		}
		msf_abs = lba_to_msf_alt(frame);
		subq_overrun = !(subq_buffer->empty());
		subq_buffer->clear();
		// http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-130.pdf
		subq_buffer->write(0x01 | (toc_table[track].is_audio ? 0x00 : 0x40));
		
		subq_buffer->write(TO_BCD(track + 1));		// TNO
		subq_buffer->write(TO_BCD((cdda_status == CDDA_PLAYING) ? 0x00 : ((cdda_status == CDDA_PAUSED) ? 0x00 : 0x01))); // INDEX
		subq_buffer->write(TO_BCD((msf_rel >> 16) & 0xff));	// M (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  8) & 0xff));	// S (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  0) & 0xff));	// F (relative)
		subq_buffer->write(TO_BCD(0x00));	// Zero (relative)
		subq_buffer->write(TO_BCD((msf_abs >> 16) & 0xff));	// M (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  8) & 0xff));	// S (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  0) & 0xff));	// F (absolute)
		// transfer length
		//remain = subq_buffer->count();
		// set first data
		// change to data in phase
		//set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
	} else {
		//write_signals(&output_subq_overrun, (subq_buffer->empty()) ? 0x00000000 : 0xffffffff); // OK?
		subq_buffer->clear();
		// transfer length
		//remain = subq_buffer->count();
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
	return;
}

uint8_t TOWNS_CDROM::get_subq_status()
{
	uint8_t val = 0x00;
	val = val | ((subq_buffer->empty()) ? 0x00 : 0x01);
	val = val | ((subq_overrun) ? 0x02 : 0x00);
	return val;
}

uint8_t TOWNS_CDROM::read_subq()
{
	uint8_t val;
//	if(subq_buffer->empty()) {
//		set_subq();
//	}
	val = (uint8_t)(subq_buffer->read() & 0xff);
	return val;
}

#define STATE_VERSION	1

bool TOWNS_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(subq_buffer->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(subq_overrun);
	state_fio->StateValue(stat_track);
	
	return SCSI_CDROM::process_state(state_fio, loading);
}
	
#endif /* !defined(__NOT_SCSI_CDROM) */	
}
