/*
	SHARP SM-B-80TE Emulator 'eSM-B-80TE'

	Author : Takeda.Toshiya
	Date   : 2016.12.29-

	[ virtual machine ]
*/

#ifndef _SM_B_80TE_H_
#define _SM_B_80TE_H_

#define DEVICE_NAME		"SHARP SM-B-80TE"
#define CONFIG_NAME		"smb80te"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		2457600
#define SCREEN_WIDTH		768
#define SCREEN_HEIGHT		512

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		8
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x01
#define USE_TAPE		1
#define USE_BINARY_FILE		1
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

#define LED_WIDTH	24
#define LED_HEIGHT	24

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	{484 + 53 * 0, 458 - 53 * 0, 49, 49, 0x80},	// 0 PC
	{484 + 53 * 1, 458 - 53 * 0, 49, 49, 0x81},	// 1 SP
	{484 + 53 * 2, 458 - 53 * 0, 49, 49, 0x82},	// 2 IX
	{484 + 53 * 3, 458 - 53 * 0, 49, 49, 0x83},	// 3 IY
	{484 + 53 * 0, 458 - 53 * 1, 49, 49, 0x84},	// 4 BA
	{484 + 53 * 1, 458 - 53 * 1, 49, 49, 0x85},	// 5 BC
	{484 + 53 * 2, 458 - 53 * 1, 49, 49, 0x86},	// 6 I
	{484 + 53 * 3, 458 - 53 * 1, 49, 49, 0x87},	// 7 IF
	{484 + 53 * 0, 458 - 53 * 2, 49, 49, 0x88},	// 8 H
	{484 + 53 * 1, 458 - 53 * 2, 49, 49, 0x89},	// 9 L
	{484 + 53 * 2, 458 - 53 * 2, 49, 49, 0x8a},	// A
	{484 + 53 * 3, 458 - 53 * 2, 49, 49, 0x8b},	// B
	{484 + 53 * 0, 458 - 53 * 3, 49, 49, 0x8c},	// C
	{484 + 53 * 1, 458 - 53 * 3, 49, 49, 0x8d},	// D
	{484 + 53 * 2, 458 - 53 * 3, 49, 49, 0x8e},	// E
	{484 + 53 * 3, 458 - 53 * 3, 49, 49, 0x8f},	// F
	{484 + 53 * 0, 458 - 53 * 4, 49, 49, 0x98},	// RUN
	{484 + 53 * 1, 458 - 53 * 4, 49, 49, 0x99},	// STEP
	{484 + 53 * 2, 458 - 53 * 4, 49, 49, 0x9a},	// LD INC
	{484 + 53 * 3, 458 - 53 * 4, 49, 49, 0x9b},	// STOR REC
	{484 + 53 * 4, 458 - 53 * 4, 49, 49, 0x97},	// RES
	{484 + 53 * 4, 458 - 53 * 3, 49, 49, 0x9c},	// SHIFT
	{484 + 53 * 4, 458 - 53 * 2, 49, 49, 0x9d},	// REG' REG
	{484 + 53 * 4, 458 - 53 * 1, 49, 49, 0x9e},	// ADRS
	{484 + 53 * 4, 458 - 53 * 0, 49, 49, 0x9f},	// WR
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{598 + 32 * 3, 110, LED_WIDTH, LED_HEIGHT},
	{598 + 32 * 2, 110, LED_WIDTH, LED_HEIGHT},
	{598 + 32 * 1, 110, LED_WIDTH, LED_HEIGHT},
	{598 + 32 * 0, 110, LED_WIDTH, LED_HEIGHT},
	{446 + 32 * 3, 110, LED_WIDTH, LED_HEIGHT},
	{446 + 32 * 2, 110, LED_WIDTH, LED_HEIGHT},
	{446 + 32 * 1, 110, LED_WIDTH, LED_HEIGHT},
	{446 + 32 * 0, 110, LED_WIDTH, LED_HEIGHT},
};

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class IO;
class NOT;
class Z80;
class Z80PIO;

class MEMORY;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	IO* io;
	NOT* not_ear;
	Z80* cpu;
	Z80PIO* pio1;
	Z80PIO* pio2;
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
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
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
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
