/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ virtual machine ]
*/

#ifndef _M23_H_
#define _M23_H_

#if defined(_M68)
#define DEVICE_NAME		"SORD M68"
#define CONFIG_NAME		"m68"
#else
#define DEVICE_NAME		"SORD M23"
#define CONFIG_NAME		"m23"
#endif

// device informations for virtual machine
#if defined(_M68)
#define FRAMES_PER_SEC		56.42
#define LINES_PER_FRAME 	432
#define CHARS_PER_LINE		108
#define HD46505_HORIZ_FREQ	24366
#else
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		114
#define HD46505_HORIZ_FREQ	15750
#endif
#define CPU_CLOCKS		3993600
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define MB8877_NO_BUSY_AFTER_SEEK
#define SINGLE_MODE_DMA

// device informations for win32
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0xff
#define USE_DRIVE_TYPE		3
#define DRIVE_TYPE_DEFAULT	1
#define USE_FLOPPY_DISK		4
#define USE_KEYBOARD_TYPE	2
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	2
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
//#define USE_PRINTER
//#define USE_PRINTER_TYPE	4
#define USE_LED_DEVICE		4
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

#ifdef USE_LED_DEVICE
static const _TCHAR *led_device_caption[] = {
	_T("S1:"),
	_T("S2:"),
	_T("SMALL:"),
	_T("KANA:"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

//class AM9511;
class HD46505;
class IO;
class MB8877;
class NOT;
class Z80;
class Z80CTC;
class Z80DMA;
class Z80PIO;
class Z80SIO;

class APU;
class BEEP;
class DISPLAY;
class FLOPPY;
class KEYBOARD;
class MEMBUS;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices for x1
	EVENT* event;
	
//	AM9511* apu;
	HD46505* crtc;
	IO* io;
	MB8877* fdc;
	NOT* not_drq;
	Z80* cpu;
	Z80CTC* ctc;
#ifdef _M68
	Z80CTC* ctc2;
#endif
	Z80DMA* dma;
	Z80PIO* pio;
	Z80SIO* sio;
	
	APU* apu;
	BEEP* beep;
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMBUS* memory;
	
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
	uint32_t get_led_status();
	
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
