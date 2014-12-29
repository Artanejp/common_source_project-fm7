/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.09 -

	[ virtual machine ]
*/

#ifndef _PC98HA_H_
#define _PC98HA_H_

#ifdef _PC98HA
#define DEVICE_NAME		"NEC PC-98HA"
#define CONFIG_NAME		"pc98ha"
#else
#define DEVICE_NAME		"NEC PC-98LT"
#define CONFIG_NAME		"pc98lt"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		56.4
#define LINES_PER_FRAME 	440
#ifdef _PC98HA
#define CPU_CLOCKS		10000000
#else
#define CPU_CLOCKS		8000000
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		1
#define HAS_V30
#define I86_BIOS_CALL
#define I8259_MAX_CHIPS		1
//#define UPD765A_DMA_MODE
//#define SINGLE_MODE_DMA
#define IO_ADDR_MAX		0x10000
#define IOBUS_RETURN_ADDR
#ifdef _PC98HA
//#define DOCKING_STATION
#endif

// device informations for win32
#define USE_FD1
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_ACCESS_LAMP
#define USE_DEBUGGER

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class I8251;
class I8253;
class I8255;
class I8259;
class I86;
class IO;
#ifdef _PC98HA
class UPD4991A;
#else
class UPD1990A;
#endif
class UPD71071;
class UPD765A;

class BIOS;
class CALENDAR;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class NOTE;
class PRINTER;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	I8251* sio_rs;
	I8251* sio_kbd;
	I8253* pit;
	I8255* pio_sys;
	I8255* pio_prn;
	I8259* pic;
	I86* cpu;
	IO* io;
#ifdef _PC98HA
	UPD4991A* rtc;
#else
	UPD1990A* rtc;
#endif
	UPD71071* dma;
	UPD765A* fdc;
	
	BIOS* bios;
	CALENDAR* calendar;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	NOTE* note;
	PRINTER* printer;
	
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
