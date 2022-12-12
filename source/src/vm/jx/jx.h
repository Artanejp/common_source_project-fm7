/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ virtual machine ]
*/

#ifndef _JX_H_
#define _JX_H_

#define DEVICE_NAME		"IBM Japan Ltd PC/JX"
#define CONFIG_NAME		"jx"

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		59.9
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		57
#define CPU_CLOCKS		4772727
//#define CPU_CLOCKS		4770000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		2
#define UPD765A_SENCE_INTSTAT_RESULT

// device informations for win32
#define USE_FLOPPY_DISK		2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

#define KEYBOARD_HACK
#define TIMER_HACK

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class HD46505;
class I8251;
class I8253;
class I8255;
class I8259;
class I86;
class IO;
class MEMORY;
class PCM1BIT;
class SN76489AN;
class UPD765A;

class DISPLAY;
class FLOPPY;
class KEYBOARD;
class SPEAKER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
	I8251* sio;
	I8253* pit;
	I8255* pio;
	I8259* pic;
	I86* cpu;
	IO* io;
	MEMORY* mem;
	PCM1BIT* pcm;
	SN76489AN* psg;
	UPD765A* fdc;
	
	DISPLAY* display;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	SPEAKER* speaker;
	
	// memory
	uint8_t font[0x800];
	uint8_t kanji[0x38000];
	uint8_t ram[0x80000];
	uint8_t ipl[0x30000];
	
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
