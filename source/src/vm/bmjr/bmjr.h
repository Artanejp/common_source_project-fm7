/*
	HITACH BASIC Master Jr Emulator 'eBASICMasterJr'

	Author : Takeda.Toshiya
	Date   : 2015.08.28-

	[ virtual machine ]
*/

#ifndef _BMJR_H_
#define _BMJR_H_

#define DEVICE_NAME		"HITACHI BASIC Master Jr"
#define CONFIG_NAME		"bmjr"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		754560
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
//#define HAS_MB8861
#define HAS_MC6800

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_TAPE			1
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
//#define USE_SCREEN_FILTER
//#define USE_SCANLINE
#define USE_SOUND_VOLUME	3
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_MC6800

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class MC6800;
class MC6820;
namespace BMJR {
	class MEMORY;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	// devices
	//EVENT* event;
	
	DATAREC* drec;
	MC6800* cpu;
	MC6820* pia;
	
	BMJR::MEMORY* memory;
	
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
	void special_reset(int num) override;
	void run() override;
	double get_frame_rate() override;
	
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
	
	// user interface
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
	bool is_frame_skippable() override;
	
	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	//DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
