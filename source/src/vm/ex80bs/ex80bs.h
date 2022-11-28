/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ virtual machine ]
*/

#ifndef _EX80_H_
#define _EX80_H_

#define DEVICE_NAME		"TOSHIBA EX-80BS"
#define CONFIG_NAME		"ex80bs"

// device informations for virtual machine
#define FRAMES_PER_SEC		59.94
#define LINES_PER_FRAME 	525
#define CPU_CLOCKS		2048000
#define HAS_I8080
#define SCREEN_WIDTH		960
#define SCREEN_HEIGHT		670
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x400
#define IO_ADDR_MAX		0x100

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
//#define MAX_DRAW_RANGES	9
/*
SW1	ON = STEP / OFF = AUTO
SW2	ON = CHAR / OFF = BIT
SW3-1/2	ON ,ON  = 8000H-81FFH
	OFF,ON  = 8200H-83FFH
	ON ,OFF = 8400H-85FFH
	OFF,OFF = 8600H-87FFH
*/
#define USE_BOOT_MODE		2
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0x0e
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#define USE_BINARY_FILE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_MONITOR_TYPE	2
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
};
#endif

#define LED_WIDTH	26
#define LED_HEIGHT	51
#define TV_WIDTH	(32 * 8 * 2 + 8)
#define TV_HEIGHT	(26 * 8 * 2 + 8)

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{763 + 36 * 0, 478 + 34 * 4, 30, 21, 0x80}, // 0
	{763 + 36 * 1, 478 + 34 * 4, 30, 21, 0x81}, // 1
	{763 + 36 * 2, 478 + 34 * 4, 30, 21, 0x82}, // 2
	{763 + 36 * 3, 478 + 34 * 4, 30, 21, 0x83}, // 3
	{763 + 36 * 0, 478 + 34 * 3, 30, 21, 0x84}, // 4
	{763 + 36 * 1, 478 + 34 * 3, 30, 21, 0x85}, // 5
	{763 + 36 * 2, 478 + 34 * 3, 30, 21, 0x86}, // 6
	{763 + 36 * 3, 478 + 34 * 3, 30, 21, 0x87}, // 7
	{763 + 36 * 0, 478 + 34 * 2, 30, 21, 0x88}, // 8
	{763 + 36 * 1, 478 + 34 * 2, 30, 21, 0x89}, // 9
	{763 + 36 * 2, 478 + 34 * 2, 30, 21, 0x8a}, // A
	{763 + 36 * 3, 478 + 34 * 2, 30, 21, 0x8b}, // B
	{763 + 36 * 0, 478 + 34 * 1, 30, 21, 0x8c}, // C
	{763 + 36 * 1, 478 + 34 * 1, 30, 21, 0x8d}, // D
	{763 + 36 * 2, 478 + 34 * 1, 30, 21, 0x8e}, // E
	{763 + 36 * 3, 478 + 34 * 1, 30, 21, 0x8f}, // F
	{763 + 36 * 0, 478 + 34 * 0, 30, 21, 0x98}, // RET
	{763 + 36 * 1, 478 + 34 * 0, 30, 21, 0x99}, // RUN
	{763 + 36 * 2, 478 + 34 * 0, 30, 21, 0x9a}, // SDA
	{763 + 36 * 3, 478 + 34 * 0, 30, 21, 0x9b}, // LDA
	{763 + 36 * 4, 478 + 34 * 0, 30, 21, 0x00}, // RST
	{763 + 36 * 4, 478 + 34 * 1, 30, 21, 0x9c}, // ADR
	{763 + 36 * 4, 478 + 34 * 2, 30, 21, 0x9d}, // RIC
	{763 + 36 * 4, 478 + 34 * 3, 30, 21, 0x9e}, // RDC
	{763 + 36 * 4, 478 + 34 * 4, 30, 21, 0x9f}, // WIC
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{668 + 33 * 0, 295, LED_WIDTH, LED_HEIGHT}, // 7-seg LEDs
	{668 + 33 * 1, 295, LED_WIDTH, LED_HEIGHT},
	{668 + 33 * 2, 295, LED_WIDTH, LED_HEIGHT},
	{668 + 33 * 3, 295, LED_WIDTH, LED_HEIGHT},
	{828 + 33 * 0, 295, LED_WIDTH, LED_HEIGHT},
	{828 + 33 * 1, 295, LED_WIDTH, LED_HEIGHT},
	{828 + 33 * 2, 295, LED_WIDTH, LED_HEIGHT},
	{828 + 33 * 3, 295, LED_WIDTH, LED_HEIGHT},
	{8, 8, TV_WIDTH, TV_HEIGHT}, // TV
};

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8255;
class IO;
class PCM1BIT;
class I8080;

class CMT;
class DISPLAY;
class MEMORY;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8251* sio;
	I8255* pio;
	IO* io;
	PCM1BIT* pcm;
	I8080* cpu;
	
	CMT* cmt;
	DISPLAY* display;
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
	int max_draw_ranges();
	
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
