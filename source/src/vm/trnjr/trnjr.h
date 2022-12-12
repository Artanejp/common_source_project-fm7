/*
	EPS TRN Junior Emulator 'eTRNJunior'

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ virtual machine ]
*/

#ifndef _TRNJR_H_
#define _TRNJR_H_

#define DEVICE_NAME		"ESP TRN Junior"
#define CONFIG_NAME		"trnjr"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		768
#define SCREEN_HEIGHT		512
#define HAS_TMPZ84C013

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		8
#define USE_BINARY_FILE		1
#define USE_MIDI
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Speaker"),
};
#endif

#define BUTTON_SPACE_X	50
#define BUTTON_SPACE_Y	52
#define BUTTON_SIZE_X	37
#define BUTTON_SIZE_Y	38
#define BUTTON_POS_X	378
#define BUTTON_POS_Y	165

#define LED_SPACE_X	42
#define LED_SIZE_X	38
#define LED_SIZE_Y	52
#define LED_POS_X1	269
#define LED_POS_X2	449
#define LED_POS_Y	81

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x30},	// 0
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x31},	// 1
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x32},	// 2
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x33},	// 3
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x34},	// 4
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x35},	// 5
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x36},	// 6
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x37},	// 7
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x38},	// 8
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x39},	// 9
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x41},	// A
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x42},	// B
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x43},	// C
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x44},	// D
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x45},	// E
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x46},	// F
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x70},	// REG
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x71},	// INC
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x72},	// DEC
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x73},	// RUN
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x74},	// STEP
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x75},	// WRT
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x76},	// ADRS
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x77},	// SHFT
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x00},	// RES
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{LED_POS_X1 + LED_SPACE_X * 0, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y}, // 7-seg LEDs
	{LED_POS_X1 + LED_SPACE_X * 1, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X1 + LED_SPACE_X * 2, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X1 + LED_SPACE_X * 3, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X2 + LED_SPACE_X * 0, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X2 + LED_SPACE_X * 1, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X2 + LED_SPACE_X * 2, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
	{LED_POS_X2 + LED_SPACE_X * 3, LED_POS_Y, LED_SIZE_X, LED_SIZE_Y},
};

class EMU;
class DEVICE;
class EVENT;

class I8255;
class IO;
class MIDI;
class PCM8BIT;
class TMPZ84C015;
class Z80;

class DISPLAY;
class MEMBUS;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8255* pio1;
	I8255* pio2;
	IO* io;
	MIDI* midi;
	PCM8BIT* speaker;
	TMPZ84C015* cpudev;
	Z80* cpu;
	
	DISPLAY* display;
	MEMBUS* memory;
	
	// memory
	uint8_t rom[0x8000];
	uint8_t ram[0x8000];
	
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
