/*
	TOSHIBA T-200/250 Emulator 'eT-250'

	Author : Takeda.Toshiya
	Date   : 2023.12.29-

	[ virtual machine ]
*/

#ifndef _T250_H_
#define _T250_H_

#ifdef _T200
#define DEVICE_NAME		"TOSHIBA T-200"
#define CONFIG_NAME		"t200"
#else
#define DEVICE_NAME		"TOSHIBA T-250"
#define CONFIG_NAME		"t250"
#endif

//#define SUPPORT_CCM

// device informations for virtual machine
#define FRAMES_PER_SEC		59.92
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		102
#define CPU_CLOCKS		2666666
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define HAS_I8085
#define HD46505_HORIZ_FREQ	(14318180.0 / 912.0)
#define MAX_DRIVE		4

// device informations for win32
#define USE_DIPSWITCH
#define DIPSWITCH_DEFAULT	0
#define USE_FLOPPY_DISK		2
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("Noise (c)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class DISK;
class HD46505;
class I8080;
#ifdef SUPPORT_CCM
class I8251;
class I8253;
#endif
class I8257;
class I8279;
class IO;
class UPD765A;

#ifdef SUPPORT_CCM
class CCM;
#endif
class MEMBUS;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	HD46505* crtc;
	I8080* cpu;
#ifdef SUPPORT_CCM
	I8251* sio;
	I8253* pit;
#endif
	I8257* dma;
	I8279* kbc;
	IO* io;
	UPD765A* fdc;
	
#ifdef SUPPORT_CCM
	CCM* ccm;
#endif
	MEMBUS* memory;
	
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
	bool get_caps_locked();
	bool get_kana_locked();
	
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
