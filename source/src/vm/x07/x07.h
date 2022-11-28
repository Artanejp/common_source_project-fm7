/*
	CANON X-07 Emulator 'eX-07'

	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ virtual machine ]
*/

#ifndef _X07_H_
#define _X07_H_

#define DEVICE_NAME		"CANON X-07"
#define CONFIG_NAME		"x07"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3840000
#define CPU_START_ADDR		0xc3c3
#define SCREEN_WIDTH		120
#define SCREEN_HEIGHT		32
#define TV_SCREEN_WIDTH		256
#define TV_SCREEN_HEIGHT	192
#define HAS_NSC800
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x800

// device informations for win32
#define TV_WINDOW_WIDTH		TV_SCREEN_WIDTH
#define TV_WINDOW_HEIGHT	TV_SCREEN_HEIGHT

#define WINDOW_MODE_BASE	3
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class MEMORY;
class Z80;

class IO;

#include "../../fileio.h"
#include "../vm_template.h"

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	MEMORY* memory;
	Z80* cpu;
	
	IO* io;
	
	// memory
//	uint8_t c3[0x2000];
	uint8_t ram[0x6000];
	uint8_t app[0x2000];
	uint8_t vram[0x1800];
	uint8_t tv[0x1000];
	uint8_t bas[0x5000];
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
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
