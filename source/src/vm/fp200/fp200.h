/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ virtual machine ]
*/

#ifndef _FP200_H_
#define _FP200_H_

#define DEVICE_NAME		"CASIO FP-200"
#define CONFIG_NAME		"fp200"

// device informations for virtual machine
#define FRAMES_PER_SEC		64
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		3072000
#define SCREEN_WIDTH		160
#define SCREEN_HEIGHT		64
#define HAS_I8085
#define I8080_MEMORY_WAIT
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x2000
#define USE_SOUND_FILES		3
#define USE_SOUND_FILES_RELAY

// device informations for win32
#define WINDOW_MODE_BASE	2
#define USE_BOOT_MODE		2
#define USE_TAPE
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#if defined(USE_SOUND_FILES)
#define USE_SOUND_VOLUME	2
#else
#define USE_SOUND_VOLUME	1
#endif
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("CMT"),
#if defined(USE_SOUND_FILES)
	_T("CMT Relay"),
#endif
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8080;
class MEMORY;
class RP5C01;

class IO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8080* cpu;
	MEMORY* memory;
	RP5C01* rtc;
	
	IO* io;
	
	// memory
	uint8_t rom[0x8000];
	uint8_t ram[0x8000];
	
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
	
	// user interface
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	bool is_frame_skippable();
	
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
