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
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
//#define USE_CRT_FILTER
//#define USE_SCANLINE
#define USE_SOUND_FILES 2
//#define USE_SOUND_FILES_FDD
#define USE_SOUND_FILES_RELAY
#if defined(USE_SOUND_FILES)
#define USE_SOUND_VOLUME	3
#else
#define USE_SOUND_VOLUME	2
#endif
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT"),
#if defined(USE_SOUND_FILES)
	_T("CMT RELAY"),
#endif
};
#endif

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
	double get_frame_rate();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
//	uint32_t get_access_lamp_status();
	
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
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward() {}
	void push_apss_rewind() {}
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
