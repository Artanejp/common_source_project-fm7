/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ virtual machine ]
*/

#ifndef _RX78_H_
#define _RX78_H_

#define DEVICE_NAME		"BANDAI RX-78"
#define CONFIG_NAME		"rx78"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4090909
#define SCREEN_WIDTH		192
#define SCREEN_WIDTH_ASPECT	288
#define SCREEN_HEIGHT		184

// device informations for win32
#define USE_CART1
#define USE_TAPE
#define USE_KEY_TO_JOY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class IO;
class SN76489AN;
class Z80;

class CMT;
class KEYBOARD;
class MEMORY;
class PRINTER;
class VDP;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	IO* io;
	SN76489AN* psg;
	Z80* cpu;
	
	CMT* cmt;
	KEYBOARD* key;
	MEMORY* memory;
	PRINTER* prt;
	VDP* vdp;
	
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
	
	// user interface
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
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
