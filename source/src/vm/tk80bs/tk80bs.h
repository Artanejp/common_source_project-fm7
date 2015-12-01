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
#define SCREEN_WIDTH		768
#define SCREEN_HEIGHT		800
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x200
#define IO_ADDR_MAX		0x10000
#define SCREEN_WIDTH_ASPECT SCREEN_WIDTH 
#define SCREEN_HEIGHT_ASPECT SCREEN_HEIGHT
#define WINDOW_WIDTH_ASPECT 256
#define WINDOW_HEIGHT_ASPECT 192 

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		25
#define MAX_DRAW_RANGES		9
#define BITMAP_OFFSET_Y		384
#define USE_BOOT_MODE		2
#define USE_TAPE
#define TAPE_BINARY_ONLY
#define USE_BINARY_FILE1
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_DEBUGGER

#include "../../common.h"
#include "../../fileio.h"

const struct {
	const _TCHAR* caption;
	int x, y;
	int width, height;
	int font_size;
	int code;
} buttons[] = {
	// virtual key codes 0x80-0x8f and 0x98-0x9f are not used in pc keyboard
	{_T("0"), 512, 750, 40, 40, 20, 0x80},
	{_T("1"), 557, 750, 40, 40, 20, 0x81},
	{_T("2"), 602, 750, 40, 40, 20, 0x82},
	{_T("3"), 647, 750, 40, 40, 20, 0x83},
	{_T("4"), 512, 705, 40, 40, 20, 0x84},
	{_T("5"), 557, 705, 40, 40, 20, 0x85},
	{_T("6"), 602, 705, 40, 40, 20, 0x86},
	{_T("7"), 647, 705, 40, 40, 20, 0x87},
	{_T("8"), 512, 660, 40, 40, 20, 0x88},
	{_T("9"), 557, 660, 40, 40, 20, 0x89},
	{_T("A"), 602, 660, 40, 40, 20, 0x8a},
	{_T("B"), 647, 660, 40, 40, 20, 0x8b},
	{_T("C"), 512, 615, 40, 40, 20, 0x8c},
	{_T("D"), 557, 615, 40, 40, 20, 0x8d},
	{_T("E"), 602, 615, 40, 40, 20, 0x8e},
	{_T("F"), 647, 615, 40, 40, 20, 0x8f},
	{_T("RET"),         512, 570, 40, 40, 9, 0x98},
	{_T("RUN"),         557, 570, 40, 40, 9, 0x99},
	{_T("STORE\nDATA"), 602, 570, 40, 40, 9, 0x9a},
	{_T("LOAD\nDATA"),  647, 570, 40, 40, 9, 0x9b},
	{_T("RESET"),       692, 570, 40, 40, 9, 0x00},
	{_T("ADRS\nSET"),   692, 615, 40, 40, 9, 0x9c},
	{_T("READ\nINCR"),  692, 660, 40, 40, 9, 0x9d},
	{_T("READ\nDECR"),  692, 705, 40, 40, 9, 0x9e},
	{_T("WRITE\nINCR"), 692, 750, 40, 40, 9, 0x9f},
};
const struct {
	int x, y;
	int width, height;
} ranges[] = {
	{451,  410, 32,  45 }, // 7-seg LEDs
	{486,  410, 32,  45 },
	{521,  410, 32,  45 },
	{556,  410, 32,  45 },
	{607,  410, 32,  45 },
	{642,  410, 32,  45 },
	{677,  410, 32,  45 },
	{712,  410, 32,  45 },
	{0,    0,   768, 384}, // CRT
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
	uint8 mon[0x800];
	uint8 ext[0x7000];
	uint8 basic[0x2000];
	uint8 bsmon[0x1000];
	uint8 ram[0x5000];	// with TK-M20K
	uint8 vram[0x200];
	
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
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool now_skip();
	
	void update_config();
	
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
