/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#ifndef _MZ80K_H_
#define _MZ80K_H_

#if defined(_MZ1200)
#define DEVICE_NAME		"SHARP MZ-1200"
#define CONFIG_NAME		"mz1200"
#elif defined(_MZ80A)
#define DEVICE_NAME		"SHARP MZ-80A"
#define CONFIG_NAME		"mz80a"
#else
#define DEVICE_NAME		"SHARP MZ-80K/C"
#define CONFIG_NAME		"mz80k"
#endif

#ifdef _MZ80A
#define SUPPORT_MZ80AIF
#else
#define SUPPORT_MZ80FIO
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		2000000
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define WINDOW_HEIGHT_ASPECT	240
#if defined(SUPPORT_MZ80AIF)
#define HAS_MB8866
#define MAX_DRIVE		4
#elif defined(SUPPORT_MZ80FIO)
#define HAS_T3444M
#define MAX_DRIVE		4
#endif
#define PRINTER_STROBE_RISING_EDGE

// device informations for win32
#define USE_DIPSWITCH
#define USE_TAPE
#define USE_TAPE_BUTTON
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_ACCESS_LAMP
#endif
#if defined(_MZ80K)
#define USE_MONITOR_TYPE	2
#endif

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

#if defined(_MZ1200) || defined(_MZ80A)
class AND;
#endif
class DATAREC;
class I8253;
class I8255;
class LS393;
class PCM1BIT;
class Z80;

class KEYBOARD;
class MEMORY;
class PRINTER;

#if defined(SUPPORT_MZ80AIF)
class MB8877;
class IO;
class MZ80AIF;
#elif defined(SUPPORT_MZ80FIO)
class T3444A;
class IO;
class MZ80FIO;
#endif

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
#if defined(_MZ1200) || defined(_MZ80A)
	AND* and_int;
#endif
	DATAREC* drec;
	I8253* ctc;
	I8255* pio;
	LS393* counter;
	PCM1BIT* pcm;
	Z80* cpu;
	
	KEYBOARD* keyboard;
	MEMORY* memory;
	PRINTER* printer;
	
#if defined(SUPPORT_MZ80AIF)
	MB8877* fdc;
	IO* io;
	MZ80AIF* mz80aif;
#elif defined(SUPPORT_MZ80FIO)
	T3444A* fdc;
	IO* io;
	MZ80FIO* mz80fio;
#endif
	
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
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	uint32_t get_access_lamp_status();
#endif
	
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
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
#endif
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward() {}
	void push_apss_rewind() {}
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
