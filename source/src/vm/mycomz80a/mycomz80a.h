/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

	[ virtual machine ]
*/

#ifndef _MYCOMZ80A_H_
#define _MYCOMZ80A_H_

#define DEVICE_NAME		"Japan Electronics College MYCOMZ-80A"
#define CONFIG_NAME		"mycomz80a"

// device informations for virtual machine
#define FRAMES_PER_SEC		60.58
#define LINES_PER_FRAME 	260
#define CHARS_PER_LINE		64
#define HD46505_CHAR_CLOCK	1008000
#define CPU_CLOCKS		2500000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define SUPPORT_VARIABLE_TIMING
#define HAS_MSM5832

// device informations for win32
#define USE_TAPE1
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define SUPPORT_ROMA_KANA_CONVERSION

#define USE_SOUND_VOLUME	3
#define SUPPORT_TV_RENDER
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class IO;
class MSM5832;
class SN76489AN;
class Z80;

class DISPLAY;
class KEYBOARD;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	HD46505* crtc;
	I8255* pio1;
	I8255* pio2;
	I8255* pio3;
	IO* io;
	MSM5832* rtc;
	SN76489AN* psg;
	Z80* cpu;
	
	DISPLAY* display;
	KEYBOARD* keyboard;
	MEMORY* memory;
	
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
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
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
