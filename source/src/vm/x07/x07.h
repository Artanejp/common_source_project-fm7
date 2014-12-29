/*
	CANON X-07 Emulator 'eX-07'

	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ virtual machine ]
*/

#ifndef _X07_H_
#define _X07_H_

#define DEVICE_NAME		"CANON X-07"
#define CONFIG_NAME		"x07"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3840000
#define CPU_START_ADDR		0xc3c3
#define SCREEN_WIDTH		120
#define SCREEN_HEIGHT		32
#define TV_SCREEN_WIDTH		256
#define TV_SCREEN_HEIGHT	192
#define HAS_NSC800
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x800

// device informations for win32
#define WINDOW_WIDTH		(SCREEN_WIDTH * 2)
#define WINDOW_HEIGHT		(SCREEN_HEIGHT * 2)
#define TV_WINDOW_WIDTH		TV_SCREEN_WIDTH
#define TV_WINDOW_HEIGHT	TV_SCREEN_HEIGHT

#define USE_TAPE
#define TAPE_BINARY_ONLY
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class MEMORY;
class Z80;

class IO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	MEMORY* memory;
	Z80* cpu;
	
	IO* io;
	
	// memory
//	uint8 c3[0x2000];
	uint8 ram[0x6000];
	uint8 app[0x2000];
	uint8 vram[0x1800];
	uint8 tv[0x1000];
	uint8 bas[0x5000];
	
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
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
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
