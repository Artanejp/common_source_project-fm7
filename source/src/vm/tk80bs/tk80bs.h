/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ virtual machine ]
*/

#ifndef _TK80BS_H_
#define _TK80BS_H_

#define DEVICE_NAME		"NEC TK-80BS"
#define CONFIG_NAME		"tk80bs"

// device informations for virtual machine
#define FRAMES_PER_SEC		59.9
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		2048000
#define HAS_I8080
#define I8255_AUTO_HAND_SHAKE
#define SCREEN_WIDTH		784
#define SCREEN_HEIGHT		428
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x200
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
//#define MAX_DRAW_RANGES		9
#define USE_BOOT_MODE		2
//#define USE_DIPSWITCH
//#define DIPSWITCH_DEFAULT	0
#define USE_TAPE
#define TAPE_BINARY_ONLY
#define USE_BINARY_FILE1
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep #1"), _T("Beep #2"),
};
#endif

const struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int font_size;
	int code;
} vm_buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{_T("0"),		523 + 46 * 0, 190 + 46 * 4, 40, 40, 20, 0x80},
	{_T("1"),		523 + 46 * 1, 190 + 46 * 4, 40, 40, 20, 0x81},
	{_T("2"),		523 + 46 * 2, 190 + 46 * 4, 40, 40, 20, 0x82},
	{_T("3"),		523 + 46 * 3, 190 + 46 * 4, 40, 40, 20, 0x83},
	{_T("4"),		523 + 46 * 0, 190 + 46 * 3, 40, 40, 20, 0x84},
	{_T("5"),		523 + 46 * 1, 190 + 46 * 3, 40, 40, 20, 0x85},
	{_T("6"),		523 + 46 * 2, 190 + 46 * 3, 40, 40, 20, 0x86},
	{_T("7"),		523 + 46 * 3, 190 + 46 * 3, 40, 40, 20, 0x87},
	{_T("8"),		523 + 46 * 0, 190 + 46 * 2, 40, 40, 20, 0x88},
	{_T("9"),		523 + 46 * 1, 190 + 46 * 2, 40, 40, 20, 0x89},
	{_T("A"),		523 + 46 * 2, 190 + 46 * 2, 40, 40, 20, 0x8a},
	{_T("B"),		523 + 46 * 3, 190 + 46 * 2, 40, 40, 20, 0x8b},
	{_T("C"),		523 + 46 * 0, 190 + 46 * 1, 40, 40, 20, 0x8c},
	{_T("D"),		523 + 46 * 1, 190 + 46 * 1, 40, 40, 20, 0x8d},
	{_T("E"),		523 + 46 * 2, 190 + 46 * 1, 40, 40, 20, 0x8e},
	{_T("F"),		523 + 46 * 3, 190 + 46 * 1, 40, 40, 20, 0x8f},
	{_T("RET"),		523 + 46 * 0, 190 + 46 * 0, 40, 40, 9,  0x98},
	{_T("RUN"),		523 + 46 * 1, 190 + 46 * 0, 40, 40, 9,  0x99},
	{_T("STORE\nDATA"),	523 + 46 * 2, 190 + 46 * 0, 40, 40, 9,  0x9a},
	{_T("LOAD\nDATA"),	523 + 46 * 3, 190 + 46 * 0, 40, 40, 9,  0x9b},
	{_T("RESET"),		523 + 46 * 4, 190 + 46 * 0, 40, 40, 9,  0x00},
	{_T("ADRS\nSET"),	523 + 46 * 4, 190 + 46 * 1, 40, 40, 9,  0x9c},
	{_T("READ\nINCR"),	523 + 46 * 4, 190 + 46 * 2, 40, 40, 9,  0x9d},
	{_T("READ\nDECR"),	523 + 46 * 4, 190 + 46 * 3, 40, 40, 9,  0x9e},
	{_T("WRITE\nINCR"),	523 + 46 * 4, 190 + 46 * 4, 40, 40, 9,  0x9f},
};
const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{461 + 36 * 0, 28, 33, 46}, // 7-seg LEDs
	{461 + 36 * 1, 28, 33, 46},
	{461 + 36 * 2, 28, 33, 46},
	{461 + 36 * 3, 28, 33, 46},
	{618 + 36 * 0, 28, 33, 46},
	{618 + 36 * 1, 28, 33, 46},
	{618 + 36 * 2, 28, 33, 46},
	{618 + 36 * 3, 28, 33, 46},
	{4, 158, 512, 256}, // CRT
};

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8255;
class IO;
class MEMORY;
class PCM1BIT;
class I8080;

class CMT;
class DISPLAY;
class KEYBOARD;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8251* sio_b;
	I8255* pio_b;
	I8255* pio_t;
	IO* memio;
	MEMORY* memory;
	PCM1BIT* pcm0;
	PCM1BIT* pcm1;
	I8080* cpu;
	
	CMT* cmt;
	DISPLAY* display;
	KEYBOARD* keyboard;
	
	// memory
	uint8_t mon[0x800];
	uint8_t ext[0x7000];
	uint8_t basic[0x2000];
	uint8_t bsmon[0x1000];
	uint8_t ram[0x5000];	// with TK-M20K
	uint8_t vram[0x200];
	
	int boot_mode;
	
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
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
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
	
	int draw_ranges;
};

#endif
