/*
	TOMY PyuTa Emulator 'ePyuTa'

	Author : Takeda.Toshiya
	Date   : 2007.07.15 -

	[ virtual machine ]
*/

#ifndef _PYUTA_H_
#define _PYUTA_H_

#define DEVICE_NAME		"TOMY PyuTa"
#define CONFIG_NAME		"pyuta"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		10700000
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
#define TMS9918A_VRAM_SIZE	0x4000
//#define TMS9918A_LIMIT_SPRITES

// device informations for win32
#define USE_CART1
#define USE_TAPE
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class SN76489AN;
class TMS9918A;
class TMS9995;

class MEMORY;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	SN76489AN* psg;
	TMS9918A* vdp;
	TMS9995* cpu;
	
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
