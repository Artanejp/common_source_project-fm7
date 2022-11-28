/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ virtual machine ]
*/

#ifndef _PV2000_H_
#define _PV2000_H_

#define DEVICE_NAME		"CASIO PV-2000"
#define CONFIG_NAME		"pv2000"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		3579545
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
#define TMS9918A_VRAM_SIZE	0x4000
//#define TMS9918A_LIMIT_SPRITES
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x1000

// device informations for win32
#define USE_CART		1
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	1
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class IO;
class MEMORY;
class SN76489AN;
class TMS9918A;
class Z80;

class CMT;
class KEYBOARD;
class PRINTER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	IO* io;
	MEMORY* memory;
	SN76489AN* psg;
	TMS9918A* vdp;
	Z80* cpu;
	
	CMT* cmt;
	KEYBOARD* key;
	PRINTER* prt;
	
	// memory
	uint8_t ipl[0x4000];	// ipl (16k)
	uint8_t ram[0x1000];	// ram (4k)
	uint8_t ext[0x4000];	// ext ram/rom (16k)
	uint8_t cart[0x4000];	// cartridge (16k)
	bool inserted;
	
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
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
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
