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
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		8
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x01
#define USE_TAPE
#define USE_BINARY_FILE1
#define NOTIFY_KEY_DOWN
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#define LED_WIDTH	24
#define LED_HEIGHT	24

const struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int font_size;
	int code;
} vm_buttons[] = {
	{_T("PC\n0"),		485 + 53 * 0, 458 - 53 * 0, 49, 49, 20, 0x80},
	{_T("SP\n1"),		485 + 53 * 1, 458 - 53 * 0, 49, 49, 20, 0x81},
	{_T("IX\n2"),		485 + 53 * 2, 458 - 53 * 0, 49, 49, 20, 0x82},
	{_T("IY\n3"),		485 + 53 * 3, 458 - 53 * 0, 49, 49, 20, 0x83},
	{_T("BA\n4"),		485 + 53 * 0, 458 - 53 * 1, 49, 49, 20, 0x84},
	{_T("BC\n5"),		485 + 53 * 1, 458 - 53 * 1, 49, 49, 20, 0x85},
	{_T("I\n6"),		485 + 53 * 2, 458 - 53 * 1, 49, 49, 20, 0x86},
	{_T("IF\n7"),		485 + 53 * 3, 458 - 53 * 1, 49, 49, 20, 0x87},
	{_T("H\n8"),		485 + 53 * 0, 458 - 53 * 2, 49, 49, 20, 0x88},
	{_T("L\n9"),		485 + 53 * 1, 458 - 53 * 2, 49, 49, 20, 0x89},
	{_T("\nA"),		485 + 53 * 2, 458 - 53 * 2, 49, 49, 20, 0x8a},
	{_T("\nB"),		485 + 53 * 3, 458 - 53 * 2, 49, 49, 20, 0x8b},
	{_T("\nC"),		485 + 53 * 0, 458 - 53 * 3, 49, 49, 20, 0x8c},
	{_T("\nD"),		485 + 53 * 1, 458 - 53 * 3, 49, 49, 20, 0x8d},
	{_T("\nE"),		485 + 53 * 2, 458 - 53 * 3, 49, 49, 20, 0x8e},
	{_T("\nF"),		485 + 53 * 3, 458 - 53 * 3, 49, 49, 20, 0x8f},
	{_T("RUN"),		485 + 53 * 0, 458 - 53 * 4, 49, 49, 15, 0x98},
	{_T("STEP"),		485 + 53 * 1, 458 - 53 * 4, 49, 49, 15, 0x99},
	{_T("LD\nINC"),		485 + 53 * 2, 458 - 53 * 4, 49, 49, 15, 0x9a},
	{_T("STOR\nDEC"),	485 + 53 * 3, 458 - 53 * 4, 49, 49, 15, 0x9b},
	{_T("RES"),		485 + 53 * 4, 458 - 53 * 4, 49, 49, 15, 0x97},
	{_T("SHIFT"),		485 + 53 * 4, 458 - 53 * 3, 49, 49, 15, 0x9c},
	{_T("REG'\nREG"),	485 + 53 * 4, 458 - 53 * 2, 49, 49, 15, 0x9d},
	{_T("ADRS"),		485 + 53 * 4, 458 - 53 * 1, 49, 49, 15, 0x9e},
	{_T("WR"),		485 + 53 * 4, 458 - 53 * 0, 49, 49, 15, 0x9f},
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

class VM
{
protected:
	EMU* emu;
	
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
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
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
