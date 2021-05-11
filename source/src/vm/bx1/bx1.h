/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-
*/

#ifndef _BX1_H_
#define _BX1_H_

#define DEVICE_NAME		"CANON BX-1"
#define CONFIG_NAME		"bx1"

// device informations for virtual machine
#define FRAMES_PER_SEC		30
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		500000 // 4MHz / ???
#define SCREEN_WIDTH		((6 * 15 + 5) * 5)
#define SCREEN_HEIGHT		(7 * 5)
#define MAX_DRIVE		1
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x1000
#define IO_ADDR_MAX		0x10000
#define HAS_MC6800

// device informations for win32
#define USE_FLOPPY_DISK		1
#define USE_NUMPAD_ENTER
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	12
#define USE_AUTO_KEY_CAPS
#define DONT_KEEEP_KEY_PRESSED
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class MC6800;
class IO;
class MEMORY;
class MC6843;
class MC6844;

namespace BX1 {
	class DISPLAY;
	class KEYBOARD;
	class PRINTER;
}
class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	MC6800* cpu;
	IO* io;
	MEMORY* memory;
	MC6843* fdc;
	MC6844* dma;
	
	BX1::DISPLAY* display;
	BX1::KEYBOARD* keyboard;
	BX1::PRINTER* printer;
	
	uint8_t bios_9000[0x5000]; // 9000h-DFFFh
	uint8_t bios_f000[0x1000]; // F000h-FFFFh
	uint8_t ram[0x4000];
	
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
