/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ virtual machine ]
*/

#ifndef _PASOPIA7_H_
#define _PASOPIA7_H_

#ifdef _LCD
#define DEVICE_NAME		"TOSHIBA PASOPIA 7 with LCD"
#define CONFIG_NAME		"pasopia7lcd"
#else
#define DEVICE_NAME		"TOSHIBA PASOPIA 7"
#define CONFIG_NAME		"pasopia7"
#endif

// device informations for virtual machine
#ifdef _LCD
#define FRAMES_PER_SEC		74.38
#define LINES_PER_FRAME 	32
#define CHARS_PER_LINE		94
#define HD46505_HORIZ_FREQ	(1789780.0 / 752.0)
#else
#define FRAMES_PER_SEC		59.92
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		57
#define HD46505_HORIZ_FREQ	(14318180.0 / 912.0)
#endif
#define CPU_CLOCKS		3993600
#ifdef _LCD
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		128
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#endif
#define MAX_DRIVE		4

// device informations for win32
#define USE_TAPE		1
#define USE_FLOPPY_DISK		2
#define USE_BINARY_FILE		2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	6
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG #1"), _T("PSG #2"), _T("Beep"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class IO;
class LS393;
class NOT;
class PCM1BIT;
class SN76489AN;
class UPD765A;
class Z80;
class Z80CTC;
class Z80PIO;

class FLOPPY;
class DISPLAY;
class IOBUS;
class IOTRAP;
class KEYBOARD;
class MEMORY;
class PAC2;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	HD46505* crtc;
	I8255* pio0;
	I8255* pio1;
	I8255* pio2;
	IO* io;
	LS393* flipflop;
	NOT* not_remote;
	PCM1BIT* pcm;
	SN76489AN* psg0;
	SN76489AN* psg1;
	UPD765A* fdc;
	Z80* cpu;
	Z80CTC* ctc;
	Z80PIO* pio;
	
	FLOPPY* floppy;
	DISPLAY* display;
	IOBUS* iobus;
	IOTRAP* iotrap;
	KEYBOARD* key;
	MEMORY* memory;
	PAC2* pac2;
	
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
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path) {}
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
