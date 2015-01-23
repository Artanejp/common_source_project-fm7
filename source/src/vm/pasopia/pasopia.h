/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2006.12.28 -

	[ virtual machine ]
*/

#ifndef _PASOPIA_H_
#define _PASOPIA_H_

#ifdef _LCD
#define DEVICE_NAME		"TOSHIBA PASOPIA with LCD"
#define CONFIG_NAME		"pasopialcd"
#else
#define DEVICE_NAME		"TOSHIBA PASOPIA"
#define CONFIG_NAME		"pasopia"
#endif

#define MODE_TBASIC_V1_0	0
#define MODE_TBASIC_V1_1	1
#define MODE_OABASIC		2
#define MODE_OABASIC_NO_DISK	3
#define MODE_MINI_PASCAL	4

#define DEVICE_RAM_PAC		0
#define DEVICE_KANJI_ROM	1
#define DEVICE_JOYSTICK		2

// device informations for virtual machine
#ifdef _LCD
#define FRAMES_PER_SEC		74.38
#define LINES_PER_FRAME 	32
#define CHARS_PER_LINE		94
#define HD46505_HORIZ_FREQ	2380
#else
#define FRAMES_PER_SEC		59.92
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		57
#define HD46505_HORIZ_FREQ	15700
#endif
#define CPU_CLOCKS		3993600
#ifdef _LCD
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		128
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#endif
#define MAX_DRIVE		4
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_BOOT_MODE		5
#define USE_DEVICE_TYPE		3
#define USE_TAPE
#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
#define USE_BINARY_FILE1
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class IO;
class LS393;
class NOT;
class PCM1BIT;
class UPD765A;
class Z80;
class Z80CTC;
class Z80PIO;

class FLOPPY;
class DISPLAY;
class KEYBOARD;
class MEMORY;
class PAC2;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	HD46505* crtc;
	I8255* pio0;
	I8255* pio1;
	I8255* pio2;
	IO* io;
	LS393* flipflop;
	NOT* not;
	PCM1BIT* pcm;
	UPD765A* fdc;
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIO* pio;
	
	FLOPPY* floppy;
	DISPLAY* display;
	KEYBOARD* key;
	MEMORY* memory;
	PAC2* pac2;
	
	int boot_mode;
	
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
	
	// user interface
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	void load_binary(int drv, _TCHAR* file_path);
	void save_binary(int drv, _TCHAR* file_path) {}
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
