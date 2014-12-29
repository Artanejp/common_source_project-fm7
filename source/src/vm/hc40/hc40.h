/*
	EPSON HC-40 Emulator 'eHC-40'

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ virtual machine ]
*/

#ifndef _HC40_H_
#define _HC40_H_

#define DEVICE_NAME		"EPSON HC-40"
#define CONFIG_NAME		"hc40"

// device informations for virtual machine
#define FRAMES_PER_SEC		72
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		3686400
#define SCREEN_WIDTH		240
#define SCREEN_HEIGHT		64
#define MAX_DRIVE		4

// device informations for win32
#define WINDOW_WIDTH		(SCREEN_WIDTH * 2)
#define WINDOW_HEIGHT		(SCREEN_HEIGHT * 2)

#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_TAPE
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class DATAREC;
class TF20;
class Z80;

class IO;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	DATAREC* drec;
	TF20* tf20;
	Z80* cpu;
	
	IO* io;
	MEMORY* memory;
	
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
