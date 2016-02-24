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
#define HD46505_HORIZ_FREQ	15600
#define CPU_CLOCKS		3993600
#define SUB_CPU_CLOCKS		1996800
//#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#define USE_TAPE
#define USE_TAPE_BAUD
#define USE_FD1
#define USE_FD2
//#define USE_FD3
//#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY_SHIFT	2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_CRT_MONITOR_4_3 1
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT"),
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

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	DATAREC* drec;
	HD46505* crtc;
	UPD765A* fdc;
	UPD7801* subcpu;
	Z80* cpu;
	
	MAIN* main;
	SUB* sub;
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
	int get_access_lamp_status();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
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
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	bool is_frame_skippable();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
