/*
	NEC PC-8801MA Emulator 'ePC-8801MA'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'

	Author : Takeda.Toshiya
	Date   : 2012.02.16-

	[ virtual machine ]
*/

#ifndef _PC8801_H_
#define _PC8801_H_

#if defined(_PC8801MA)
#define DEVICE_NAME		"NEC PC-8801MA"
#define CONFIG_NAME		"pc8801ma"
#elif defined(_PC8001SR)
#define DEVICE_NAME		"NEC PC-8001mkIISR"
#define CONFIG_NAME		"pc8001mk2sr"
#endif

#if defined(_PC8001SR)
#define MODE_PC80_V1	0
#define MODE_PC80_V2	1
#define MODE_PC80_N	2
#else
#define MODE_PC88_V1S	0
#define MODE_PC88_V1H	1
#define MODE_PC88_V2	2
#define MODE_PC88_N	3
#endif

#if defined(_PC8801MA)
#define SUPPORT_PC88_DICTIONARY
#define SUPPORT_PC88_HIGH_CLOCK
#define SUPPORT_PC88_OPNA
#define SUPPORT_PC88_SB2
#define SUPPORT_PC88_CDROM
#define SUPPORT_PC88_VAB
#define SUPPORT_PC88_HMB20
#if defined(SUPPORT_PC88_VAB)
// X88000
#define PC88_EXRAM_BANKS	8
#define PC88_VAB_PAGE		1
#else
#define PC88_EXRAM_BANKS	4
#endif
#define HAS_UPD4990A
#endif
#define SUPPORT_PC88_JOYSTICK
#define SUPPORT_PC88_PCG8100

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
#if defined(_PC8001SR)
#define USE_BOOT_MODE		3
#define USE_CPU_TYPE		2
#else
#define USE_BOOT_MODE		4
#define USE_CPU_TYPE		3
#endif
#if defined(_PC8801MA)
// V2 mode, 4MHz
#define BOOT_MODE_DEFAULT	2
#define CPU_TYPE_DEFAULT	1
#endif
#define USE_DIPSWITCH
#define USE_JOYSTICK_TYPE	2
#define USE_FLOPPY_DISK		2
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
#ifdef SUPPORT_PC88_OPNA
#ifdef SUPPORT_PC88_SB2
#define USE_SOUND_TYPE		6
#else
#define USE_SOUND_TYPE		2
#endif
#endif

#undef _SOUNDS_OPN
#undef _SOUNDS_SB2
#undef _SOUNDS_PCG8100
#undef _SOUNDS_CDROM
#undef _SOUNDS_HMB20
#undef _SOUNDS_BEEP
#undef _SOUNDS_NOISE

#if defined(SUPPORT_PC88_OPNA)
	#define _SOUNDS_OPN 4
#else /* ToDo: PC8001 without OPN/OPNA */
	#define _SOUNDS_OPN 2
#endif
#if defined(SUPPORT_PC88_SB2)
	#if defined(SUPPORT_PC88_OPNA)
	#define _SOUNDS_SB2 4
	#else /* SB2=OPN */
	#define _SOUNDS_SB2 2
	#endif
#else /* Have not SB2 */
	#define _SOUNDS_SB2 0
#endif
#if defined(SUPPORT_PC88_PCG8100)
	#define _SOUNDS_PCG8100 1
#else
	#define _SOUNDS_PCG8100 0
#endif
#if defined(SUPPORT_PC88_CDROM)
	#define _SOUNDS_CDROM 1
#else
	#define _SOUNDS_CDROM 0
#endif
#if defined(SUPPORT_PC88_HMB20)
	#define _SOUNDS_HMB20 1
#else
	#define _SOUNDS_HMB20 0
#endif
#define _SOUNDS_BEEP 1
#define _SOUNDS_NOISE 1

#define USE_SOUND_VOLUME (_SOUNDS_OPN + _SOUNDS_SB2 + _SOUNDS_PCG8100 + _SOUNDS_CDROM + _SOUNDS_HMB20 + _SOUNDS_BEEP + _SOUNDS_NOISE)


#define SUPPORT_TV_RENDER
#define USE_JOYSTICK
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[USE_SOUND_VOLUME] = {
#ifdef SUPPORT_PC88_OPNA
	_T("OPNA (FM)"), _T("OPNA (PSG)"), _T("OPNA (ADPCM)"), _T("OPNA (Rhythm)"),
#else
	_T("OPN (FM)"), _T("OPN (PSG)"),
#endif
#ifdef SUPPORT_PC88_SB2
#ifdef SUPPORT_PC88_OPNA
	_T("SB2 (FM)"), _T("SB2 (PSG)"), _T("SB2 (ADPCM)"), _T("SB2 (Rhythm)"),
#else
	_T("SB2 (FM)"), _T("SB2 (PSG)"),
#endif
#endif
#ifdef SUPPORT_PC88_CDROM
	_T("CD-DA"),
#endif
#ifdef SUPPORT_PC88_HMB20
	_T("HMB-20"),
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

class I8251;
class I8255;
class NOISE;
class PCM1BIT;
class UPD1990A;
class YM2203;
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

#ifdef SUPPORT_PC88_PCG8100
class I8253;
#endif
namespace PC88DEV {
	class PC88;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	EVENT* pc88event;
	
	DEVICE* pc88prn;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
	YM2203* pc88opn;
#ifdef SUPPORT_PC88_SB2
	YM2203* pc88sb2;
#endif
	DEVICE* dummycpu;
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	NOISE* pc88noise_seek;
	NOISE* pc88noise_head_down;
	NOISE* pc88noise_head_up;
	Z80* pc88cpu_sub;

#ifdef SUPPORT_PC88_CDROM
	SCSI_HOST* pc88scsi_host;
	SCSI_CDROM* pc88scsi_cdrom;
#endif

#ifdef SUPPORT_PC88_HMB20
	YM2151* pc88opm;
#endif
	
#ifdef SUPPORT_PC88_PCG8100
	I8253* pc88pit;
	PCM1BIT* pc88pcm0;
	PCM1BIT* pc88pcm1;
	PCM1BIT* pc88pcm2;
#endif
	
	PC88DEV::PC88* pc88;
	
	int boot_mode;
	
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
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
