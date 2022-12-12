/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ virtual machine ]
*/

#ifndef _TK80BS_H_
#define _TK80BS_H_

#if defined(_TK80BS)
#define DEVICE_NAME		"NEC TK-80BS"
#define CONFIG_NAME		"tk80bs"
#elif defined(_TK80)
#define DEVICE_NAME		"NEC TK-80"
#define CONFIG_NAME		"tk80"
#elif defined(_TK85)
#define DEVICE_NAME		"NEC TK-85"
#define CONFIG_NAME		"tk85"
#endif

// device informations for virtual machine
#if defined(_TK80BS)
#define FRAMES_PER_SEC		59.9
#define LINES_PER_FRAME 	262
#elif defined(_TK80) || defined(_TK85)
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#endif
#if defined(_TK80BS) || defined(_TK80)
#define CPU_CLOCKS		2048000
#define SCREEN_WIDTH		784
#define SCREEN_HEIGHT		428
#define HAS_I8080
#elif defined(_TK85)
#define CPU_CLOCKS		2457600
#define SCREEN_WIDTH		784
#define SCREEN_HEIGHT		534
#define HAS_I8085
#endif
#define I8255_AUTO_HAND_SHAKE

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#if defined(_TK80BS) || defined(_TK80)
#define MAX_BUTTONS		25
#elif defined(_TK85)
#define MAX_BUTTONS		26
#endif
#if defined(_TK80) || defined(_TK85)
#define MAX_DRAW_RANGES		8
#endif
#if defined(_TK80BS)
#define USE_BOOT_MODE		2
#endif
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0
#if defined(_TK80BS)
#define USE_TAPE		2
#else
#define USE_TAPE		1
#endif
#define USE_BINARY_FILE		1
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_AUTO_KEY_NUMPAD
#define USE_SOUND_VOLUME	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep #1"), _T("Beep #2"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

#if defined(_TK80BS) || defined(_TK80)
#define BUTTON_SPACE_X	46
#define BUTTON_SPACE_Y	46
#define BUTTON_SIZE_X	40
#define BUTTON_SIZE_Y	40
#define BUTTON_POS_X1	523
#define BUTTON_POS_X2	(BUTTON_POS_X1 + BUTTON_SPACE_X * 4)
#define BUTTON_POS_Y1	374
#define BUTTON_POS_Y2	190

#define LED_SPACE_X	36
#define LED_SIZE_X	33
#define LED_SIZE_Y	46
#define LED_POS_X1	461
#define LED_POS_X2	618
#define LED_POS_Y	28
#else
#define BUTTON_SPACE_X	47
#define BUTTON_SPACE_Y	47
#define BUTTON_SIZE_X	44
#define BUTTON_SIZE_Y	44
#define BUTTON_POS_X1	536
#define BUTTON_POS_X2	729
#define BUTTON_POS_Y1	488
#define BUTTON_POS_Y2	296

#define LED_SPACE_X	39
#define LED_SIZE_X	29
#define LED_SIZE_Y	39
#define LED_POS_X1	446
#define LED_POS_X2	607
#define LED_POS_Y	130
#endif

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x80},	// 0
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x81},	// 1
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x82},	// 2
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x83},	// 3
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x84},	// 4
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x85},	// 5
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x86},	// 6
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x87},	// 7
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x88},	// 8
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x89},	// 9
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8a},	// A
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8b},	// B
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8c},	// C
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8d},	// D
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8e},	// E
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x8f},	// F
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 0, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x98},	// RET
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 1, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x99},	// RUN
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 2, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9a},	// STORE DATA
	{BUTTON_POS_X1 + BUTTON_SPACE_X * 3, BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9b},	// LOAD DATA
#if defined(_TK80BS) || defined(_TK80)
	{BUTTON_POS_X2                     , BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x00},	// RESET
#elif defined(_TK85)
	{BUTTON_POS_X2                     , BUTTON_POS_Y2                     , BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x97},	// MON
#endif
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9c},	// ADRS SET
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9d},	// READ INCR
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9e},	// READ DECR
	{BUTTON_POS_X2                     , BUTTON_POS_Y1 - BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x9f},	// WRITE INCR
#if defined(_TK85)
	{727                               , 237                               , 29           , 29           , 0x00},	// RESET
#endif
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
#if defined(_TK80BS)
	{4, 158, 512, 256}, // CRT
#endif
};

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8080;
#if defined(_TK80BS)
class I8251;
class IO;
#endif
class I8255;
//class MEMORY;
class PCM1BIT;

#if defined(_TK80BS)
class CMT;
#endif
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
#if defined(_TK80BS)
	I8251* sio_b;
	I8255* pio_b;
	IO* memio;
#endif
	I8255* pio_t;
//	MEMORY* memory;
	PCM1BIT* pcm0;
	PCM1BIT* pcm1;
	
#if defined(_TK80BS)
	CMT* cmt;
#endif
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMBUS* memory;
	
	// memory
	uint8_t mon[0x800];
	uint8_t ext[0x7000];
	uint8_t ram[0x5000];	// with TK-M20K
#if defined(_TK80BS)
	uint8_t basic[0x2000];
	uint8_t bsmon[0x1000];
	uint8_t vram[0x200];
	
	int boot_mode;
#endif
	
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
#if defined(_TK80BS)
	int max_draw_ranges();
#endif
	
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
	
#if defined(_TK80BS)
	int draw_ranges;
#endif
};

#endif
