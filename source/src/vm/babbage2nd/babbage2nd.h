/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ virtual machine ]
*/

#ifndef _BABBAGE_2ND_H_
#define _BABBAGE_2ND_H_

#define DEVICE_NAME		"GIJUTSU HYORON SHA Babbage-2nd"
#define CONFIG_NAME		"babbage2nd"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		2500000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		483
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x800

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		21
#define MAX_DRAW_RANGES		14
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
	{353, 419, 49, 49, 0x30},	// 0
	{407, 419, 49, 49, 0x31},	// 1
	{461, 419, 49, 49, 0x32},	// 2
	{515, 419, 49, 49, 0x33},	// 3
	{353, 353, 49, 49, 0x34},	// 4
	{407, 353, 49, 49, 0x35},	// 5
	{461, 353, 49, 49, 0x36},	// 6
	{515, 353, 49, 49, 0x37},	// 7
	{353, 287, 49, 49, 0x38},	// 8
	{407, 287, 49, 49, 0x39},	// 9
	{461, 287, 49, 49, 0x41},	// A
	{515, 287, 49, 49, 0x42},	// B
	{353, 221, 49, 49, 0x43},	// C
	{407, 221, 49, 49, 0x44},	// D
	{461, 221, 49, 49, 0x45},	// E
	{515, 221, 49, 49, 0x46},	// F
	{575, 419, 49, 49, 0x70},	// INC
	{575, 353, 49, 49, 0x71},	// DA
	{575, 287, 49, 49, 0x72},	// AD
	{575, 221, 49, 49, 0x73},	// GO
	{ 36,  18, 49, 49, 0x00},	// RES
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{587,  37, 34, 58},
	{530,  37, 34, 58},
	{455,  37, 34, 58},
	{398,  37, 34, 58},
	{341,  37, 34, 58},
	{284,  37, 34, 58},
	{600, 133, 17, 17},
	{567, 133, 17, 17},
	{534, 133, 17, 17},
	{501, 133, 17, 17},
	{468, 133, 17, 17},
	{435, 133, 17, 17},
	{402, 133, 17, 17},
	{369, 133, 17, 17},
};

class EMU;
class DEVICE;
class EVENT;

class IO;
class MEMORY;
class Z80;
class Z80CTC;
class Z80PIO;

class DISPLAY;
class KEYBOARD;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	IO* io;
	MEMORY* memory;
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIO* pio1;
	Z80PIO* pio2;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	
	// memory
	uint8_t rom[0x800];
	uint8_t ram[0x800];
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
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
