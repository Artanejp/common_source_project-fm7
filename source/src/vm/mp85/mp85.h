/*
	MITEC MP-85 Emulator 'eMP-85'

	Author : Takeda.Toshiya
	Date   : 2021.01.19-

	[ virtual machine ]
*/

#ifndef _MP85_H_
#define _MP85_H_

#define DEVICE_NAME		"MITEC MP-85"
#define CONFIG_NAME		"mp85"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		2000000
#define SCREEN_WIDTH		784
#define SCREEN_HEIGHT		523
#define HAS_I8085
#define I8255_AUTO_HAND_SHAKE

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		6
#define USE_TAPE		1
#define USE_BINARY_FILE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
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

#define BUTTON_SPACE_X	47
#define BUTTON_SPACE_Y	53
#define BUTTON_SIZE_X	47
#define BUTTON_SIZE_Y	52
#define BUTTON_POS_X1	536
#define BUTTON_POS_X2	730
#define BUTTON_POS_Y1	470
#define BUTTON_POS_Y2	254

#define LED_SPACE_X	38
#define LED_SIZE_X	33
#define LED_SIZE_Y	50
#define LED_POS_X1	530
#define LED_POS_X2	696
#define LED_POS_Y	44

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x30},	// 0
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x31},	// 1
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x32},	// 2
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x33},	// 3
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x34},	// 4
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x35},	// 5
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x36},	// 6
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x37},	// 7
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x38},	// 8
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x39},	// 9
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x41},	// A
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x42},	// B
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x43},	// C
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x44},	// D
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x45},	// E
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x46},	// F
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x70},	// RUN
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x71},	// STEP
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x72},	// MONI IN
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x00},	// RESET
	{BUTTON_POS_X2                     , BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x73},	// ADRS SET
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x74},	// INCR READ
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x75},	// DECR READ
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x76},	// FUNC
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x77},	// INCR WRITE
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

class DATAREC;
class I8080;
class I8255;
class I8279;
class IO;

class DISPLAY;
class KEYBOARD;
class MEMBUS;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8080* cpu;
	I8255* pio;
	I8279* kdc;
	IO* io;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMBUS* memory;
	
	// memory
	uint8_t mon[0x800];
	uint8_t ext[0x800];
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
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
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
