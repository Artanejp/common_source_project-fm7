/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ virtual machine ]
*/

#ifndef _GAMEGEAR_H_
#define _GAMEGEAR_H_

#define DEVICE_NAME		"SEGA GAME GEAR"
#define CONFIG_NAME		"gamegear"

// device informations for virtual machine
#define FRAMES_PER_SEC			60
#define LINES_PER_FRAME			262
#define CPU_CLOCKS				3579545
#define SCREEN_WIDTH			256
#define SCREEN_HEIGHT			192
#define TMS9918A_VRAM_SIZE		0x4000
#define TMS9918A_LIMIT_SPRITES
#define MAX_DRIVE				1

// device informations for win32
#define USE_CART		1
#define USE_FLOPPY_DISK		1
#define USE_TAPE		1
#define USE_AUTO_KEY			5
#define USE_AUTO_KEY_RELEASE	8
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	4
#define USE_JOYSTICK
#define USE_DEBUGGER

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8251;
class I8255;
class IO;
class SN76489AN;
class _315_5124;
class UPD765A;
class Z80;

class KEYBOARD;
class MEMORY;
class SYSTEM;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8251* sio;
	I8255* pio_k;
	I8255* pio_f;
	IO* io;
	SN76489AN* psg;
	_315_5124* vdp;
	UPD765A* fdc;
	Z80* cpu;
	
	KEYBOARD* key;
	MEMORY* memory;
	SYSTEM* system;
	
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
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
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
