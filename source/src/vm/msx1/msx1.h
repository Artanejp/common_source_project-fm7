/*
	ASCII MSX1 Emulator 'yaMSX1'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya, umaiboux

	[ virtual machine ]
*/

#ifndef _MSX1_H_
#define _MSX1_H_

#ifdef _PX7
#define DEVICE_NAME		"PIONEER PX-7"
#define CONFIG_NAME		"px7"
#else
#define DEVICE_NAME		"ASCII MSX1"
#define CONFIG_NAME		"msx1"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3579545
//#define SCREEN_WIDTH		256
//#define SCREEN_HEIGHT		192
#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		384
#define TMS9918A_VRAM_SIZE	0x4000
#define TMS9918A_LIMIT_SPRITES
#define TMS9918A_SUPER_IMPOSE
#define HAS_AY_3_8910
// for Flappy Limited '85
#define YM2203_PORT_MODE	0x80

// device informations for win32
#define USE_CART1
#define USE_CART2
#define USE_TAPE
#define USE_LASER_DISC
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8255;
class IO;
class LD700;
class NOT;
class YM2203;
class PCM1BIT;
class TMS9918A;
class Z80;

class JOYSTICK;
class KEYBOARD;
class MEMORY;
class SLOT_CART;
class SLOT_MAIN;
class SLOT_SUB;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8255* pio;
	IO* io;
	LD700* ldp;
	NOT* not;
	YM2203* psg;
	PCM1BIT* pcm;
	TMS9918A* vdp;
	Z80* cpu;
	
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SLOT_MAIN *slot0;
	SLOT_CART *slot1;
	SLOT_SUB *slot2;
	SLOT_CART *slot3;
	
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
	void movie_sound_callback(uint8 *buffer, long size);
	
	// user interface
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	void open_laser_disc(_TCHAR* file_path);
	void close_laser_disc();
	bool laser_disc_inserted();
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
