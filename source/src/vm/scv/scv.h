/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ virtual machine ]
*/

#ifndef _SCV_H_
#define _SCV_H_

#define DEVICE_NAME		"EPOCH SCV"
#define CONFIG_NAME		"scv"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		192
#define SCREEN_HEIGHT		222
#define WINDOW_WIDTH_ASPECT	288

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_CART			1
#define USE_SOUND_VOLUME	2
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_UPD7801

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("PCM"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class UPD7801;

namespace SCV {
	class IO;
	class MEMORY;
	class SOUND;
	class VDP;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;

	// devices
	//EVENT* event;

	UPD7801* cpu;

	SCV::IO* io;
	SCV::MEMORY* memory;
	SCV::SOUND* sound;
	SCV::VDP* vdp;

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
	void reset();
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
	bool is_frame_skippable();

	double get_current_usec();
	uint64_t get_current_clock_uint64();

	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
