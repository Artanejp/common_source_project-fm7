/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ virtual machine ]
*/

#ifndef _YIS_H_
#define _YIS_H_

#define DEVICE_NAME		"YAMAHA YIS"
#define CONFIG_NAME		"yis"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
// NDK 7.2MHz/4 ???
#define CPU_CLOCKS		1800000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
//#define WINDOW_HEIGHT_ASPECT	480
#define HAS_MSM5832
#define MAX_DRIVE		2
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x100
#define MEMORY_DISABLE_DMA_MMIO
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FLOPPY_DISK		2
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS_LOCK	(0xf0 | 0x100)
#define USE_AUTO_KEY_NUMPAD
#define USE_VM_AUTO_KEY_TABLE
#define USE_MONITOR_TYPE	3
#define USE_SCREEN_FILTER
//#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

static const int vm_auto_key_table_base[][2] = {
	{0xb0,	0x300 | 0xba},	// '°' -> ':'+SHIFT
	{0xb9,	0x200 | 0x4c},	// '¹' -> 'L'
	{0xcd,	0x200 | 0xbd},	// 'Í' -> '-'
	{0xce,	0x200 | 0xba},	// 'Î' -> ':'
	{0xd1,	0x200 | 0xbb},	// 'Ñ' -> ';'
	{0xd8,	0x300 | 0x4b},	// 'Ø' -> 'K'+SHIFT
	{0xdb,	0x300 | 0xbb},	// 'Û' -> ';'+SHIFT
	{0xda,	0x300 | 0x4c},	// 'Ú' -> 'L'+SHIFT
	{0xde,	0x300 | 0x4f},	// 'Þ' -> 'O'+SHIFT
	{0xdf,	0x300 | 0x50},	// 'ß' -> 'P'+SHIFT
	{-1,	-1},
};

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class M6502;
class IO;
class MEMORY;
class AM9511;
class BEEP;
class MB8877;
class MC6820;
class MC6844;
class MC6850;
class MSM58321;

class CALENDAR;
class DISPLAY;
class FLOPPY;
class KEYBOARD;
class MAPPER;
class SOUND;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	M6502* cpu;
	IO* io;
	MEMORY* memory;
	AM9511* apu;
	BEEP* beep;
	MB8877* fdc;
	MC6820* pia;
	MC6844* dma;
	MC6850* acia1;
	MC6850* acia2;
	MSM58321* rtc;
	
	CALENDAR* calendar;
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MAPPER* mapper;
	SOUND* sound;
	
	uint8_t rom[0x1000];
	
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
