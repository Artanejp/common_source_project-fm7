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
#define PC88_EXRAM_BANKS	4
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
#define MAX_DRIVE		2
#define UPD765A_NO_ST1_EN_OR_FOR_RESULT7
#ifdef SUPPORT_PC88_OPNA
#define HAS_YM2608
#endif
#define Z80_MEMORY_WAIT
#define PCM1BIT_HIGH_QUALITY
#define OVERRIDE_SOUND_FREQ_48000HZ	55467
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#if defined(_PC8001SR)
#define USE_BOOT_MODE		3
#else
#define USE_BOOT_MODE		4
#endif
#define USE_CPU_TYPE		2
#define USE_DIPSWITCH
#define USE_DEVICE_TYPE		2
#define USE_FD1
#define USE_FD2
#define USE_TAPE
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
// slow enough for N88-“ú–{ŒêBASIC
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_MONITOR_TYPE	2
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class I8251;
class I8255;
class PCM1BIT;
class UPD1990A;
class YM2203;
class Z80;

class PC80S31K;
class UPD765A;

#ifdef SUPPORT_PC88_PCG8100
class I8253;
#endif

class PC88;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* pc88event;
	
	BEEP* pc88beep;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
	YM2203* pc88opn;
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	Z80* pc88cpu_sub;
	
#ifdef SUPPORT_PC88_PCG8100
	I8253* pc88pit;
	PCM1BIT* pc88pcm0;
	PCM1BIT* pc88pcm1;
	PCM1BIT* pc88pcm2;
#endif
	
	PC88* pc88;
	
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
	double frame_rate();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool now_skip();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
