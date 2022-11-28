/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-
*/

#ifndef _BX1_H_
#define _BX1_H_

#if defined(_AX1)
#define DEVICE_NAME		"CANON AX-1"
#define CONFIG_NAME		"ax1"
#elif defined(_BX1)
#define DEVICE_NAME		"CANON BX-1"
#define CONFIG_NAME		"bx1"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		500000 // 4MHz / ???
#define SCREEN_WIDTH		1280
#define SCREEN_HEIGHT		576
#define MAX_DRIVE		1
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x400
#define IO_ADDR_MAX		0x10000
#define HAS_MC6800

// device informations for win32
#define ONE_BOARD_MICRO_COMPUTER
#define MAX_BUTTONS		100
#define MAX_DRAW_RANGES		19
#define USE_DIPSWITCH
#define USE_FLOPPY_DISK		1
#define USE_NUMPAD_ENTER
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	12
#define USE_AUTO_KEY_CAPS
#define DONT_KEEEP_KEY_PRESSED
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Noise (FDD)"),
};
#endif

const struct {
	int x, y;
	int width, height;
	int code;
} vm_buttons[] = {
	{ 891, 446,  51,  51, 0x61}, // 1
	{ 951, 446,  51,  51, 0x62}, // 2
	{1011, 446,  51,  51, 0x63}, // 3
	{ 891, 382,  51,  51, 0x64}, // 4
	{ 951, 382,  51,  51, 0x65}, // 5
	{1011, 382,  51,  51, 0x66}, // 6
	{ 891, 318,  51,  51, 0x67}, // 7
	{ 951, 318,  51,  51, 0x68}, // 8
	{1011, 318,  51,  51, 0x69}, // 9
	{ 891, 253,  51,  51, 0x78}, // EXP
	{ 951, 253,  51,  51, 0x79}, // SIGN CHG
	{1011, 253,  51,  51, 0x7A}, // CE
	{ 853, 510, 120,  44, 0x60}, // 0
	{ 979, 510,  82,  44, 0x6E}, // .
	{1081, 318,  51, 235, 0x7B}, // START
	{  75, 458,  51,  39, 0xA2}, // CAP
	{  56, 400,  51,  39, 0xA0}, // SML
	{  37, 342,  51,  39, 0x14}, // CTRL
	{  18, 284,  51,  39, 0x09}, // INST
	{ 133, 458,  51,  39, 0x5A}, // Z
	{ 114, 400,  51,  39, 0x41}, // A
	{  95, 342,  51,  39, 0x51}, // Q
	{  76, 284,  51,  39, 0x31}, // ! 1
	{ 191, 458,  51,  39, 0x58}, // X
	{ 172, 400,  51,  39, 0x53}, // S
	{ 153, 342,  51,  39, 0x57}, // W
	{ 134, 284,  51,  39, 0x32}, // " 2
	{ 249, 458,  51,  39, 0x43}, // C
	{ 230, 400,  51,  39, 0x44}, // D
	{ 211, 342,  51,  39, 0x45}, // E
	{ 192, 284,  51,  39, 0x33}, // # 3
	{ 307, 458,  51,  39, 0x56}, // V
	{ 288, 400,  51,  39, 0x46}, // F
	{ 269, 342,  51,  39, 0x52}, // R
	{ 250, 284,  51,  39, 0x34}, // $ 4
	{ 365, 458,  51,  39, 0x42}, // B
	{ 346, 400,  51,  39, 0x47}, // G
	{ 327, 342,  51,  39, 0x54}, // T
	{ 308, 284,  51,  39, 0x35}, // % 5
	{ 423, 458,  51,  39, 0x4E}, // N
	{ 404, 400,  51,  39, 0x48}, // H
	{ 385, 342,  51,  39, 0x59}, // Y
	{ 366, 284,  51,  39, 0x36}, // & 6
	{ 481, 458,  51,  39, 0x4D}, // M
	{ 462, 400,  51,  39, 0x4A}, // J
	{ 443, 342,  51,  39, 0x55}, // U
	{ 424, 284,  51,  39, 0x37}, // ' 7
	{ 539, 458,  51,  39, 0xBC}, // ,
	{ 520, 400,  51,  39, 0x4B}, // K
	{ 501, 342,  51,  39, 0x49}, // I
	{ 482, 284,  51,  39, 0x38}, // [ 8
	{ 597, 458,  51,  39, 0xBE}, // ?
	{ 578, 400,  51,  39, 0x4C}, // L
	{ 559, 342,  51,  39, 0x4F}, // O
	{ 540, 284,  51,  39, 0x39}, // \ 9
	{ 655, 458,  51,  39, 0xBF}, // ^
	{ 636, 400,  51,  39, 0xBB}, // ;
	{ 617, 342,  51,  39, 0x50}, // P
	{ 598, 284,  51,  39, 0x30}, // ] 0
	{ 713, 458,  51,  39, 0xA3}, // UC
	{ 694, 400,  51,  39, 0xBA}, // :
	{ 675, 342,  51,  39, 0xC0}, // _
	{ 656, 284,  51,  39, 0xBD}, // @
	{ 365, 515, 109,  39, 0x20}, // SPACE
	{ 733, 342, 107,  39, 0x0D}, // CR/LF
	{ 714, 284,  51,  39, 0xDE}, // <
	{ 772, 284,  51,  39, 0xDC}, // >
	{1151, 253,  51,  51, 0x24}, // alpha^x
	{1151, 318,  51,  51, 0x23}, // (
	{1151, 382,  51,  51, 0x6A}, // *
	{1151, 446,  51,  51, 0x6B}, // +
	{1208, 253,  51,  51, 0x21}, // SQRT
	{1208, 318,  51,  51, 0x22}, // )
	{1208, 382,  51,  51, 0x6F}, // /
	{1208, 446,  51,  51, 0x6D}, // -
	{1151, 510, 108,  44, 0x6C}, // =
	{ 951, 167, 107,  32, 0x91}, // PAPER FEED
	{  62, 158,  51,  51, 0x1B}, // CLEAR
	{ 131, 136,  51,  32, 0x7C}, // PROGRAM
	{ 131, 198,  51,  32, 0x70}, // RUN
	{ 189, 136,  51,  32, 0x7D}, // NEW
	{ 189, 198,  51,  32, 0x71}, // RENAME
	{ 247, 136,  51,  32, 0x7E}, // SECURE
	{ 247, 198,  51,  32, 0x72}, // PROTECT
	{ 324, 136,  51,  32, 0x7F}, // OPERATE
	{ 324, 198,  51,  32, 0x73}, // PROG LIST
	{ 382, 136,  51,  32, 0x80}, // TRACE
	{ 382, 198,  51,  32, 0x74}, // STOP
	{ 459, 136,  51,  32, 0x81}, // DISK LIST
	{ 459, 198,  51,  32, 0x75}, // CONDENSE
	{ 517, 136,  51,  32, 0x82}, // RECALL
	{ 517, 198,  51,  32, 0x76}, // LINE NO.
	{ 594, 136,  51,  32, 0x2D}, // INSERT
	{ 594, 198,  51,  32, 0x2E}, // DELETE
	{ 652, 136,  51,  32, 0x26}, // UP
	{ 652, 198,  51,  32, 0x25}, // LEFT
	{ 710, 136,  51,  32, 0x28}, // DOWN
	{ 710, 198,  51,  32, 0x27}, // RIGHT
	{ 802, 136, 107,  32, 0x83}, // AUTO PRINT
	{ 802, 198, 107,  32, 0x77}, // PROG.SELECT
};

const struct {
	int x, y;
	int width, height;
} vm_ranges[] = {
	{320, 32, 33, 47},
	{361, 32, 33, 47},
	{402, 32, 33, 47},
	{443, 32, 33, 47},
	{484, 32, 33, 47},
	{525, 32, 33, 47},
	{566, 32, 33, 47},
	{607, 32, 33, 47},
	{648, 32, 33, 47},
	{689, 32, 33, 47},
	{730, 32, 33, 47},
	{771, 32, 33, 47},
	{812, 32, 33, 47},
	{853, 32, 33, 47},
	{894, 32, 33, 47},
	{935, 32, 33, 47},
	{ 97, 53,  7,  7},
	{171, 53,  7,  7},
	{245, 53,  7,  7},
};

class EMU;
class DEVICE;
class EVENT;

class MC6800;
class IO;
class MEMORY;
class MC6843;
class MC6844;

class DISPLAY;
class FLOPPY;
class KEYBOARD;
class PRINTER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	MC6800* cpu;
	IO* io;
	MEMORY* memory;
	MC6843* fdc;
	MC6844* dma;
	
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	PRINTER* printer;
	
	uint8_t cart_5000[0x1000]; // 5000h-5FFFh
	uint8_t cart_6000[0x1000]; // 6000h-6FFFh
	uint8_t cart_7000[0x1000]; // 7000h-7FFFh
	uint8_t cart_8000[0x1000]; // 8000h-8FFFh
	uint8_t bios_9000[0x5000]; // 9000h-DFFFh (Integrated)
	uint8_t bios_f000[0x1000]; // F000h-FFFFh (Integrated)
	uint8_t ram[0x5000];
	
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
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
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
