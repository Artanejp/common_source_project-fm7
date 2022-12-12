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
#define USE_TAPE		1
#define USE_FLOPPY_DISK		2
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

class MAIN;
class SUB;
class FDCPACK;
class RAMPACK;
class ROMPACK;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	DATAREC* drec;
	HD46505* crtc;
	UPD765A* fdc;
	UPD7801* subcpu;
	Z80* maincpu;
	
	MAIN* mainbus;
	SUB* subbus;
	FDCPACK* fdcpack;
	RAMPACK* rampack1;
	RAMPACK* rampack2;
	RAMPACK* rampack3;
	RAMPACK* rampack4;
	RAMPACK* rampack5;
	RAMPACK* rampack6;
	ROMPACK* rompack;
	
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
	double get_frame_rate();
	
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
