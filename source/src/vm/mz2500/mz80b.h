/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ virtual machine ]
*/

#ifndef _MZ80B_H_
#define _MZ80B_H_

#if defined(_MZ80B)
#define DEVICE_NAME		"SHARP MZ-80B"
#define CONFIG_NAME		"mz80b"
#elif defined(_MZ2000)
#define DEVICE_NAME		"SHARP MZ-2000"
#define CONFIG_NAME		"mz2000"
#else
#define DEVICE_NAME		"SHARP MZ-2200"
#define CONFIG_NAME		"mz2200"
#endif

#ifndef _MZ80B
#define SUPPORT_QUICK_DISK
#define SUPPORT_16BIT_BOARD
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define CPU_CLOCKS_HIGH		6000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define HAS_MB8876
#ifdef SUPPORT_QUICK_DISK
#endif
#define PRINTER_STROBE_RISING_EDGE

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_CPU_TYPE		2
#define USE_FLOPPY_DISK		4
#ifdef SUPPORT_QUICK_DISK
#define USE_QUICK_DISK		1
#endif
#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	4
#ifndef _MZ80B
#define USE_SCREEN_FILTER
#endif
#define USE_SCANLINE
#define USE_SOUND_VOLUME	4
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8253;
class I8255;
class IO;
class MB8877;
class PCM1BIT;
class Z80;
class Z80PIO;

class CMT;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class MZ1R12;
class MZ1R13;
class PRINTER;
class TIMER;

#ifdef SUPPORT_QUICK_DISK
class Z80SIO;
class QUICKDISK;
#endif

#ifdef SUPPORT_16BIT_BOARD
class I86;
class I8259;
class MZ1M01;
#endif

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8253* pit;
	I8255* pio_i;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	Z80* cpu;
	Z80PIO* pio;
	
	CMT* cmt;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MZ1R12* mz1r12;
	MZ1R13* mz1r13;
	PRINTER* printer;
	TIMER* timer;
	
#ifdef SUPPORT_QUICK_DISK
	Z80SIO* sio;
	QUICKDISK* qd;
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	Z80PIO* pio_to16;
	I86* cpu_16;
	I8259* pic_16;
	MZ1M01* mz1m01;
#endif
	
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
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#ifdef SUPPORT_QUICK_DISK
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
	uint32_t is_quick_disk_accessed();
#endif
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
	void push_play(int drv);
	void push_stop(int drv);
	void push_fast_forward(int drv);
	void push_fast_rewind(int drv);
	void push_apss_forward(int drv) {}
	void push_apss_rewind(int drv) {}
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
