/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ virtual machine ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#ifndef _Z80TVGAME_H_
#define _Z80TVGAME_H_

#ifdef _USE_I8255
#define DEVICE_NAME		"Homebrew Z80 TV GAME SYSTEM (i8255)"
#define CONFIG_NAME		"z80tvgame_i8255"
#else
#define DEVICE_NAME		"Homebrew Z80 TV GAME SYSTEM (Z80PIO)"
#define CONFIG_NAME		"z80tvgame_z80pio"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		176
#define SCREEN_WIDTH_ASPECT	280
#define SCREEN_HEIGHT		210

// device informations for win32
#define USE_CART1
#define USE_KEY_TO_JOY
#define KEY_TO_JOY_BUTTON_1	0x5a
#define KEY_TO_JOY_BUTTON_2	0x58
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

class EMU;
class DEVICE;
class EVENT;

#ifdef _USE_I8255
class I8255;
#else
class Z80PIO;
#endif
class PCM1BIT;
class Z80;

class JOYSTICK;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
#ifdef _USE_I8255
	I8255* pio;
#else
	Z80PIO* pio;
#endif
	PCM1BIT* pcm;
	Z80* cpu;
	
	JOYSTICK* joystick;
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
