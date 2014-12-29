/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.13 -

	[ virtual machine ]
*/

#ifndef _QC10_H_
#define _QC10_H_

#ifdef _COLOR_MONITOR
#define DEVICE_NAME		"EPSON QC-10 with color monitor subboard"
#define CONFIG_NAME		"qc10cms"
#else
#define DEVICE_NAME		"EPSON QC-10"
#define CONFIG_NAME		"qc10"
#endif

// device informations for virtual machine
#ifdef _COLOR_MONITOR
#define FRAMES_PER_SEC		56.92
#define LINES_PER_FRAME 	441
#define UPD7220_HORIZ_FREQ	25100
#else
#define FRAMES_PER_SEC		45.84
#define LINES_PER_FRAME 	421
#define UPD7220_HORIZ_FREQ	19300
#endif
#define CPU_CLOCKS		3993600
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define I8259_MAX_CHIPS		2
#define UPD7201
#define UPD7220_FIXED_PITCH
#define UPD765A_DMA_MODE
//#define SINGLE_MODE_DMA
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x1f
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#ifdef _COLOR_MONITOR
#define USE_CRT_FILTER
#endif
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HD146818P;
class I8237;
class I8253;
class I8255;
class I8259;
class IO;
class PCM1BIT;
class UPD7220;
class UPD765A;
class Z80;
class Z80SIO;

class DISPLAY;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class MFONT;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD146818P* rtc;
	I8237* dma0;
	I8237* dma1;
	I8253* pit0;
	I8253* pit1;
	I8255* pio;
	I8259* pic;	// includes 2chips
	IO* io;
	PCM1BIT* pcm;
	UPD7220* gdc;
	UPD765A* fdc;
	Z80* cpu;
	Z80SIO* sio;
	
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MFONT* mfont;
	
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
