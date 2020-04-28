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
//#include "../scsi_host.h"

// SAME AS SCSI_CDROM::
#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA						100
#define EVENT_CDDA_DELAY_PLAY			101
#define EVENT_CDROM_SEEK				102
#define EVENT_CDROM_DELAY_INTERRUPT_ON	103
#define EVENT_CDROM_DELAY_INTERRUPT_OFF	104
#define EVENT_CDDA_DELAY_STOP			105
#define EVENT_CDROM_SEEK_COMPLETED		106
#define EVENT_CDROM_DRQ					107
#define EVENT_CDROM_NEXT_SECTOR			108

#define _SCSI_DEBUG_LOG
#define _CDROM_DEBUG_LOG

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
	subq_overrun = false;
	stat_track = 0;
	status_queue = new FIFO(4);
	
//   	SCSI_DEV::initialize();
	// ToDo: MasterDevice::initialize()
	fio_img = new FILEIO();
	__CDROM_DEBUG_LOG = osd->check_feature(_T("_CDROM_DEBUG_LOG"));
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
	event_seek = -1;

	buffer = new FIFO(8192);

	cdda_status = CDDA_OFF;
	is_cue = false;
	current_track = 0;
	read_sectors = 0;

	transfer_speed = 1;	
	for(int i = 0; i < 99; i++) {
		memset(track_data_path[i], 0x00, _MAX_PATH * sizeof(_TCHAR));
	}
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
	if(buffer != NULL) {
		buffer->release();
		delete buffer;
		buffer = NULL;
	}
	if(status_queue != NULL) {
		status_queue->release();
		delete status_queue;
		status_queue = NULL;
	}
	
}

void TOWNS_CDROM::reset()
{
	memset(subq_buffer, 0x00, sizeof(subq_buffer));
	memset(param_queue, 0x00, sizeof(param_queue));
	memset(w_regs, 0x00, sizeof(w_regs));
	
	param_ptr = 0;
	subq_overrun = false;
	stat_track = current_track;
	out_debug_log("RESET");
	extra_status = 0;
	data_reg = 0x00;
	position = 0;
	mcu_ready = true;
	has_status = false;
	req_status = false;
	
	touch_sound();
	if(event_delay_interrupt != -1) cancel_event(this, event_delay_interrupt);
	if(event_cdda_delay_play != -1) cancel_event(this, event_cdda_delay_play);
	if(event_cdda != -1) cancel_event(this, event_cdda);
	if(event_drq != -1) cancel_event(this, event_drq);
	if(event_next_sector != -1) cancel_event(this, event_next_sector);
	if(event_seek != -1) cancel_event(this, event_seek);
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	event_drq = -1;
	event_next_sector = -1;
	event_seek = -1;
	read_length = 0;
	
	buffer->clear();
	status_queue->clear();
	latest_command = 0x00;
	if(is_cue) {
		if(fio_img->IsOpened()) {
			fio_img->Fclose();
		}
		current_track = 0;
	} else {
		if(fio_img->IsOpened()) {
			fio_img->Fseek(0, FILEIO_SEEK_SET);
		}
		current_track = 0;
	}
	read_mode = false;
	set_cdda_status(CDDA_OFF);
	is_playing = false;
	read_sectors = 0;
	write_signals(&outputs_drq, 0);
	mcu_intr = false;
	dma_intr = false;
	stat_reply_intr = false;
	mcu_intr_mask = false;
	dma_intr_mask = false;
	dma_transfer = true;
	pio_transfer = false;
	dma_transfer_phase = false;
	pio_transfer_phase = false;
	write_signals(&outputs_mcuint, 0);
	
	// Q: Does not seek to track 0? 20181118 K.O
	//current_track = 0;
//	TOWNS_CDROM::reset();
}

void TOWNS_CDROM::set_dma_intr(bool val)
{
	if(val) {
		dma_intr = true;
		if(!(dma_intr_mask)) {
			write_signals(&outputs_mcuint, 0xffffffff);
		}
	} else {
		if(!(mcu_intr) && (dma_intr)) {
			dma_intr = false;
			write_signals(&outputs_mcuint, 0x0);
		} else {
			dma_intr = false;
		}
	}
}

void TOWNS_CDROM::set_mcu_intr(bool val)
{
	if(!(stat_reply_intr)) return;
	if(val) {
		mcu_intr = true;
		if(!(mcu_intr_mask)) {
			write_signals(&outputs_mcuint, 0xffffffff);
		}
	} else {
		if(!(dma_intr) && (mcu_intr)) {
			mcu_intr = false;
			write_signals(&outputs_mcuint, 0x0);
		} else {
			mcu_intr = false;
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
			reset();
		}
		break;
	default:
//		SCSI_DEV::write_signal(id, data, mask);
		// ToDo: Implement master devices.
		break;
	}

}
void TOWNS_CDROM::status_not_ready()
{
	out_debug_log(_T("CMD (%02X) BUT DISC NOT ACTIVE"), latest_command);
	set_status(req_status, 0, TOWNS_CD_STATUS_DISC_NOT_READY, 0, 0, 0);
//		set_sense_code(SCSI_SENSE_NOTREADY);
	send_mcu_ready();	
}

void TOWNS_CDROM::status_accept(int extra, uint8_t s1, uint8_t s2, uint8_t s3)
{
	set_status(req_status, extra, TOWNS_CD_STATUS_ACCEPT, s1, s2, s3);
//		set_sense_code(SCSI_SENSE_NOTREADY);
	send_mcu_ready();	
}

void TOWNS_CDROM::send_mcu_ready()
{
	mcu_ready = true;
	set_mcu_intr(true);
}

void TOWNS_CDROM::status_not_accept(int extra, uint8_t s1, uint8_t s2, uint8_t s3)
{
	set_status(req_status, extra, TOWNS_CD_STATUS_NOT_ACCEPT, s1, s2, s3);
//		set_sense_code(SCSI_SENSE_NOTREADY);
	send_mcu_ready();	
}

void TOWNS_CDROM::execute_command(uint8_t command)
{
//	status &= ~0x02;
	latest_command = command;
	if(!(mounted()))  {
		status_not_ready();
		return;
	}
	switch(command & 0x9f) {
	case CDROM_COMMAND_SEEK:
		{
			uint8_t m, s, f;
//			m = FROM_BCD(param_queue[0]);
//			s = FROM_BCD(param_queue[1]);
//			f = FROM_BCD(param_queue[2]);
			m = 0;
			s = 2;
			f = 0;
			int32_t lba = ((m * (60 * 75)) + (s * 75) + f) - 150;
			if(lba >= 0) {
				register_event(this, EVENT_CDROM_SEEK, get_seek_time(lba),
							   false, &event_seek);
			}
			out_debug_log(_T("CMD SEEK(%02X) M S F = %d %d %d  LBA=%d"), command,
						  TO_BCD(m), TO_BCD(s), TO_BCD(f), lba
				);
		}
		break;
	case CDROM_COMMAND_READ_MODE2:
		out_debug_log(_T("CMD READ MODE2(%02X)"), command);
		status_accept(0, 0xff, 0xff, 0xff);
		break;
	case CDROM_COMMAND_READ_MODE1:
		out_debug_log(_T("CMD READ MODE1(%02X)"), command);
		read_cdrom();
		break;
	case CDROM_COMMAND_PLAY_TRACK:
		out_debug_log(_T("CMD PLAY TRACK(%02X)"), command);
		play_cdda_from_cmd(); // ToDo : Re-Implement.
//		play_cdda(req_status);
		break;
	case CDROM_COMMAND_READ_TOC:
		out_debug_log(_T("CMD READ TOC(%02X)"), command);
		if(req_status) {
			set_status(true, 1, TOWNS_CD_STATUS_ACCEPT, 0, 0, 0);
		} else {
			set_status(true, 2, TOWNS_CD_STATUS_TOC_ADDR, 0, 0xa0, 0);
		}
		send_mcu_ready();
		// TOC READING
		break;
	case CDROM_COMMAND_READ_CDDA_STATE:
		out_debug_log(_T("CMD SET CDDA STATE(%02X)"), command);
		status_accept(1, 0x00, 0x00, 0x00);
		break;
	case CDROM_COMMAND_1F:
		out_debug_log(_T("CMD UNKNOWN 1F(%02X)"), command);
		status_not_ready(); // ToDo: Will implement
		break;
	case CDROM_COMMAND_SET_STATE:
		{
			uint8_t playcode;
			out_debug_log(_T("CMD SET STATE(%02X)"), command);
			if(is_playing) { // OK?
				playcode = 3;
			} else {
				playcode = 1;
			}
			status_accept(0, playcode, 0x00, 0x00);
		}
		break;
	case CDROM_COMMAND_SET_CDDASET:
		out_debug_log(_T("CMD CDDA SET(%02X)"), command);
		status_accept(0, 0x00, 0x00, 0x00);
		break;
	case CDROM_COMMAND_STOP_CDDA:
		out_debug_log(_T("CMD STOP CDDA(%02X)"), command);
		stop_cdda_from_cmd();
		break;
	case CDROM_COMMAND_PAUSE_CDDA:
		out_debug_log(_T("CMD PLAY CDDA2(%02X)"), command);
		pause_cdda_from_cmd(); // ToDo : Re-Implement.
		break;
	case CDROM_COMMAND_RESUME_CDDA:
		out_debug_log(_T("CMD RESUME CDDA(%02X)"), command);
		unpause_cdda_from_cmd();
		break;
	default:
		out_debug_log(_T("CMD Illegal(%02X)"), command);
		status_not_accept(0, 0x00, 0x00, 0x00); // ToDo: Will implement
		break;
	}
}
void TOWNS_CDROM::set_status_extra(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	has_status = true;
	mcu_ready = true;
	status_queue->write(s0);
	status_queue->write(s1);
	status_queue->write(s2);
	status_queue->write(s3);
	out_debug_log(_T("SET EXTRA STATUS %02x: %02x %02x %02x %02x EXTRA COUNT=%d"), latest_command, s0, s1, s2, s3, extra_status);
	set_mcu_intr(true);
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
	uint8_t val = 0x00;
	if(status_queue->empty()) {
		has_status = false;
		return val;
	}
	val = status_queue->read();
	if((status_queue->empty()) && (extra_status > 0)) {
		has_status = false;
		switch(latest_command & 0x9f) {
		case CDROM_COMMAND_SEEK: // seek
			set_status_extra(4, 0, 0, 0);
			extra_status = 0;
			break;
		case CDROM_COMMAND_READ_MODE1: // seek
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
				set_status_extra_toc_data(TO_BCD(track_num), 0x00, 0x00); // OK?
				break;
			case 5:
				set_status_extra_toc_addr(0x00, 0xa2, 0x00);
				break;
			case 6:
				{
					pair32_t msf;
					msf.d= read_signal(SIG_TOWNS_CDROM_START_MSF_AA);
					set_status_extra_toc_data(msf.b.h2, msf.b.h, msf.b.l); // OK?
				}
				break;
			default:
				if((extra_status & 0x01) != 0) {
					uint32_t adr_control = cdrom_get_adr((extra_status - 1) >> 1);
					set_status_extra_toc_addr(((adr_control & 0x0f) << 4) | ((adr_control >> 4) & 0x0f), TO_BCD((extra_status / 2) - 2), 0x00);
				} else {
					pair32_t msf;
					msf.d = read_signal(SIG_TOWNS_CDROM_START_MSF);
					set_status_extra_toc_data(msf.b.h2, msf.b.h, msf.b.l); // OK?
					if((track_num <= 0) || ((extra_status >> 1) >= track_num)) {
						extra_status = 0; // It's end.
					}
				}
				break;
			}
		case CDROM_COMMAND_READ_CDDA_STATE: // READ CDDA status
			{
				switch(extra_status) {
				case 1: // Get current track
					set_status_extra(TOWNS_CD_STATUS_SUBQ_READ, TOWNS_CD_STATUS_ACCEPT, read_signal(SIG_TOWNS_CDROM_CURRENT_TRACK), 0x00);
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
		
			}
		case CDROM_COMMAND_STOP_CDDA:
			set_status_extra(TOWNS_CD_STATUS_STOP_DONE, 0x00, 0x00, 0x00);
			extra_status = 0;
			break;
		case CDROM_COMMAND_PAUSE_CDDA:
			set_status_extra(TOWNS_CD_STATUS_PAUSE_DONE, 0x00, 0x00, 0x00);
			extra_status = 0;
			break;
			
		}
	} else if(status_queue->empty()) {
		has_status = false;
	}
	
	return val;
}

uint32_t TOWNS_CDROM::read_dma_io8(uint32_t addr)
{
	data_reg = (uint8_t)(buffer->read() & 0xff);
	return data_reg;
}

void TOWNS_CDROM::write_dma_io8(uint32_t addr, uint32_t data)
{
	// data_reg = data;
	return; // OK?
}

void TOWNS_CDROM::read_cdrom()
{
	if(!(is_device_ready())) {
		out_debug_log(_T("DEVICE NOT READY"));
		status_not_ready();
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
	track = get_track(lba1);
	out_debug_log(_T("READ_CDROM TRACK=%d LBA1=%06X LBA2=%06X F1/S1/M1=%02X/%02X/%02X F2/S2/M2=%02X/%02X/%02X PAD=%02X DCMD=%02X"), track, lba1, lba2, f1, s1, m1, f2, s2, m2, pad1, dcmd);
	set_cdda_status(CDDA_OFF);
	
	if(lba1 > lba2) { // NOOP?
		extra_status = 0;
		read_length = 0;
		status_not_accept(0, 0x00, 0x00, 0x00);
		return;
	}
	
	__remain = (lba2 - lba1 + 1);
	read_length = __remain * logical_block_size();
	extra_status = 0;
	dma_transfer_phase = dma_transfer;
	pio_transfer_phase = pio_transfer;
	if(event_drq > -1) {  
		cancel_event(this, event_drq);
		event_drq = -1;
	}
	if(event_next_sector > -1) {  
		cancel_event(this, event_next_sector);
		event_next_sector = -1;
	}
	// Kick a first
	double usec = get_seek_time(lba1);
	if(usec < ((1.0e6 * 2048.0) / 150.0e3)) usec = ((1.0e6 * 2048.0) / 150.0e3);
	register_event(this, EVENT_CDROM_SEEK_COMPLETED, usec, false, NULL);
	if(req_status) {
		set_status(req_status, 2, 0x00, 0x00, 0x00, 0x00);
	} else {
		if(pio_transfer) {
			set_status(true, 0, TOWNS_CD_STATUS_CMD_ABEND, 0x00, 0x00, 0x00); // OK?
		} else {
			set_status(true, 0, TOWNS_CD_STATUS_DATA_READY, 0x00, 0x00, 0x00);
		}
	}
}	


void TOWNS_CDROM::set_status(bool _req_status, int extra, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	status_queue->clear();
	extra_status = 0;
	if(extra > 0) extra_status = extra;
	if(_req_status) {
		has_status = true;
//		mcu_ready = true;
		status_queue->write(s0);
		status_queue->write(s1);
		status_queue->write(s2);
		status_queue->write(s3);
		out_debug_log(_T("SET STATUS %02x: %02x %02x %02x %02x EXTRA=%d"), latest_command, s0, s1, s2, s3, extra_status);
	} else {
//		mcu_ready = true;
	}
}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	switch(id) {
	case SIG_TOWNS_CDROM_READ_DATA:
//		if(is_data_in) {
//			is_data_in = false;
			return data_reg;
//		} else {
//			return 0x00; // OK?
//		}
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


int TOWNS_CDROM::logical_block_size()
{
	if(current_track <= 0) return 2048; // PAD
	if(!mounted()) return 2048; // PAD
	if(toc_table[current_track].is_audio) {
		return 2352;
	}
	if(read_mode) { // MODE2
		return 2352;
	}
	return 2048;
}


void TOWNS_CDROM::event_callback(int event_id, int err)
{
	switch (event_id) {
	case EVENT_CDROM_DELAY_INTERRUPT_ON:
		write_signals(&outputs_mcuint, 0xffffffff);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDROM_DELAY_INTERRUPT_OFF:
		write_signals(&outputs_mcuint, 0x00000000);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDDA_DELAY_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			set_cdda_status(CDDA_PLAYING);
		}
		event_cdda_delay_play = -1;
		break;
	case EVENT_CDDA:
		read_a_cdda_sample();
		return;
		break;
	case EVENT_CDDA_DELAY_STOP:
		if(cdda_interrupt) {
			write_signals(&outputs_mcuint, 0xffffffff);
		}
		set_cdda_status(CDDA_OFF);
		event_cdda_delay_play = -1;
		break;
	case EVENT_CDROM_SEEK:
		{
			event_seek = -1;
			status_accept(1, 0x00, 0x00, 0x00);
		}
		break;
	case EVENT_CDROM_SEEK_COMPLETED:
		if(read_length > 0) {
			if(read_length >= logical_block_size()) {
				read_buffer(logical_block_size());
			} else {
				read_buffer(read_length);
			}				
		}
		break;
	case EVENT_CDROM_NEXT_SECTOR:
		event_next_sector = -1;
//		set_dma_intr(true);
		register_event(this, EVENT_CDROM_SEEK_COMPLETED,
					   (1.0e6 / ((double)transfer_speed * 150.0e3)) * 0.5, // OK?
					   false, NULL);
		break;
	case EVENT_CDROM_DRQ:
		if(buffer->empty()) {
			cancel_event(this, event_drq);
			event_drq = -1;
			if(read_length > 0) {
				out_debug_log(_T("READ NEXT SECTOR"));
				set_status(true, 0, TOWNS_CD_STATUS_DATA_READY, 0x00, 0x00, 0x00);
				set_dma_intr(true);
				register_event(this, EVENT_CDROM_SEEK_COMPLETED,
							   (1.0e6 / ((double)transfer_speed * 150.0e3)) * 16.0, // OK?
							   false, NULL);
			} else {
				out_debug_log(_T("EOT"));
				set_status(true, 0, TOWNS_CD_STATUS_READ_DONE, 0x00, 0x00, 0x00);
				dma_transfer_phase = false;
				pio_transfer_phase = false;
				send_mcu_ready();
			}
			return;
		}
		if(dma_transfer_phase) {
			write_signals(&outputs_drq, 0xffffffff);
		}
		break;
	default:
		// ToDo: Another events.
		//SCSI_DEV::event_callback(event_id, err);
		break;
	}
}

bool TOWNS_CDROM::read_buffer(int length)
{
	if(!fio_img->IsOpened()) {
		status_not_ready();
		return false;
	}
	uint32_t offset = (uint32_t)(position % 2352);
	if(is_cue) {
		// ToDo: Need seek wait.
		out_debug_log(_T("Seek to LBA %d LENGTH=%d\n"), position / 2352, length);
		if(fio_img->Fseek(((long)position - (long)(toc_table[current_track].lba_offset * 2352)), FILEIO_SEEK_SET) != 0) {
//			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
			send_mcu_ready();
			return false;
		}
	} else {
		if(fio_img->Fseek((long)position, FILEIO_SEEK_SET) != 0) {
//			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
			send_mcu_ready();
			return false;
		}
	}
	int n_length = length;
	while(length > 0) {
		uint8_t tmp_buffer[2352];
//		int tmp_length = min(length, (int)sizeof(tmp_buffer));
		int tmp_length = 2352 - offset;
		if(fio_img->Fread(tmp_buffer, tmp_length, 1) != 1) {
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
//			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_NORECORDFND
			send_mcu_ready();
			return false;
		}
		for(int i = 0; i < tmp_length; i++) {
			if((offset >= 16) && (offset < (16 + logical_block_size()))) {
				int value = tmp_buffer[i];
				buffer->write(value);
//				is_data_in = false;
				length--;
				read_length--;
				// Kick DRQ
				if(event_drq < 0) {
					out_debug_log(_T("KICK DRQ"));
					register_event(this, EVENT_CDROM_DRQ, 1.0e6 / ((double)transfer_speed * 150.0e3), true, &event_drq);
				}
			}
			position++;
			offset = (offset + 1) % 2352;
		}
		access = true;
	}
	return true;
}

void TOWNS_CDROM::read_a_cdda_sample()
{
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
			write_signals(&outputs_mcuint, 0x00000000);
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
			is_playing = true;
		}
	} else {
		if(event_cdda != -1) {
			cancel_event(this, event_cdda);
			event_cdda = -1;
		}
		if(cdda_status == CDDA_PLAYING) {
			// Notify to release bus.
			write_signals(&outputs_mcuint, 0x00000000);
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
			is_playing = true;
		}
	}
	cdda_status = status;
}

void TOWNS_CDROM::reset_device()
{
	set_cdda_status(CDDA_OFF);
	is_playing = false;
//	SCSI_DEV::reset_device();
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
	if(is_cue) {
		get_track_by_track_num(track);
	} else {
		current_track = track;
	}
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

uint32_t TOWNS_CDROM::lba_to_msf(uint32_t lba)
{
	uint8_t m = lba / (60 * 75);
	lba -= m * (60 * 75);
	uint8_t s = lba / 75;
	uint8_t f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) | ((s / 10) << 12) | ((s % 10) << 8) | ((f / 10) << 4) | ((f % 10) << 0);
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
	if(cdda_status == CDDA_PAUSED) {
		set_cdda_status(CDDA_PLAYING);
		//set_subq();
	}
	send_mcu_ready();	
}

void TOWNS_CDROM::stop_cdda_from_cmd()
{
	if(!is_device_ready()) {
		status_not_ready();
		return;
	}
	set_cdda_status(CDDA_OFF);
	set_subq();
	status_accept(1, 0x00, 0x00, 0x00);
}

void TOWNS_CDROM::pause_cdda_from_cmd()
{
	if(!is_device_ready()) {
		status_not_ready();
		return;
	}
	uint8_t m1, s1, f1;
	uint8_t m2, s2, f2;
	
	m1 = FROM_BCD(param_queue[0]);
	s1 = FROM_BCD(param_queue[1]);
	f1 = FROM_BCD(param_queue[2]);

	m2 = FROM_BCD(param_queue[3]);
	s2 = FROM_BCD(param_queue[4]);
	f2 = FROM_BCD(param_queue[5]);
	uint8_t pad1 = param_queue[6];
	uint8_t dcmd = param_queue[7];
	
	set_cdda_status(CDDA_OFF);
	set_subq();
	status_accept(1, 0x00, 0x00, 0x00);
}

void TOWNS_CDROM::play_cdda_from_cmd()
{
	uint8_t m_start = param_queue[0]; 
	uint8_t s_start = param_queue[1];
	uint8_t f_start = param_queue[2];
	uint8_t m_end   = param_queue[3];
	uint8_t s_end   = param_queue[4];
	uint8_t f_end   = param_queue[5];
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
	send_mcu_ready();	
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
			((data[nbit >> 3] & (1 << (7 - (nbit & 7))) != 0) ? 1 : 0);
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
		if(subq_bitptr < subq_bitwidth) {
			subq_overrun = true;
		}
		msf_abs = lba_to_msf_alt(frame);
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

int TOWNS_CDROM::hexatoi(const char *s)
{
	const char *ptr = s;
	int value = 0;
	
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

bool TOWNS_CDROM::open_cue_file(const _TCHAR* file_path)
{
	std::string line_buf;
	std::string line_buf_shadow;
	std::string image_tmp_data_path;

	_TCHAR full_path_cue[_MAX_PATH];
	size_t ptr;
	int line_count = 0;
	int slen;
	int nr_current_track = 0;
	FILEIO* fio = new FILEIO();
	if(fio == NULL) return false;

	memset(full_path_cue, 0x00, sizeof(full_path_cue));
	image_tmp_data_path.clear();

	get_long_full_path_name(file_path, full_path_cue, sizeof(full_path_cue));
	
	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_cue);

	size_t _arg1_ptr;
	size_t _arg2_ptr;
	size_t _arg2_ptr_s;
	size_t _arg3_ptr;
	size_t _arg3_ptr_s;

	std::string _arg1;
	std::string _arg2;
	std::string _arg3;
	
	if(fio->Fopen(file_path, FILEIO_READ_ASCII)) { // ToDo: Support not ASCII cue file (i.e. SJIS/UTF8).20181118 K.O
		line_buf.clear();
		for(int i = 0; i < 100; i++) {
			memset(&(track_data_path[i][0]), 0x00, _MAX_PATH * sizeof(_TCHAR));
		}
		int _c;
		bool is_eof = false;
		int sptr = 0;
		while(1) {
			line_buf.clear();
			int _np = 0;
			_c = EOF;
			do {
				_c = fio->Fgetc();
				if((_c == '\0') || (_c == '\n') || (_c == EOF)) break;;
				if(_c != '\r') line_buf.push_back((char)_c);
			} while(1);
			if(_c == EOF) is_eof = true;
			slen = (int)line_buf.length();
			if(slen <= 0) goto _n_continue;
			// Trim head of Space or TAB
			ptr = 0;
			sptr = 0;
			// Tokenize
			_arg1.clear();
			_arg2.clear();
			_arg3.clear();
			
			ptr = line_buf.find_first_not_of((const char*)" \t");
			if(ptr == std::string::npos) {
				goto _n_continue;
			}
			// Token1
			line_buf_shadow = line_buf.substr(ptr);

			_arg1_ptr = line_buf_shadow.find_first_of((const char *)" \t");
			_arg1 = line_buf_shadow.substr(0, _arg1_ptr);
			_arg2 = line_buf_shadow.substr(_arg1_ptr);
			std::transform(_arg1.begin(), _arg1.end(), _arg1.begin(),
						   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

			_arg2_ptr = _arg2.find_first_not_of((const char *)" \t");

			if(_arg2_ptr != std::string::npos) {
				_arg2 = _arg2.substr(_arg2_ptr);
			}

			if(_arg1 == "REM") {
				// REM
				goto _n_continue;
			} else if(_arg1 == "FILE") {
				_arg2_ptr = _arg2.find_first_of((const char *)"\"") + 1;
				if(_arg2_ptr == std::string::npos) goto _n_continue;
				
				_arg2 = _arg2.substr(_arg2_ptr);
				_arg3_ptr = _arg2.find_first_of((const char *)"\"");
				if(_arg3_ptr == std::string::npos) goto _n_continue;
				_arg2 = _arg2.substr(0, _arg3_ptr);
				
				image_tmp_data_path.clear();
				image_tmp_data_path = std::string(parent_dir);
				image_tmp_data_path.append(_arg2);
				
				out_debug_log(_T("**FILE %s\n"), image_tmp_data_path.c_str());
				goto _n_continue; // ToDo: Check ARG2 (BINARY etc..) 20181118 K.O
			} else if(_arg1 == "TRACK") {
				_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
				
				_arg3 = _arg2.substr(_arg2_ptr_s);
				_arg2 = _arg2.substr(0, _arg2_ptr_s);
				_arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
				int _nr_num = atoi(_arg2.c_str());
				
				// Set image file
				if((_nr_num > 0) && (_nr_num < 100) && (_arg3_ptr != std::string::npos)) {
					nr_current_track = _nr_num;
					_arg3 = _arg3.substr(_arg3_ptr);
					
					memset(track_data_path[_nr_num - 1], 0x00, sizeof(_TCHAR) * _MAX_PATH);
					strncpy((char *)(track_data_path[_nr_num - 1]), image_tmp_data_path.c_str(), _MAX_PATH);
					_arg3_ptr_s = _arg3.find_first_of((const char *)" \t\n");
					_arg3.substr(0, _arg3_ptr_s);
					
					std::transform(_arg3.begin(), _arg3.end(), _arg3.begin(),
								   [](unsigned char c) -> unsigned char{ return std::toupper(c); });
					
					toc_table[nr_current_track].is_audio = false;
					toc_table[nr_current_track].index0 = 0;
					toc_table[nr_current_track].index1 = 0;
					toc_table[nr_current_track].pregap = 0;
						
					if(_arg3.compare("AUDIO") == 0) {
						toc_table[nr_current_track].is_audio = true;
					} else if(_arg3.compare("MODE1/2352") == 0) {
						toc_table[nr_current_track].is_audio = false;
					} else {
						// ToDo:  another type
					}
					if(track_num < (_nr_num + 1)) track_num = _nr_num + 1;
				} else {
					// ToDo: 20181118 K.Ohta
					nr_current_track = 0;
				}
				goto _n_continue;
			} else if(_arg1 == "INDEX") {
				
				if((nr_current_track > 0) && (nr_current_track < 100)) {
					_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
					if(_arg2_ptr_s == std::string::npos) goto _n_continue;
					
					_arg3 = _arg2.substr(_arg2_ptr_s);
					_arg2 = _arg2.substr(0, _arg2_ptr_s);
					_arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
					if(_arg3_ptr == std::string::npos) goto _n_continue;

					_arg3 = _arg3.substr(_arg3_ptr);
					_arg3_ptr_s = _arg3.find_first_of((const char *)" \t");
					_arg3.substr(0, _arg3_ptr_s);
					int index_type = atoi(_arg2.c_str());

					switch(index_type) {
					case 0:
						toc_table[nr_current_track].index0 = get_frames_from_msf(_arg3.c_str());
						break;
					case 1:
						toc_table[nr_current_track].index1 = get_frames_from_msf(_arg3.c_str());
						break;
					default:
						break;
					}
					goto _n_continue;
				} else {
					goto _n_continue;
				}
			} else if(_arg1 == "PREGAP") {
				if((nr_current_track > 0) && (nr_current_track < 100)) {
					_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
					_arg2 = _arg2.substr(0, _arg2_ptr_s - 1);
					
					toc_table[nr_current_track].pregap = get_frames_from_msf(_arg2.c_str());
					goto _n_continue;
				} else {
					goto _n_continue;
				}
			}
		_n_continue:
			if(is_eof) break;
			line_buf.clear();
			continue;
		}
		// Finish
		max_logical_block = 0;
		if(track_num > 0) {
			toc_table[0].lba_offset = 0;
			toc_table[0].lba_size = 0;
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			// P1: Calc
			int _n = 0;
			for(int i = 1; i < track_num; i++) {

				if(fio_img->IsOpened()) {
					fio_img->Fclose();
				}
				// Even...
				//if(toc_table[i].pregap <= 0) {
				//	toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				//}
				if(toc_table[i].index1 != 0) {
					toc_table[i].index0 = toc_table[i].index0 + max_logical_block;
					toc_table[i].index1 = toc_table[i].index1 + max_logical_block;
					if(toc_table[i].pregap == 0) {
						toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
					}
				} else { // index1 == 0
					if(toc_table[i].pregap == 0) {
						toc_table[i].pregap = toc_table[i].index0;
					}
					toc_table[i].index1 = toc_table[i].index0 + max_logical_block;
					toc_table[i].index0 = max_logical_block;
				}
				// Even...
				if(toc_table[i].pregap <= 150) {
					toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				}

				_n = 0;
				if(strlen(track_data_path[i - 1]) > 0) {
					if(fio_img->Fopen(track_data_path[i - 1], FILEIO_READ_BINARY)) {
						if((_n = fio_img->FileLength() / 2352) > 0) {
							max_logical_block += _n;
						} else {
							_n = 0;
						}
						fio_img->Fclose();
					}
				}
				toc_table[i].lba_size = _n;
				toc_table[i].lba_offset = max_logical_block - _n;

				
				//#ifdef _CDROM_DEBUG_LOG
				out_debug_log(_T("TRACK#%02d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d LBA_OFFSET=%d PATH=%s\n"),
									i, (toc_table[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352"),
									toc_table[i].pregap, toc_table[i].index0, toc_table[i].index1,
									toc_table[i].lba_size, toc_table[i].lba_offset, track_data_path[i - 1]);
				//#endif
			}
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
//			toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
//			toc_table[track_num].lba_offset = max_logical_block;
//			toc_table[track_num].lba_size = 0;
			
		}
		fio->Fclose();
	}
	delete fio;

	is_cue = false;
	if(track_num > 0) is_cue = true;
	// Not Cue FILE.
	return is_cue;
}

void TOWNS_CDROM::open(const _TCHAR* file_path)
{
	_TCHAR img_file_path[_MAX_PATH] = {0};
	memset(img_file_path_bak, 0x00, sizeof(img_file_path_bak));
	
	close();
	access = false;
	
	if(check_file_extension(file_path, _T(".cue"))) {
		is_cue = false;
		current_track = 0;
		if(open_cue_file(file_path)) {
			strncpy(img_file_path_bak, file_path, _MAX_PATH);
		}

	} else if(check_file_extension(file_path, _T(".ccd"))) {
		// get image file name
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
		if(!FILEIO::IsFileExisting(img_file_path)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
			if(!FILEIO::IsFileExisting(img_file_path)) {
				my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
			}
		}
		if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
			is_cue = false;
			current_track = 0;
			// get image file size
			if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
				// read cue file
				FILEIO* fio = new FILEIO();
				if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
					char line[1024], *ptr;
					int track = -1;
					while(fio->Fgets(line, 1024) != NULL) {
						if(strstr(line, "[Session ") != NULL) {
							track = -1;
						} else if((ptr = strstr(line, "Point=0x")) != NULL) {
							if((track = hexatoi(ptr + 8)) > 0 && track < 0xa0) {
								if(track > track_num) {
									track_num = track;
								}
							}
						} else if((ptr = strstr(line, "Control=0x")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].is_audio = (hexatoi(ptr + 10) != 4);
							}
						} else if((ptr = strstr(line, "ALBA=-")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].pregap = atoi(ptr + 6);
							}
						} else if((ptr = strstr(line, "PLBA=")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].index1 = atoi(ptr + 5);
							}
						}
					}
					if(track_num != 0) {
						toc_table[0].lba_offset = 0;
						toc_table[0].pregap = 0;
						for(int i = 1; i < track_num; i++) {
							toc_table[i].index0 = toc_table[i].index1 - toc_table[i].pregap;
							toc_table[i].lba_offset = toc_table[i].pregap;
							toc_table[i - 1].lba_size = toc_table[i].pregap - toc_table[i - 1].pregap;
						}
						toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
						toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
						if(track_num > 0) {
							toc_table[track_num].lba_size = max_logical_block - toc_table[track_num - 1].lba_offset;
						} else {
							toc_table[track_num].lba_size = 0;
						}
					} else {
						fio_img->Fclose();
					}
					strncpy(img_file_path_bak, file_path, _MAX_PATH);
					fio->Fclose();
				}
				delete fio;
			}
		}
	}
 
	if(mounted() /*&& (__SCSI_DEBUG_LOG)*/) {
		for(int i = 0; i < track_num + 1; i++) {
			uint32_t idx0_msf = lba_to_msf(toc_table[i].index0);
			uint32_t idx1_msf = lba_to_msf(toc_table[i].index1);
			uint32_t pgap_msf = lba_to_msf(toc_table[i].pregap);
			this->out_debug_log(_T("Track%02d: Index0=%02x:%02x:%02x Index1=%02x:%02x:%02x PreGap=%02x:%02x:%02x\n"), i + 1,
			(idx0_msf >> 16) & 0xff, (idx0_msf >> 8) & 0xff, idx0_msf & 0xff,
			(idx1_msf >> 16) & 0xff, (idx1_msf >> 8) & 0xff, idx1_msf & 0xff,
			(pgap_msf >> 16) & 0xff, (pgap_msf >> 8) & 0xff, pgap_msf & 0xff);
		}
	}
}

void TOWNS_CDROM::close()
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
		int32_t val_l = apply_volume(apply_volume(cdda_sample_l, volume_m), volume_l);
		int32_t val_r = apply_volume(apply_volume(cdda_sample_r, volume_m), volume_r);
		
		for(int i = 0; i < cnt; i++) {
			*buffer++ += val_l; // L
			*buffer++ += val_r; // R
		}
	}
}

void TOWNS_CDROM::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l + 3.0);
	volume_r = decibel_to_volume(decibel_r + 3.0);
}

void TOWNS_CDROM::set_volume(int volume)
{
	volume_m = (int)(1024.0 * (max(0, min(100, volume)) / 100.0));
}

uint32_t TOWNS_CDROM::read_io8(uint32_t addr)
{
	/*
	 * 04C0h : Master status
	 * 04C2h : CDC status
	 * 04C4h : DATA
	 * 04CCh : SUBQ CODE
	 * 04CDh : SUBQ STATUS 
	 */
	uint32_t val = 0;
	switch(addr & 0x0f) {
	case 0x00:
		val = val | ((mcu_intr)					? 0x80 : 0x00);
		val = val | ((dma_intr)					? 0x40 : 0x00);
		val = val | ((pio_transfer_phase)				? 0x20 : 0x00);
//			val = val | ((d_dmac->read_signal(SIG_UPD71071_IS_TRANSFERING + 3) !=0) ? 0x10 : 0x00); // USING DMAC ch.3
		val = val | ((dma_transfer_phase)				? 0x10 : 0x00); // USING DMAC ch.3
		val = val | ((has_status)				? 0x02 : 0x00);
		val = val | ((mcu_ready)				? 0x01 : 0x00);
		if((mcu_intr) || (dma_intr)) { 
			mcu_intr = false;
			dma_intr = false;
			write_signals(&outputs_mcuint, 0x00000000);
		}
		break;
	case 0x02:
		val = read_status();
		break;
	case 0x04:
		if(pio_transfer_phase) {
			val = (buffer->read() & 0xff);
			data_reg = val;
		}
		break;
	case 0x0c: // Subq code
		val = read_subq();
		break;
	case 0x0d: // Subq status
		val = get_subq_status();
		break;
	}

	return val;
}

void TOWNS_CDROM::write_io8(uint32_t addr, uint32_t data)
{
	/*
	 * 04C0h : Master control register
	 * 04C2h : Command register
	 * 04C4h : Parameter register
	 * 04C6h : Transfer control register.
	 */
	w_regs[addr & 0x0f] = data;
	switch(addr & 0x0f) {
	case 0x00: // Master control register
		if((data & 0x04) != 0) {
			reset();
			break;
		}
		mcu_intr_mask = ((data & 0x02) == 0) ? true : false;
		dma_intr_mask = ((data & 0x01) == 0) ? true : false;
		if((data & 0x80) != 0) {
			if(mcu_intr) set_mcu_intr(false);
		}
		if((data & 0x40) != 0) {
			if(dma_intr) set_dma_intr(false);
		}
		break;
	case 0x02: // Command
		if(mcu_ready) {
			stat_reply_intr	= ((data & 0x40) != 0) ? true : false;
			req_status		= ((data & 0x20) != 0) ? true : false;
			param_ptr = 0;
			mcu_ready = false;
			extra_status = 0;
			dma_transfer_phase = false;
			pio_transfer_phase = false;
			execute_command(data);
		}
		break;
	case 0x04: // Param
		param_queue[param_ptr] = data;
		param_ptr = (param_ptr + 1) & 0x07;
		break;
	case 0x06:
		if((data & 0x08) != 0) {
			dma_transfer = false;
			pio_transfer = true;
		}
		if((data & 0x10) != 0) {
			dma_transfer = true;
			pio_transfer = false;
		}
		out_debug_log(_T("SET TRANSFER NODE to %02X"), data);
		break;
	}
}

/*
 * Note: 20200428 K.O: DO NOT USE STATE SAVE, STILL don't implement completely yet.
 */
#define STATE_VERSION	1

bool TOWNS_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(buffer->process_state((void *)state_fio, loading))) {
		return false;
	}
	if(!(status_queue->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateArray(w_regs, sizeof(w_regs), 1);
	for(int i = 0; i < (sizeof(subq_buffer) / sizeof(SUBC_t)); i++) {
		state_fio->StateValue(subq_buffer[i].byte);
	}
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
	state_fio->StateValue(has_status);
	state_fio->StateValue(mcu_intr_mask);
	state_fio->StateValue(dma_intr_mask);
	state_fio->StateValue(transfer_speed);
	state_fio->StateValue(read_length);
	
	state_fio->StateValue(param_ptr);
	state_fio->StateArray(param_queue, sizeof(param_queue), 1);
	state_fio->StateValue(extra_status);

	state_fio->StateValue(subq_bitwidth);
	state_fio->StateValue(subq_bitptr);
	state_fio->StateValue(subq_overrun);
	state_fio->StateValue(stat_track);
	// SCSI_CDROM
	uint32_t offset = 0;
	state_fio->StateValue(read_sectors);
	state_fio->StateValue(mix_loop_num);
	state_fio->StateValue(read_mode);
	
	state_fio->StateArray(img_file_path_bak, sizeof(img_file_path_bak), 1);
	state_fio->StateValue(is_cue);
	state_fio->StateValue(current_track);
	state_fio->StateValue(track_num);
	state_fio->StateValue(is_playing);

	state_fio->StateValue(cdda_start_frame);
	state_fio->StateValue(cdda_end_frame);
	state_fio->StateValue(cdda_playing_frame);
	state_fio->StateValue(cdda_status);
	state_fio->StateValue(cdda_repeat);
	state_fio->StateValue(cdda_interrupt);
	state_fio->StateArray(cdda_buffer, sizeof(cdda_buffer), 1);
	state_fio->StateValue(cdda_buffer_ptr);
	state_fio->StateValue(cdda_sample_l);
	state_fio->StateValue(cdda_sample_r);
	
	state_fio->StateValue(volume_l);
	state_fio->StateValue(volume_r);
	state_fio->StateValue(volume_m);
	
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
			close();
		}
		bool is_cue_bak = is_cue;
		int track_num_bak = track_num;
		if(strlen(img_file_path_bak) > 0) {
			open(img_file_path_bak);
		}
		if((is_cue_bak == is_cue) && (track_num_bak == track_num)) {
			if((current_track > 0) && (current_track < 100)) {
				get_track_by_track_num(current_track); // Re-Play
			}
			if(fio_img->IsOpened()) {
				fio_img->Fseek(offset, FILEIO_SEEK_SET);
			}
		} else {
			close();
		}
 	}
	state_fio->StateValue(event_seek);
	state_fio->StateValue(event_cdda);
	state_fio->StateValue(event_cdda_delay_play);
	state_fio->StateValue(event_delay_interrupt);
	state_fio->StateValue(event_drq);
	state_fio->StateValue(event_next_sector);
	
	// SCSI_DEV
	state_fio->StateValue(position);
//	state_fio->StateValue(remain);
//	state_fio->StateValue(local_data_pos);
//	state_fio->StateValue(sense_code);
 	return true;
}
	


}
