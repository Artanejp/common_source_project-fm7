/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ virtual machine ]
*/

#ifndef _MZ2800_H_
#define _MZ2800_H_

#define DEVICE_NAME		"SHARP MZ-2800"
#define CONFIG_NAME		"mz2800"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define HAS_I286
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define HAS_RP5C15
#define IO_ADDR_MAX		0x8000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class I8253;
class I8255;
class I8259;
class I286;
class IO;
class MB8877;
class PCM1BIT;
class RP5C01;
//class SASI;
class UPD71071;
class YM2203;
class Z80PIO;
class Z80SIO;

class CRTC;
class FLOPPY;
class JOYSTICK;
class KEYBOARD;
class MEMORY;
class MOUSE;
class RESET;
class SERIAL;
class SYSPORT;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8253* pit;
	I8255* pio0;
	I8259* pic;
	I286* cpu;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	RP5C01* rtc;
//	SASI* sasi;
	UPD71071* dma;
	YM2203* opn;
	Z80PIO* pio1;
	Z80SIO* sio;
	
	CRTC* crtc;
	FLOPPY* floppy;
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MOUSE* mouse;
	RESET* rst;
	SERIAL* serial;
	SYSPORT* sysport;
	
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
	void cpu_reset();
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
