/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ virtual machine ]
*/

#ifndef _CRC80_H_
#define _CRC80_H_

#define DEVICE_NAME		"Computer Research CRC-80"
#define CONFIG_NAME		"crc80"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME 	256
#define CPU_CLOCKS		2500000
#define SCREEN_WIDTH		768
#define SCREEN_HEIGHT		512
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x400
#define IO_ADDR_MAX		0x100

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		6
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0
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

#define BUTTON_SPACE_X	68
#define BUTTON_SPACE_Y	53
#define BUTTON_SIZE_X	45
#define BUTTON_SIZE_Y	35
#define BUTTON_POS_X	432
#define BUTTON_POS_Y	251

#define LED_SPACE_X	38
#define LED_SIZE_X	14
#define LED_SIZE_Y	15
#define LED_POS_X1	512
#define LED_POS_X2	678
#define LED_POS_Y	48

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
	{BUTTON_POS_X + BUTTON_SPACE_X * 0, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x70},	// GO
	{BUTTON_POS_X + BUTTON_SPACE_X * 1, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x71},	// DA
	{BUTTON_POS_X + BUTTON_SPACE_X * 2, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x72},	// AD
	{BUTTON_POS_X + BUTTON_SPACE_X * 3, BUTTON_POS_Y + BUTTON_SPACE_Y * 0, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x73},	// INC
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 4, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x77},	// ST
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 3, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x76},	// STP
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 2, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x75},	// LD
	{BUTTON_POS_X + BUTTON_SPACE_X * 4, BUTTON_POS_Y + BUTTON_SPACE_Y * 1, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0x74},	// DEC
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
};

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class IO;
class NOT;
class Z80;
class Z80PIO;

namespace CRC80 {
class DISPLAY;
class MEMBUS;
};

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	IO* io;
	NOT* m_not;
	Z80* cpu;
	Z80PIO* pio;
	
	CRC80::DISPLAY* display;
	CRC80::MEMBUS* memory;
	
	// memory
	uint8_t mon[0x400];
	uint8_t tty[0x400];
	uint8_t ext[0x400];
	uint8_t ram[0x1000];
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU_TEMPLATE* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset() override;
	void run() override;
	double get_frame_rate() override
	{
		return FRAMES_PER_SEC;
	}
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index) override;
#endif
	
	// draw screen
	void draw_screen() override;
	
	// sound generation
	void initialize_sound(int rate, int samples) override;
	uint16_t* create_sound(int* extra_frames) override;
	int get_sound_buffer_ptr() override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r) override;
#endif
	
	// notify key
	void key_down(int code, bool repeat) override;
	void key_up(int code) override;
	bool get_caps_locked() override;
	bool get_kana_locked() override;
	
	// user interface
	void load_binary(int drv, const _TCHAR* file_path) override;
	void save_binary(int drv, const _TCHAR* file_path) override;
	void play_tape(int drv, const _TCHAR* file_path) override;
	void rec_tape(int drv, const _TCHAR* file_path) override;
	void close_tape(int drv) override;
	bool is_tape_inserted(int drv) override;
	bool is_tape_playing(int drv) override;
	bool is_tape_recording(int drv) override;
	int get_tape_position(int drv) override;
	const _TCHAR* get_tape_message(int drv) override;
	void push_play(int drv) override;
	void push_stop(int drv) override;
	void push_fast_forward(int drv) override;
	void push_fast_rewind(int drv) override;
	void push_apss_forward(int drv) override{}
	void push_apss_rewind(int drv) override{}
	bool is_frame_skippable() override;
	
	void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id) override;
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
