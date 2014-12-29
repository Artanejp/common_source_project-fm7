/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ virtual machine ]
*/

#ifndef _MULTI8_H_
#define _MULTI8_H_

#define DEVICE_NAME		"MITSUBISHI Electric MULTI 8"
#define CONFIG_NAME		"multi8"

// device informations for virtual machine
#define FRAMES_PER_SEC		60.58
#define LINES_PER_FRAME 	260
#define CHARS_PER_LINE		112
#define HD46505_HORIZ_FREQ	15750
#define CPU_CLOCKS		3993600
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define I8259_MAX_CHIPS		1
#define MAX_DRIVE		4
#define UPD765A_DONT_WAIT_SEEK
#define HAS_AY_3_8912
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_TAPE
#define TAPE_BINARY_ONLY
#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HD46505;
class I8251;
class I8253;
class I8255;
class I8259;
class IO;
class UPD765A;
class YM2203;
class Z80;

class CMT;
class DISPLAY;
class FLOPPY;
class KANJI;
class KEYBOARD;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
	I8251* sio;
	I8253* pit;
	I8255* pio;
	I8259* pic;
	IO* io;
	UPD765A* fdc;
	YM2203* psg;
	Z80* cpu;
	
	CMT* cmt;
	DISPLAY* display;
	FLOPPY* floppy;
	KANJI* kanji;
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
	double frame_rate();
	
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
