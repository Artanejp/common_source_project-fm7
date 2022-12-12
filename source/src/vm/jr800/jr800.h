/*
	National JR-800 Emulator 'eJR-800'

	Author : Takeda.Toshiya
	Date   : 2017.03.13-

	[ virtual machine ]
*/

#ifndef _JR800_H_
#define _JR800_H_

#define DEVICE_NAME		"National JR-800"
#define CONFIG_NAME		"jr800"

// device informations for virtual machine
#define FRAMES_PER_SEC		72
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		491520
#define SCREEN_WIDTH		192
#define SCREEN_HEIGHT		64
#define HAS_HD6301

// device informations for win32
#define WINDOW_MODE_BASE	3
#define USE_TAPE		1
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
//#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	3
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD44102;
class MC6800;
class MEMORY;
class PCM1BIT;

class IO;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	HD44102* lcd[8];
	MC6800* cpu;
	MEMORY* memory;
	PCM1BIT* pcm;
	
	IO* io;
	
	uint8_t ram[0x6000];
	uint8_t rom[0x8000];
	
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
