/*
	SEGA MASTER SYSTEM Emulator 'yaMASTER SYSTEM'

	Author : tanam
	Date   : 2013.10.20-

	[ virtual machine ]
*/

#ifndef _MASTERSYSTEM_H_
#define _MASTERSYSTEM_H_

#define DEVICE_NAME		"SEGA MASTER SYSTEM"
#define CONFIG_NAME		"mastersystem"

// device informations for virtual machine
#define FRAMES_PER_SEC			60
#define LINES_PER_FRAME			262
#define CPU_CLOCKS				3579545
#define SCREEN_WIDTH			256
#define SCREEN_HEIGHT			192
#define TMS9918A_VRAM_SIZE		0x4000
#define TMS9918A_LIMIT_SPRITES
///#define MAX_DRIVE			1

// device informations for win32
#define USE_CART		1
//#define USE_FLOPPY_DISK	1
//#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	8
#define USE_AUTO_KEY_CAPS
#define SUPPORT_TV_RENDER
#define USE_SOUND_VOLUME	2
///#define USE_SOUND_VOLUME	4
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("OPLL"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

///class DATAREC;
///class I8251;
class I8255;
class IO;
class YM2413;
class SN76489AN;
class _315_5124;
///class UPD765A;
class Z80;

namespace GAMEGEAR {
	class KEYBOARD;
	class MEMORY;
	class SYSTEM;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	
	// devices
	//EVENT* event;
	
///	DATAREC* drec;
///	I8251* sio;
	I8255* pio_k;
	I8255* pio_f;
	IO* io;
	YM2413* fm;
	SN76489AN* psg;
	_315_5124* vdp;
///	UPD765A* fdc;
	Z80* cpu;
	
	GAMEGEAR::KEYBOARD* key;
	GAMEGEAR::MEMORY* memory;
	GAMEGEAR::SYSTEM* system;
	
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
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path) override;
	void close_cart(int drv) override;
	bool is_cart_inserted(int drv) override;
///	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
///	void close_floppy_disk(int drv) override;
///	bool is_floppy_disk_inserted(int drv) override;
///	void is_floppy_disk_protected(int drv, bool value) override;
///	bool is_floppy_disk_protected(int drv) override;
///	uint32_t is_floppy_disk_accessed() override;
///	void play_tape(int drv, const _TCHAR* file_path) override;
///	void rec_tape(int drv, const _TCHAR* file_path) override;
///	void close_tape(int drv) override;
///	bool is_tape_inserted(int drv) override;
///	bool is_tape_playing(int drv) override;
///	bool is_tape_recording(int drv) override;
///	int get_tape_position(int drv) override;
///	const _TCHAR* get_tape_message(int drv) override;
///	void push_play(int drv) override;
///	void push_stop(int drv) override;
///	void push_fast_forward(int drv) override;
///	void push_fast_rewind(int drv) override;
///	void push_apss_forward(int drv)  override {}
///	void push_apss_rewind(int drv)  override {}
	bool is_frame_skippable() override;
	
	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;
	
	void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};
#endif
