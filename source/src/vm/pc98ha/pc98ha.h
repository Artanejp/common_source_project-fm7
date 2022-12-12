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
#define I86_PSEUDO_BIOS
//#define UPD765A_DMA_MODE
//#define SINGLE_MODE_DMA
#define IOBUS_RETURN_ADDR
#ifdef _PC98HA
//#define DOCKING_STATION
#endif

// device informations for win32
#define USE_FLOPPY_DISK		1
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("Noise (FDD)"),
};
#endif

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
class NOT;
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
class MEMBUS;
class NOTE;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	DEVICE* printer;
	I8251* sio_rs;
	I8251* sio_kbd;
	I8253* pit;
	I8255* pio_sys;
	I8255* pio_prn;
	I8259* pic;
	I86* cpu;
	IO* io;
	NOT* not_busy;
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
	MEMBUS* memory;
	NOTE* note;
	
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
	double get_frame_rate()
	{
		return FRAMES_PER_SEC;
	}
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
	bool is_frame_skippable();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
