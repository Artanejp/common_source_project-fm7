/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.28 -

	[ virtual machine ]
*/

#ifndef _FMR50_H_
#define _FMR50_H_

#if defined(_FMR50)
#if defined(HAS_I286)
#define DEVICE_NAME		"FUJITSU FMR-50 (i286)"
#define CONFIG_NAME		"fmr50_i286"
#elif defined(HAS_I386)
#define DEVICE_NAME		"FUJITSU FMR-50 (i386)"
#define CONFIG_NAME		"fmr50_i386"
#elif defined(HAS_I486)
#define DEVICE_NAME		"FUJITSU FMR-50 (i486)"
#define CONFIG_NAME		"fmr50_i486"
#elif defined(HAS_PENTIUM)
#define DEVICE_NAME		"FUJITSU FMR-250"
#define CONFIG_NAME		"fmr250"
#endif
#elif defined(_FMR60)
#if defined(HAS_I286)
#define DEVICE_NAME		"FUJITSU FMR-60"
#define CONFIG_NAME		"fmr60"
#elif defined(HAS_I386)
#define DEVICE_NAME		"FUJITSU FMR-70"
#define CONFIG_NAME		"fmr70"
#elif defined(HAS_I486)
#define DEVICE_NAME		"FUJITSU FMR-80"
#define CONFIG_NAME		"fmr80"
#elif defined(HAS_PENTIUM)
#define DEVICE_NAME		"FUJITSU FMR-280"
#define CONFIG_NAME		"fmr280"
#endif
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#if defined(_FMR60)
#define LINES_PER_FRAME 	784
#define CHARS_PER_LINE		98
#else
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		54
#endif
//#define CPU_CLOCKS		12000000
#define CPU_CLOCKS		8000000
#if defined(_FMR60)
#define SCREEN_WIDTH		1120
#define SCREEN_HEIGHT		750
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#endif
#define MAX_DRIVE		4
#define MAX_SCSI		8
#define MAX_MEMCARD		2
#if defined(HAS_I286)
#define I86_BIOS_CALL
#else
#define I386_BIOS_CALL
#endif
#define I8259_MAX_CHIPS		2
//#define SINGLE_MODE_DMA
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_CPU_TYPE		2
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
// i386 core will support debugger later
//#if defined(HAS_I286)
#define USE_DEBUGGER
//#endif

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HD46505;
#ifdef _FMR60
class HD63484;
#endif
class I8251;
class I8253;
class I8259;
#if defined(HAS_I286)
class I286;
#else
class I386;
#endif
class IO;
class MB8877;
class MSM58321;
class PCM1BIT;
class UPD71071;

class BIOS;
class CMOS;
class FLOPPY;
class KEYBOARD;
class MEMORY;
//class SERIAL;
class SCSI;
class TIMER;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
#if defined(_FMR60)
	HD63484* acrtc;
#endif
	I8251* sio;
	I8253* pit0;
	I8253* pit1;
	I8259* pic;
#if defined(HAS_I286)
	I286* cpu;
#else
	I386* cpu;
#endif
	IO* io;
	MB8877* fdc;
	MSM58321* rtc;
	PCM1BIT* pcm;
	UPD71071* dma;
	
	BIOS* bios;
	CMOS* cmos;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SCSI* scsi;
//	SERIAL* serial;
	TIMER* timer;
	
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
