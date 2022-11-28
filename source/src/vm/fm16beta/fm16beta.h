/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ virtual machine ]
*/

#ifndef _FM16BETA_H_
#define _FM16BETA_H_

#if defined(HAS_I186)
#define DEVICE_NAME		"FUJITSU FM16beta (i186)"
#define CONFIG_NAME		"fm16beta_i186"
#elif defined(HAS_I286)
#define DEVICE_NAME		"FUJITSU FM16beta (i286)"
#define CONFIG_NAME		"fm16beta_i286"
#endif

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		55.38
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		54
#define HD46505_HORIZ_FREQ	(21052600.0 / 864)

#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
//#define MB8877_NO_BUSY_AFTER_SEEK

#if defined(HAS_I186)
#define MEMORY_ADDR_MAX		0x100000	// 1MB
#elif defined(HAS_I286)
#define MEMORY_ADDR_MAX		0x1000000	// 16MB
#endif
#define MEMORY_BANK_SIZE	0x4000

#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FLOPPY_DISK		4
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

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
class I8251;
class I8259;
#if defined(HAS_I186)
class I86;
#elif defined(HAS_I286)
class I286;
#endif
class IO;
class MB8877;
class MC6809;
class MC6840;
class MSM58321;
class PCM1BIT;

class CMOS;
class KEYBOARD;
class MAIN;
class SUB;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	HD46505* crtc;
	I8237* dma;
	I8251* sio;
	I8259* pic;
#if defined(HAS_I186)
	I86* cpu;
#elif defined(HAS_I286)
	I286* cpu;
#endif
	IO* io;
	MB8877* fdc_2hd;
	MB8877* fdc_2d;
	MC6809* subcpu;
	MC6840* ptm;
	MSM58321* rtc;
	PCM1BIT* pcm;
	
	CMOS* cmos;
	MAIN* mainbus;
	KEYBOARD* keyboard;
	SUB* subbus;
	
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
