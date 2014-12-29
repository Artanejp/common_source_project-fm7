/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ virtual machine ]
*/

#ifndef _MZ3500_H_
#define _MZ3500_H_

#define DEVICE_NAME		"SHARP MZ-3500"
#define CONFIG_NAME		"mz3500"

// device informations for virtual machine
#define FRAMES_PER_SEC		47.3
#define LINES_PER_FRAME 	441
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define UPD765A_WAIT_RESULT7
#define UPD765A_EXT_DRVSEL
#define UPD7220_HORIZ_FREQ	20860
#define IO_ADDR_MAX		0x100
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
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
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8253;
class I8255;
class IO;
class PCM1BIT;
class UPD1990A;
class UPD7220;
class UPD765A;
class Z80;

class MAIN;
class SUB;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	// for main cpu
	IO* io;
	UPD765A* fdc;
	Z80* cpu;
	MAIN* main;
	
	// for sub cpu
	I8251* sio;
	I8253* pit;
	I8255* pio;
	IO* subio;
	PCM1BIT* pcm;
	UPD1990A* rtc;
	UPD7220* gdc_chr;
	UPD7220* gdc_gfx;
	Z80* subcpu;
	SUB* sub;
	
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
