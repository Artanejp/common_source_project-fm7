/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ virtual machine ]
*/

#ifndef _JX_H_
#define _JX_H_

#define DEVICE_NAME		"IBM Japan Ltd PC/JX"
#define CONFIG_NAME		"jx"

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		59.9
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		57
#define CPU_CLOCKS		4772727
//#define CPU_CLOCKS		4770000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define UPD765A_SENCE_INTSTAT_RESULT
#define HAS_I86
#define I8259_MAX_CHIPS		1
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x4000
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#define KEYBOARD_HACK
#define TIMER_HACK

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HD46505;
class I8251;
class I8253;
class I8255;
class I8259;
class I86;
class IO;
class MEMORY;
class NOT;
class PCM1BIT;
class SN76489AN;
class UPD765A;

class DISPLAY;
class FLOPPY;
class KEYBOARD;
class SPEAKER;

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
	I86* cpu;
	IO* io;
	MEMORY* mem;
	NOT* not;
	PCM1BIT* pcm;
	SN76489AN* psg;
	UPD765A* fdc;
	
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	SPEAKER* speaker;
	
	// memory
	uint8 font[0x800];
	uint8 kanji[0x38000];
	uint8 ram[0x80000];
	uint8 ipl[0x30000];
	
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
	void notify_power_off();
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
