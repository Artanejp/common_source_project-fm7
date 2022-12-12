/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ virtual machine ]
*/

#ifndef _MZ5500_H_
#define _MZ5500_H_

#if defined(_MZ5500)
#define DEVICE_NAME		"SHARP MZ-5500"
#define CONFIG_NAME		"mz5500"
#elif defined(_MZ6500)
#define DEVICE_NAME		"SHARP MZ-6500"
#define CONFIG_NAME		"mz6500"
#elif defined(_MZ6550)
#define DEVICE_NAME		"SHARP MZ-6550"
#define CONFIG_NAME		"mz6550"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.49
#define LINES_PER_FRAME 	448
#if defined(_MZ5500)
#define CPU_CLOCKS		4915200
#elif defined(_MZ6500) || defined(_MZ6550)
#define CPU_CLOCKS		8000000
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define UPD7220_HORIZ_FREQ	24860
#define Z80CTC_CLOCKS		2457600
#define SINGLE_MODE_DMA
#define HAS_AY_3_8912
#define PRINTER_STROBE_RISING_EDGE

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FLOPPY_DISK		4
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8255;
class I8259;
#if defined(_MZ6550)
class I286;
#else
class I86;
#endif
class IO;
class LS393;
class NOT;
class RP5C01;
class UPD7220;
class UPD765A;
//class YM2203;
class AY_3_891X;
class Z80CTC;
class Z80SIO;

class DISPLAY;
class KEYBOARD;
class MEMBUS;
class SYSPORT;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DEVICE* printer;
	I8237* dma;
	I8255* pio;
	I8259* pic;	// includes 2chips
#if defined(_MZ6550)
	I286* cpu;
#else
	I86* cpu;
#endif
	IO* io;
	LS393* div;
	NOT* not_data0;
	NOT* not_data1;
	NOT* not_data2;
	NOT* not_data3;
	NOT* not_data4;
	NOT* not_data5;
	NOT* not_data6;
	NOT* not_data7;
	NOT* not_busy;
	RP5C01* rtc;
	UPD7220* gdc;
	UPD765A* fdc;
//	YM2203* psg;
	AY_3_891X* psg;
	Z80CTC* ctc0;
#if defined(_MZ6500) || defined(_MZ6550)
	Z80CTC* ctc1;
#endif
	Z80SIO* sio;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMBUS* memory;
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
	void special_reset();
	void run();
	double get_frame_rate();
	
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
