/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ virtual machine ]
*/

#ifndef _FP1100_H_
#define _FP1100_H_

#define DEVICE_NAME		"CASIO FP-1100"
#define CONFIG_NAME		"fp1100"

// device informations for virtual machine
#define FRAMES_PER_SEC		59.77
#define LINES_PER_FRAME 	261
#define CHARS_PER_LINE		128
#define HD46505_CHAR_CLOCK	998400
#define CPU_CLOCKS		3993600
#define SUB_CPU_CLOCKS		1996800

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_TAPE		1
#define USE_FLOPPY_DISK		2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY_SHIFT	2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	4
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_UPD7801
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class DATAREC;
class HD46505;
class UPD765A;
class UPD7801;
class Z80;

namespace FP1100 {
	class MAIN;
	class SUB;
	class FDCPACK;
	class RAMPACK;
	class ROMPACK;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils *state_entry;

	// devices
	//EVENT* event;

	BEEP* beep;
	DATAREC* drec;
	HD46505* crtc;
	UPD765A* fdc;
	UPD7801* subcpu;
	Z80* maincpu;

	FP1100::MAIN* mainbus;
	FP1100::SUB* subbus;
	FP1100::FDCPACK* fdcpack;
	FP1100::RAMPACK* rampack1;
	FP1100::RAMPACK* rampack2;
	FP1100::RAMPACK* rampack3;
	FP1100::RAMPACK* rampack4;
	FP1100::RAMPACK* rampack5;
	FP1100::RAMPACK* rampack6;
	FP1100::ROMPACK* rompack;

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
	//void reset() override;
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
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
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
	void push_apss_forward(int drv) override {}
	void push_apss_rewind(int drv) override {}
	bool is_frame_skippable() override;

	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

	//void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	//DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
