/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ virtual machine ]
*/

#ifndef _SCV_H_
#define _SCV_H_

#define DEVICE_NAME		"EPOCH SCV"
#define CONFIG_NAME		"scv"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		192
#define SCREEN_WIDTH_ASPECT	288
#define SCREEN_HEIGHT		222

// memory wait
//#define UPD7801_MEMORY_WAIT

// device informations for win32
#define USE_CART1
#define USE_KEY_TO_JOY
#define KEY_TO_JOY_BUTTON_1	0x5a
#define KEY_TO_JOY_BUTTON_2	0x58
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class UPD7801;

class IO;
class MEMORY;
class SOUND;
class VDP;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	UPD7801* cpu;
	
	IO* io;
	MEMORY* memory;
	SOUND* sound;
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
