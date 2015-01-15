/*
	SEGA SC-3000 Emulator 'eSC-3000'

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

	[ virtual machine ]
*/

#ifndef _SC3000_H_
#define _SC3000_H_

#define DEVICE_NAME		"SEGA SC-3000"
#define CONFIG_NAME		"sc3000"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3579545
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
#define TMS9918A_VRAM_SIZE	0x4000
#define TMS9918A_LIMIT_SPRITES
#define MAX_DRIVE		4

// device informations for win32
#define USE_CART1
#define USE_FD1
#define USE_TAPE
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	8
#define USE_AUTO_KEY_CAPS
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8251;
class I8255;
class IO;
class SN76489AN;
class TMS9918A;
class UPD765A;
class Z80;

class KEYBOARD;
class MEMORY;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8251* sio;
	I8255* pio_k;
	I8255* pio_f;
	IO* io;
	SN76489AN* psg;
	TMS9918A* vdp;
	UPD765A* fdc;
	Z80* cpu;
	
	KEYBOARD* key;
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
	
	// user interface
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
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
