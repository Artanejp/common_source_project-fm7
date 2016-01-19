/*
	HITACH BASIC Master Jr Emulator 'eBASICMasterJr'

	Author : Takeda.Toshiya
	Date   : 2015.08.28-

	[ virtual machine ]
*/

#ifndef _BMJR_H_
#define _BMJR_H_

#define DEVICE_NAME		"HITACHI BASIC Master Jr"
#define CONFIG_NAME		"bmjr"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		754560
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
//#define HAS_MB8861
#define HAS_MC6800

// device informations for win32
#define USE_TAPE
#define USE_TAPE_BUTTON
#define NOTIFY_KEY_DOWN
#define USE_CRT_MONITOR_4_3 1
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
//#define USE_CRT_FILTER
//#define USE_SCANLINE
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class MC6800;
class MC6820;

class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	MC6800* cpu;
	MC6820* pia;
	
	MEMORY* memory;
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool tape_playing();
	bool tape_recording();
	int tape_position();
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward() {}
	void push_apss_rewind() {}
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
