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
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define HAS_MB8876
#define PCM1BIT_HIGH_QUALITY
#ifdef SUPPORT_QUICK_DISK
// 1byte=32clock/3.25MHz*8=79usec
#define Z80SIO_DELAY_SEND	100
#define Z80SIO_DELAY_RECV	100
#endif
#ifdef SUPPORT_16BIT_BOARD
#define HAS_I88
#define I8259_MAX_CHIPS		1
#endif

// memory wait
#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#ifdef SUPPORT_QUICK_DISK
#define USE_QD1
#endif
#define USE_TAPE
#define USE_TAPE_BUTTON
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#ifndef _MZ80B
#define USE_MONITOR_TYPE	2
#define USE_CRT_FILTER
#endif
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

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
class TIMER;

#ifdef SUPPORT_QUICK_DISK
class Z80SIO;
class QUICKDISK;
#endif

#ifdef SUPPORT_16BIT_BOARD
class I286;
class I8259;
class MZ1M01;
#endif

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
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
	TIMER* timer;
	
#ifdef SUPPORT_QUICK_DISK
	Z80SIO* sio;
	QUICKDISK* qd;
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	Z80PIO* pio_to16;
	I286* cpu_16;
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
	
	// user interface
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
#ifdef SUPPORT_QUICK_DISK
	void open_quickdisk(int drv, _TCHAR* file_path);
	void close_quickdisk(int drv);
	bool quickdisk_inserted(int drv);
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
