/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#ifndef _MZ80K_H_
#define _MZ80K_H_

#if defined(_MZ1200)
#define DEVICE_NAME		"SHARP MZ-1200"
#define CONFIG_NAME		"mz1200"
#elif defined(_MZ80A)
#define DEVICE_NAME		"SHARP MZ-80A"
#define CONFIG_NAME		"mz80a"
#else
#define DEVICE_NAME		"SHARP MZ-80K"
#define CONFIG_NAME		"mz80k"
#endif

#ifdef _MZ80A
#define SUPPORT_MZ80AIF
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		2000000
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define PCM1BIT_HIGH_QUALITY
//#define LOW_PASS_FILTER
#ifdef SUPPORT_MZ80AIF
#define HAS_MB8876
#define MAX_DRIVE		4
#endif

// device informations for win32
#define USE_TAPE
#define USE_TAPE_BUTTON
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_DEBUGGER
#define USE_STATE
#ifdef SUPPORT_MZ80AIF
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_ACCESS_LAMP
#endif

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

#if defined(_MZ1200) || defined(_MZ80A)
class AND;
#endif
class DATAREC;
class I8253;
class I8255;
class LS393;
class PCM1BIT;
class Z80;

class DISPLAY;
class KEYBOARD;
class MEMORY;

#ifdef SUPPORT_MZ80AIF
class MB8877;
class FLOPPY;
class IO;
#endif

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
#if defined(_MZ1200) || defined(_MZ80A)
	AND* and;
#endif
	DATAREC* drec;
	I8253* ctc;
	I8255* pio;
	LS393* counter;
	PCM1BIT* pcm;
	Z80* cpu;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMORY* memory;
	
#ifdef SUPPORT_MZ80AIF
	MB8877* fdc;
	FLOPPY* floppy;
	IO* io;
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
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
#ifdef SUPPORT_MZ80AIF
	int access_lamp();
#endif
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// user interface
#ifdef SUPPORT_MZ80AIF
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
#endif
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	void push_play();
	void push_stop();
	bool now_skip();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
