/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ virtual machine ]
*/

#ifndef _PASOPIA7_H_
#define _PASOPIA7_H_

#ifdef _LCD
#define DEVICE_NAME		"TOSHIBA PASOPIA 7 with LCD"
#define CONFIG_NAME		"pasopia7lcd"
#else
#define DEVICE_NAME		"TOSHIBA PASOPIA 7"
#define CONFIG_NAME		"pasopia7"
#endif

// device informations for virtual machine
#ifdef _LCD
#define FRAMES_PER_SEC		74.38
#define LINES_PER_FRAME 	32
#define CHARS_PER_LINE		94
#define HD46505_HORIZ_FREQ	2380
#else
#define FRAMES_PER_SEC		59.92
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		57
#define HD46505_HORIZ_FREQ	15700
#endif
#define CPU_CLOCKS		3993600
#ifdef _LCD
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		128
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#endif
#define MAX_DRIVE		4
#define IO_ADDR_MAX		0x100
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_TAPE
#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
#define USE_BINARY_FILE1
#define USE_BINARY_FILE2
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class LS393;
class NOT;
class PCM1BIT;
class SN76489AN;
class UPD765A;
class Z80;
class Z80CTC;
class Z80PIO;

class FLOPPY;
class DISPLAY;
class IO;
class IOTRAP;
class KEYBOARD;
class MEMORY;
class PAC2;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	HD46505* crtc;
	I8255* pio0;
	I8255* pio1;
	I8255* pio2;
	LS393* flipflop;
	NOT* not;
	PCM1BIT* pcm;
	SN76489AN* psg0;
	SN76489AN* psg1;
	UPD765A* fdc;
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIO* pio;
	
	FLOPPY* floppy;
	DISPLAY* display;
	IO* io;
	IOTRAP* iotrap;
	KEYBOARD* key;
	MEMORY* memory;
	PAC2* pac2;
	
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
	void load_binary(int drv, _TCHAR* file_path);
	void save_binary(int drv, _TCHAR* file_path) {}
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
