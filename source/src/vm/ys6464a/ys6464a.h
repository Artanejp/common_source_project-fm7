/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ virtual machine ]
*/

#ifndef _BABBAGE_2ND_H_
#define _BABBAGE_2ND_H_

#define DEVICE_NAME		"SHINKO SANGYO YS-6464A"
#define CONFIG_NAME		"ys6464a"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		357

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		21
#define MAX_DRAW_RANGES		6
#define USE_BINARY_FILE		1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	{342 + 59 * 0, 287 - 59 * 0, 46, 46, 0x30},	// 0
	{342 + 59 * 1, 287 - 59 * 0, 46, 46, 0x31},	// 1
	{342 + 59 * 2, 287 - 59 * 0, 46, 46, 0x32},	// 2
	{342 + 59 * 3, 287 - 59 * 0, 46, 46, 0x33},	// 3
	{342 + 59 * 0, 287 - 59 * 1, 46, 46, 0x34},	// 4
	{342 + 59 * 1, 287 - 59 * 1, 46, 46, 0x35},	// 5
	{342 + 59 * 2, 287 - 59 * 1, 46, 46, 0x36},	// 6
	{342 + 59 * 3, 287 - 59 * 1, 46, 46, 0x37},	// 7
	{342 + 59 * 0, 287 - 59 * 2, 46, 46, 0x38},	// 8
	{342 + 59 * 1, 287 - 59 * 2, 46, 46, 0x39},	// 9
	{342 + 59 * 2, 287 - 59 * 2, 46, 46, 0x41},	// A
	{342 + 59 * 3, 287 - 59 * 2, 46, 46, 0x42},	// B
	{342 + 59 * 0, 287 - 59 * 3, 46, 46, 0x43},	// C
	{342 + 59 * 1, 287 - 59 * 3, 46, 46, 0x44},	// D
	{342 + 59 * 2, 287 - 59 * 3, 46, 46, 0x45},	// E
	{342 + 59 * 3, 287 - 59 * 3, 46, 46, 0x46},	// F
	{342 + 59 * 4, 287 - 59 * 0, 46, 46, 0x70},	// WRITE INC
	{342 + 59 * 4, 287 - 59 * 1, 46, 46, 0x71},	// READ DEC
	{342 + 59 * 4, 287 - 59 * 2, 46, 46, 0x72},	// READ INC
	{342 + 59 * 4, 287 - 59 * 3, 46, 46, 0x73},	// ADR RUN
	{262         , 287         , 46, 46, 0x00},	// RESET
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{357, 23, 28, 40},
	{392, 23, 28, 40},
	{439, 23, 28, 40},
	{474, 23, 28, 40},
	{547, 23, 28, 40},
	{582, 23, 28, 40},
};

class EMU;
class DEVICE;
class EVENT;

class IO;
class I8255;
class MEMORY;
//class PCM1BIT;
class Z80;

class DISPLAY;
class KEYBOARD;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	IO* io;
	I8255* pio;
	MEMORY* memory;
//	PCM1BIT* pcm;
	Z80* cpu;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	
	// memory
	uint8_t rom[0x2000];
	uint8_t ram[0x2000];
	
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
	
	// user interface
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
