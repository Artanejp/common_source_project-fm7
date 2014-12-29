/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ virtual machine ]
*/

#ifndef _MZ5500_H_
#define _MZ5500_H_

#if defined(_MZ5500)
#define DEVICE_NAME		"SHARP MZ-5500"
#define CONFIG_NAME		"mz5500"
#elif defined(_MZ6500)
#define DEVICE_NAME		"SHARP MZ-6500"
#define CONFIG_NAME		"mz6500"
#elif defined(_MZ6550)
#define DEVICE_NAME		"SHARP MZ-6550"
#define CONFIG_NAME		"mz6550"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.49
#define LINES_PER_FRAME 	448
#if defined(_MZ5500)
#define CPU_CLOCKS		4915200
#elif defined(_MZ6500) || defined(_MZ6550)
#define CPU_CLOCKS		8000000
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#ifdef _MZ6550
#define HAS_I286
#else
#define HAS_I86
#endif
#define I8259_MAX_CHIPS		2
#define UPD7220_HORIZ_FREQ	24860
#define Z80CTC_CLOCKS		2457600
#define SINGLE_MODE_DMA
#define IO_ADDR_MAX		0x400
#define HAS_AY_3_8912
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8255;
class I8259;
class I286;
class IO;
class LS393;
class RP5C01;
class UPD7220;
class UPD765A;
class YM2203;
class Z80CTC;
class Z80SIO;

class DISPLAY;
class KEYBOARD;
class MEMORY;
class SYSPORT;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8237* dma;
	I8255* pio;
	I8259* pic;	// includes 2chips
	I286* cpu;
	IO* io;
	LS393* div;
	RP5C01* rtc;
	UPD7220* gdc;
	UPD765A* fdc;
	YM2203* psg;
	Z80CTC* ctc0;
#if defined(_MZ6500) || defined(_MZ6550)
	Z80CTC* ctc1;
#endif
	Z80SIO* sio;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SYSPORT* sysport;
	
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
	void special_reset();
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
	bool now_skip();
	
	void update_config();
	
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
