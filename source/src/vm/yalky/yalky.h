/*
	Yuasa Kyouiku System YALKY Emulator 'eYALKY'

	Author : Takeda.Toshiya
	Date   : 2016.03.28-

	[ virtual machine ]
*/

#ifndef _YALKY_H_
#define _YALKY_H_

#define DEVICE_NAME		"Yuasa Kyouiku System YALKY"
#define CONFIG_NAME		"yalky"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3000000
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		256
#define WINDOW_HEIGHT_ASPECT	192
#define HAS_I8085
#define DATAREC_SOUND
#define DATAREC_SOUND_LEFT
#define DATAREC_FAST_FWD_SPEED	10
#define DATAREC_FAST_REW_SPEED	10
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x100

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("CMT (Voice)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8080;
class I8155;
class MEMORY;

class IO;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8080* cpu;
	I8155* pio;
	MEMORY* memory;
	
	IO* io;
	
	uint8_t rom[0x2000];
	uint8_t ram[0x100];
	uint8_t vram[0x400];
	
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
