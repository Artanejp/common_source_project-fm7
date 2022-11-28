/*
	National JR-100 Emulator 'eJR-100'

	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ virtual machine ]
*/

#ifndef _JR100_H_
#define _JR100_H_

#define DEVICE_NAME		"National JR-100"
#define CONFIG_NAME		"jr100"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		894886
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
#define HAS_MB8861

// device informations for win32
#define USE_TAPE		1
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_VM_AUTO_KEY_TABLE
//#define USE_SCREEN_FILTER
//#define USE_SCANLINE
#define USE_SOUND_VOLUME	3
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE

static const int vm_auto_key_table_base[][2] = {
	// 0x100: shift
	{0x2f,	0x100 | 0x4c},	// '/' => 'L' + SHIFT
	{0x3f,	0x100 | 0x4b},	// '?' => 'K' + SHIFT
	{0x40,	0x100 | 0x55},	// '@' => 'U' + SHIFT
	{0x41,	0x000 | 0x41},	// 'A'
	{0x42,	0x000 | 0x42},	// 'B'
	{0x43,	0x000 | 0x43},	// 'C'
	{0x44,	0x000 | 0x44},	// 'D'
	{0x45,	0x000 | 0x45},	// 'E'
	{0x46,	0x000 | 0x46},	// 'F'
	{0x47,	0x000 | 0x47},	// 'G'
	{0x48,	0x000 | 0x48},	// 'H'
	{0x49,	0x000 | 0x49},	// 'I'
	{0x4a,	0x000 | 0x4a},	// 'J'
	{0x4b,	0x000 | 0x4b},	// 'K'
	{0x4c,	0x000 | 0x4c},	// 'L'
	{0x4d,	0x000 | 0x4d},	// 'M'
	{0x4e,	0x000 | 0x4e},	// 'N'
	{0x4f,	0x000 | 0x4f},	// 'O'
	{0x50,	0x000 | 0x50},	// 'P'
	{0x51,	0x000 | 0x51},	// 'Q'
	{0x52,	0x000 | 0x52},	// 'R'
	{0x53,	0x000 | 0x53},	// 'S'
	{0x54,	0x000 | 0x54},	// 'T'
	{0x55,	0x000 | 0x55},	// 'U'
	{0x56,	0x000 | 0x56},	// 'V'
	{0x57,	0x000 | 0x57},	// 'W'
	{0x58,	0x000 | 0x58},	// 'X'
	{0x59,	0x000 | 0x59},	// 'Y'
	{0x5a,	0x000 | 0x5a},	// 'Z'
	{0x5b,	0x100 | 0x4f},	// '[' => 'O' + SHIFT
	{0x5c,	0x100 | 0x49},	// '\' => 'I' + SHIFT
	{0x5d,	0x100 | 0x50},	// ']' => 'P' + SHIFT
	{0x5e,	0x100 | 0x30},	// '^' => '0' + SHIFT
	{0x5f,	0x100 | 0x4d},	// '_' => 'M' + SHIFT
	{0x61,	0x000 | 0x41},	// 'a'
	{0x62,	0x000 | 0x42},	// 'b'
	{0x63,	0x000 | 0x43},	// 'c'
	{0x64,	0x000 | 0x44},	// 'd'
	{0x65,	0x000 | 0x45},	// 'e'
	{0x66,	0x000 | 0x46},	// 'f'
	{0x67,	0x000 | 0x47},	// 'g'
	{0x68,	0x000 | 0x48},	// 'h'
	{0x69,	0x000 | 0x49},	// 'i'
	{0x6a,	0x000 | 0x4a},	// 'j'
	{0x6b,	0x000 | 0x4b},	// 'k'
	{0x6c,	0x000 | 0x4c},	// 'l'
	{0x6d,	0x000 | 0x4d},	// 'm'
	{0x6e,	0x000 | 0x4e},	// 'n'
	{0x6f,	0x000 | 0x4f},	// 'o'
	{0x70,	0x000 | 0x50},	// 'p'
	{0x71,	0x000 | 0x51},	// 'q'
	{0x72,	0x000 | 0x52},	// 'r'
	{0x73,	0x000 | 0x53},	// 's'
	{0x74,	0x000 | 0x54},	// 't'
	{0x75,	0x000 | 0x55},	// 'u'
	{0x76,	0x000 | 0x56},	// 'v'
	{0x77,	0x000 | 0x57},	// 'w'
	{0x78,	0x000 | 0x58},	// 'x'
	{0x79,	0x000 | 0x59},	// 'y'
	{0x7a,	0x000 | 0x5a},	// 'z'
	{-1, -1},
};

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
class MC6800;
class NOT;
class PCM1BIT;
class SY6522;

class MEMORY;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	MC6800* cpu;
	NOT* not_mic;
	NOT* not_ear;
	PCM1BIT* pcm;
	SY6522* via;
	
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
