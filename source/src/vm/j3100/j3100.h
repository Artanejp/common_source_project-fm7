/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ virtual machine ]
*/

#ifndef _J3100_H_
#define _J3100_H_

#if defined(_J3100GT)
#define DEVICE_NAME		"TOSHIBA J-3100GT"
#define CONFIG_NAME		"j3100gt"
#elif defined(_J3100SL)
#define DEVICE_NAME		"TOSHIBA J-3100SL"
#define CONFIG_NAME		"j3100sl"
#endif

// device informations for virtual machine

#if defined(_J3100SL) || defined(_J3100SS) || defined(_J3100SE)
#define TYPE_SL
#endif

// TODO: check refresh rate
#define FRAMES_PER_SEC		59.9
// óví≤ç∏
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		54
#define CPU_CLOCKS		9545456
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define UPD765A_SENCE_INTSTAT_RESULT
#define UPD765A_EXT_DRVSEL
#ifdef TYPE_SL
#define HAS_I86
#define I8259_MAX_CHIPS		1
#else
#define HAS_I286
#define I8259_MAX_CHIPS		2
#endif
#if !(defined(_J3100SS) || defined(_J3100SE))
#define HAS_I8254
#endif
#define SINGLE_MODE_DMA
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_ACCESS_LAMP

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HD46505;
class I8237;
//class I8250;
class I8253;
class I8259;
class I86;
class IO;
class PCM1BIT;
class UPD765A;
#ifdef TYPE_SL
class RP5C01;
#else
class HD146818P;
#endif

class DISPLAY;
class DMAREG;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class SASI;
class SYSTEM;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
	I8237* dma;
//	I8250* sio;
	I8253* pit;
	I8259* pic;
	I86* cpu;
	IO* io;
	PCM1BIT* pcm;
	UPD765A* fdc;
#ifdef TYPE_SL
	RP5C01* rtc;
#else
	HD146818P* rtc;
	I8237* dma2;
#endif
	
	DISPLAY* display;
	DMAREG* dmareg;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SASI* sasi;
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
	void notify_power_off();
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
