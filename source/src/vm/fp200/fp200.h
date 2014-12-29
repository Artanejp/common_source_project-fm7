/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ virtual machine ]
*/

#ifndef _FP200_H_
#define _FP200_H_

#define DEVICE_NAME		"CASIO FP-200"
#define CONFIG_NAME		"fp200"

// device informations for virtual machine
#define FRAMES_PER_SEC		64
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		3077000
#define SCREEN_WIDTH		160
#define SCREEN_HEIGHT		64
#define HAS_I8085
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x2000

// device informations for win32
#define WINDOW_WIDTH		(SCREEN_WIDTH * 2)
#define WINDOW_HEIGHT		(SCREEN_HEIGHT * 2)

#define USE_BOOT_MODE		2
#define USE_TAPE
#define TAPE_BINARY_ONLY
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8080;
class MEMORY;
class RP5C01;

class IO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8080* cpu;
	MEMORY* memory;
	RP5C01* rtc;
	
	IO* io;
	
	// memory
	uint8 rom[0x8000];
	uint8 ram[0x8000];
	
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
