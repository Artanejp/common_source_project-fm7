/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ virtual machine ]
*/

#ifndef _MZ3500_H_
#define _MZ3500_H_

#define DEVICE_NAME		"SHARP MZ-3500"
#define CONFIG_NAME		"mz3500"

// device informations for virtual machine
#define FRAMES_PER_SEC		47.3
#define LINES_PER_FRAME 	441
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define UPD765A_WAIT_RESULT7
#define UPD765A_EXT_DRVSEL
#define UPD7220_HORIZ_FREQ	20920
#define PRINTER_STROBE_RISING_EDGE

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x1fd
#define USE_FLOPPY_DISK		4
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	4
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
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

class I8251;
class I8253;
class I8255;
class IO;
class LS244;
class NOT;
class PCM1BIT;
class UPD1990A;
class UPD7220;
class UPD765A;
class Z80;

class MAIN;
class SUB;
class KEYBOARD;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	// for main cpu
	IO* mainio;
	UPD765A* fdc;
	Z80* maincpu;
	MAIN* mainbus;
	
	// for sub cpu
	DEVICE* printer;
	I8251* sio;
	I8253* pit;
	I8255* pio;
	IO* subio;
	LS244* ls244;
	NOT* not_data0;
	NOT* not_data1;
	NOT* not_data2;
	NOT* not_data3;
	NOT* not_data4;
	NOT* not_data5;
	NOT* not_data6;
	NOT* not_data7;
	NOT* not_busy;
	PCM1BIT* pcm;
	UPD1990A* rtc;
	UPD7220* gdc_chr;
	UPD7220* gdc_gfx;
	Z80* subcpu;
	SUB* subbus;
	KEYBOARD* kbd;
	
	uint8_t halt;
	
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
