/*
	NEC PC-8001 Emulator 'ePC-8001'
	NEC PC-8001mkII Emulator 'ePC-8001mkII'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'
	NEC PC-8801 Emulator 'ePC-8801'
	NEC PC-8801mkII Emulator 'ePC-8801mkII'
	NEC PC-8801MA Emulator 'ePC-8801MA'

	Author : Takeda.Toshiya
	Date   : 2012.02.16-

	[ virtual machine ]
*/

#ifndef _PC8801_H_
#define _PC8801_H_

#if defined(_PC8801MA)
#define DEVICE_NAME		"NEC PC-8801MA"
#define CONFIG_NAME		"pc8801ma"
#elif defined(_PC8801MK2)
#define DEVICE_NAME		"NEC PC-8801mkII"
#define CONFIG_NAME		"pc8801mk2"
#elif defined(_PC8801)
#define DEVICE_NAME		"NEC PC-8801"
#define CONFIG_NAME		"pc8801"
#elif defined(_PC8001SR)
#define DEVICE_NAME		"NEC PC-8001mkIISR"
#define CONFIG_NAME		"pc8001mk2sr"
#elif defined(_PC8001MK2)
#define DEVICE_NAME		"NEC PC-8001mkII"
#define CONFIG_NAME		"pc8001mk2"
#elif defined(_PC8001)
#define DEVICE_NAME		"NEC PC-8001"
#define CONFIG_NAME		"pc8001"
#endif

#if defined(_PC8001) || defined(_PC8001MK2) || defined(_PC8001SR)
#define PC8001_VARIANT
#else
#define PC8801_VARIANT
#endif
#if defined(_PC8801MA)
#define PC8801SR_VARIANT
#endif

#if defined(PC8001_VARIANT)
	#define MODE_PC80_V1	0
	#define MODE_PC80_V2	1
	#define MODE_PC80_N	2
#else
	#define MODE_PC88_V1S	0
	#define MODE_PC88_V1H	1
	#define MODE_PC88_V2	2
	#define MODE_PC88_N	3
	#define MODE_PC88_V2CD	4
#endif

#if defined(_PC8801MA)
	#define SUPPORT_PC88_KANJI1
	#define SUPPORT_PC88_KANJI2
	#define SUPPORT_PC88_DICTIONARY
	#define SUPPORT_PC88_HIGH_CLOCK
	#define SUPPORT_PC88_OPN1
	#define SUPPORT_PC88_OPN2
	#define SUPPORT_PC88_OPNA
	#define SUPPORT_PC88_FDD_8INCH
	#define SUPPORT_PC88_CDROM
	#define SUPPORT_PC88_VAB
	#define SUPPORT_PC88_HMB20
	#define SUPPORT_PC88_JOYSTICK
	#if defined(SUPPORT_PC88_VAB)
		// X88000
//		#define PC88_EXRAM_BANKS	8
//		#define PC88_VAB_PAGE		1
		#define PC88_EXRAM_BANKS	4
		#define PC88_VAB_PAGE		0
	#else
		#define PC88_EXRAM_BANKS	4
	#endif
	#define HAS_UPD4990A
	#define SUPPORT_M88_DISKDRV
#elif defined(_PC8801MK2)
	#define SUPPORT_PC88_KANJI1
//	#define SUPPORT_PC88_KANJI2
	#define SUPPORT_PC88_OPN2
	#define SUPPORT_PC88_FDD_8INCH
	#define SUPPORT_M88_DISKDRV
#elif defined(_PC8801)
	#define SUPPORT_PC88_KANJI1
//	#define SUPPORT_PC88_KANJI2
	#define SUPPORT_PC88_OPN2
	#define SUPPORT_PC88_FDD_8INCH
	#define SUPPORT_M88_DISKDRV
#elif defined(_PC8001SR)
	#define SUPPORT_PC88_KANJI1
//	#define SUPPORT_PC88_KANJI2
	#define SUPPORT_PC88_OPN1
	#define SUPPORT_PC88_OPN2
	#define PC88_EXRAM_BANKS	1
	#define SUPPORT_PC88_FDD_8INCH
	#define SUPPORT_M88_DISKDRV
#elif defined(_PC8001MK2)
	#define SUPPORT_PC88_KANJI1
//	#define SUPPORT_PC88_KANJI2
	#define SUPPORT_PC88_OPN2
	#define PC88_EXRAM_BANKS	1
	#define SUPPORT_PC88_FDD_8INCH
	#define SUPPORT_M88_DISKDRV
#elif defined(_PC8001)
//	#define SUPPORT_PC88_KANJI1
//	#define SUPPORT_PC88_KANJI2
//	#define SUPPORT_PC88_FDD_8INCH
//	#define SUPPORT_M88_DISKDRV
#endif
#define SUPPORT_PC88_GSX8800
#define SUPPORT_PC88_PCG8100
#define SUPPORT_QUASIS88_CMT

// device informations for virtual machine
#define FRAMES_PER_SEC		62.422
#define LINES_PER_FRAME 	260
#define CPU_CLOCKS		3993624
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		2
#define UPD765A_NO_ST1_EN_OR_FOR_RESULT7
#if defined(_PC8801MA)
#define PC80S31K_NO_WAIT
#endif
#if defined(SUPPORT_PC88_CDROM)
#define SCSI_HOST_AUTO_ACK
#define SCSI_DEV_IMMEDIATE_SELECT
#endif
#define Z80_MEMORY_WAIT
#define OVERRIDE_SOUND_FREQ_48000HZ	55467

// device informations for win32
#if defined(PC8001_VARIANT)
#define USE_BOOT_MODE		3
#define USE_CPU_TYPE		2
#else
#define USE_BOOT_MODE		5
#define USE_CPU_TYPE		3
#endif
#if defined(_PC8801MA)
// V2 mode, 4MHz
#define BOOT_MODE_DEFAULT	2
#define CPU_TYPE_DEFAULT	1
#endif
#define USE_DIPSWITCH
#define DIPSWITCH_MEMWAIT	0x01
#define DIPSWITCH_HMB20		0x02
#define DIPSWITCH_GSX8800	0x04
#define DIPSWITCH_PCG8100	0x08
#define DIPSWITCH_CMDSING	0x10
#define DIPSWITCH_PALETTE	0x20
#define DIPSWITCH_FDD_5INCH	0x40
#define DIPSWITCH_FDD_8INCH	0x80
#define DIPSWITCH_M88_DISKDRV	0x100
#define DIPSWITCH_QUASIS88_CMT	0x200
#define DIPSWITCH_DEFAULT	(DIPSWITCH_HMB20 + DIPSWITCH_GSX8800 + DIPSWITCH_PCG8100 + DIPSWITCH_CMDSING + DIPSWITCH_FDD_5INCH)
#define USE_JOYSTICK_TYPE	2
#if defined(SUPPORT_PC88_FDD_8INCH)
#define USE_FLOPPY_DISK		4
#else
#define USE_FLOPPY_DISK		2
#endif
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#if defined(SUPPORT_PC88_CDROM)
#define USE_COMPACT_DISC	1
#endif
#define USE_KEY_LOCKED
// slow enough for N88-“ú–{ŒêBASIC
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	2
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#if defined(_PC8801MA)
	#define USE_SOUND_TYPE		6	// OPNA,OPN,OPN+OPNA,OPN+OPN,OPNA+OPNA,OPNA+OPN
#elif defined(_PC8001SR)
	#define USE_SOUND_TYPE		2	// OPN,OPN+OPN
#elif defined(_PC8001MK2) || defined(_PC8801) || defined(_PC8801MK2)
	#define USE_SOUND_TYPE		2	// None,OPN
#endif
#if defined(SUPPORT_PC88_OPN1)
	#if defined(SUPPORT_PC88_OPNA)
		#define SOUND_VOLUME_OPN1	4
	#else
		#define SOUND_VOLUME_OPN1	2
	#endif
#else
	#define SOUND_VOLUME_OPN1	0
#endif
#if defined(SUPPORT_PC88_OPN2)
	#if defined(SUPPORT_PC88_OPNA)
		#define SOUND_VOLUME_OPN2	4
	#else
		#define SOUND_VOLUME_OPN2	2
	#endif
#else
	#define SOUND_VOLUME_OPN2	0
#endif
#if defined(SUPPORT_PC88_CDROM)
	#define SOUND_VOLUME_CDROM	1
#else
	#define SOUND_VOLUME_CDROM	0
#endif
#if defined(SUPPORT_PC88_HMB20)
	#define SOUND_VOLUME_HMB20	1
#else
	#define SOUND_VOLUME_HMB20	0
#endif
#if defined(SUPPORT_PC88_GSX8800)
	#define SOUD_VOLUME_GSX8800	1
#else
	#define SOUD_VOLUME_GSX8800	0
#endif
#if defined(SUPPORT_PC88_PCG8100)
	#define SOUND_VOLUME_PCG8100	1
#else
	#define SOUND_VOLUME_PCG8100	0
#endif
#define USE_SOUND_VOLUME	(SOUND_VOLUME_OPN1 + SOUND_VOLUME_OPN2 + SOUND_VOLUME_CDROM + SOUND_VOLUME_HMB20 + SOUD_VOLUME_GSX8800 + SOUND_VOLUME_PCG8100 + 1 + 1)
#define USE_JOYSTICK
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[USE_SOUND_VOLUME] = {
#ifdef SUPPORT_PC88_OPN1
	#ifdef SUPPORT_PC88_OPN2
		#define NUM1 " #1 "
	#else
		#define NUM1 " "
	#endif
#ifdef SUPPORT_PC88_OPNA
	_T("OPNA") _T(NUM1) _T("(FM)"), _T("OPNA") _T(NUM1) _T("(PSG)"), _T("OPNA") _T(NUM1) _T("(ADPCM)"), _T("OPNA") _T(NUM1) _T("(Rhythm)"),
#else
	_T("OPN") _T(NUM1) _T("(FM)"), _T("OPN") _T(NUM1) _T("(PSG)"),
#endif
#endif
#ifdef SUPPORT_PC88_OPN2
	#ifdef SUPPORT_PC88_OPN1
		#define NUM2 " #2 "
	#else
		#define NUM2 " "
	#endif
#ifdef SUPPORT_PC88_OPNA
	_T("OPNA") _T(NUM2) _T("(FM)"), _T("OPNA") _T(NUM2) _T("(PSG)"), _T("OPNA") _T(NUM2) _T("(ADPCM)"), _T("OPNA") _T(NUM2) _T("(Rhythm)"),
#else
	_T("OPN") _T(NUM2) _T("(FM)"), _T("OPN") _T(NUM2) _T("(PSG)"),
#endif
#endif
#ifdef SUPPORT_PC88_CDROM
	_T("CD-DA"),
#endif
#ifdef SUPPORT_PC88_HMB20
	_T("HMB-20"),
#endif
#ifdef SUPPORT_PC88_GSX8800
	_T("GSX-8800"),
#endif
#ifdef SUPPORT_PC88_PCG8100
	_T("PCG-8100"),
#endif
	_T("Beep"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DISK;
class I8251;
class I8255;
class NOISE;
class PCM1BIT;
class UPD1990A;
#if defined(SUPPORT_PC88_OPN1) || defined(SUPPORT_PC88_OPN2)
class YM2203;
#endif
class Z80;

class PC80S31K;
class UPD765A;

#ifdef SUPPORT_PC88_CDROM
class SCSI_HOST;
class SCSI_CDROM;
#endif

#ifdef SUPPORT_PC88_HMB20
class YM2151;
#endif

#ifdef SUPPORT_PC88_GSX8800
class AY_3_891X;
#endif

#if defined(SUPPORT_PC88_GSX8800) || defined(SUPPORT_PC88_PCG8100)
class I8253;
#endif

#ifdef SUPPORT_M88_DISKDRV
class DiskIO;
#endif

class PC88;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* pc88event;
	
	DEVICE* pc88prn;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
#ifdef SUPPORT_PC88_OPN1
	YM2203* pc88opn1;
#endif
#ifdef SUPPORT_PC88_OPN2
	YM2203* pc88opn2;
#endif
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	NOISE* pc88noise_seek;
	NOISE* pc88noise_head_down;
	NOISE* pc88noise_head_up;
	Z80* pc88cpu_sub;
	
#ifdef SUPPORT_PC88_FDD_8INCH
	UPD765A* pc88fdc_8inch;
	NOISE* pc88noise_8inch_seek;
	NOISE* pc88noise_8inch_head_down;
	NOISE* pc88noise_8inch_head_up;
#endif
	
#ifdef SUPPORT_PC88_CDROM
	SCSI_HOST* pc88scsi_host;
	SCSI_CDROM* pc88scsi_cdrom;
#endif
	
#ifdef SUPPORT_PC88_HMB20
	YM2151* pc88opm;
#endif
	
#ifdef SUPPORT_PC88_GSX8800
//	I8253* pc88gsx_pit;
	AY_3_891X* pc88gsx_psg1;
	AY_3_891X* pc88gsx_psg2;
	AY_3_891X* pc88gsx_psg3;
	AY_3_891X* pc88gsx_psg4;
#endif
	
#ifdef SUPPORT_PC88_PCG8100
	I8253* pc88pcg_pit;
	PCM1BIT* pc88pcg_pcm1;
	PCM1BIT* pc88pcg_pcm2;
	PCM1BIT* pc88pcg_pcm3;
#endif
	
#ifdef SUPPORT_M88_DISKDRV
	DiskIO* pc88diskio;
#endif
	
	PC88* pc88;
	
	int boot_mode;
	
	// drives
	UPD765A *get_floppy_disk_controller(int drv);
	DISK *get_floppy_disk_handler(int drv);
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	double get_frame_rate();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_connected(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
#ifdef SUPPORT_PC88_CDROM
	void open_compact_disc(int drv, const _TCHAR* file_path);
	void close_compact_disc(int drv);
	bool is_compact_disc_inserted(int drv);
	uint32_t is_compact_disc_accessed();
#endif
	bool is_frame_skippable();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
