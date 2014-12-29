/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ virtual machine ]
*/

#ifndef _FAMILYBASIC_H_
#define _FAMILYBASIC_H_

#define DEVICE_NAME		"Nintendo Family BASIC"
#define CONFIG_NAME		"familybasic"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		1789772
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		240
#define HAS_N2A03

// device informations for win32
#define USE_BOOT_MODE		3
#define USE_TAPE
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class M6502;

class MEMORY;
class APU;
class PPU;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	M6502* cpu;
	
	MEMORY* memory;
	APU* apu;
	PPU* ppu;
	
	int boot_mode;
	
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
