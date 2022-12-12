/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ virtual machine ]
*/

#ifndef _J3100_H_
#define _J3100_H_

#if defined(_J3100GT)
#define DEVICE_NAME		"TOSHIBA J-3100GT"
#define CONFIG_NAME		"j3100gt"
#elif defined(_J3100SL)
#define DEVICE_NAME		"TOSHIBA J-3100SL"
#define CONFIG_NAME		"j3100sl"
#endif

// device informations for virtual machine

#if defined(_J3100SL) || defined(_J3100SS) || defined(_J3100SE)
#define TYPE_SL
#endif

// TODO: check refresh rate
#define FRAMES_PER_SEC		59.9
// óví≤ç∏
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		54
#define CPU_CLOCKS		9545456
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define UPD765A_SENCE_INTSTAT_RESULT
#define UPD765A_EXT_DRVSEL
#define SINGLE_MODE_DMA

// device informations for win32
#define USE_FLOPPY_DISK		2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER

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
class I8237;
//class I8250;
class I8253;
class I8259;
#ifdef TYPE_SL
class I86;
#else
class I286;
#endif
class IO;
class PCM1BIT;
class UPD765A;
#ifdef TYPE_SL
class RP5C01;
#else
class HD146818P;
#endif

class DISPLAY;
class DMAREG;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class SASI;
class SYSTEM;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
	I8237* dma;
//	I8250* sio;
	I8253* pit;
	I8259* pic;
#ifdef TYPE_SL
	I86* cpu;
#else
	I286* cpu;
#endif
	IO* io;
	PCM1BIT* pcm;
	UPD765A* fdc;
#ifdef TYPE_SL
	RP5C01* rtc;
#else
	HD146818P* rtc;
	I8237* dma2;
#endif
	
	DISPLAY* display;
	DMAREG* dmareg;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	SASI* sasi;
	SYSTEM* system;
	
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
