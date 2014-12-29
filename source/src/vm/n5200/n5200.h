/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.03-

	[ virtual machine ]
*/

#ifndef _N5200_H_
#define _N5200_H_

#define DEVICE_NAME		"NEC N5200"
#define CONFIG_NAME		"n5200"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	550
#define CPU_CLOCKS		16000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define HAS_I386
#define I8259_MAX_CHIPS		2
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class I386;
class I8237;
class I8251;
class I8253;
class I8255;
class I8259;
class IO;
class UPD1990A;
class UPD7220;
class UPD765A;

class DISPLAY;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class SYSTEM;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	I386* cpu;
	I8237* dma;
	I8251* sio_r;
	I8251* sio_k;
	I8253* pit;
	I8255* pio_s;
	I8255* pio_p;
	I8259* pic;
	IO* io;
	UPD1990A* rtc;
	UPD7220* gdc_c;
	UPD7220* gdc_g;
	UPD765A* fdc;
	
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SYSTEM* system;
	
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
