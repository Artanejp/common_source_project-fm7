/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ virtual machine ]
*/

#ifndef _BUBCOM80_H_
#define _BUBCOM80_H_

#define DEVICE_NAME		"Systems Formulate BUBCOM80"
#define CONFIG_NAME		"bubcom80"

// device informations for virtual machine
#define FRAMES_PER_SEC		62.422
#define LINES_PER_FRAME 	260
//#define CPU_CLOCKS		3993624
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4

// device informations for win32
#define USE_FLOPPY_DISK		4
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#define USE_BUBBLE		2
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	2
#define USE_JOYSTICK
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
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

class IO;
class LS393;
class MB8877;
class MC6850;
class MEMORY;
class PCM1BIT;
class Z80;
class Z80CTC;

namespace BUBCOM80 {
	class BUBBLECASETTE;
	class CMT;
	class DISPLAY;
	class FLOPPY;
	class KEYBOARD;
	class MEMBUS;
	class RTC;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;

	// devices
	//EVENT* event;
	//csp_state_utils *state_entry;

	IO* io;
	LS393* flipflop;
	MB8877* fdc;
//	MC6850* sio_rs;
	MC6850* sio_cmt;
//	MC6850* sio_key;
	PCM1BIT* pcm;
	Z80* cpu;
	Z80CTC* ctc;

	BUBCOM80::BUBBLECASETTE* bubblecasette[2];
	BUBCOM80::CMT* cmt;
	DEVICE* printer;
	BUBCOM80::DISPLAY* display;
	BUBCOM80::FLOPPY* floppy;
	BUBCOM80::KEYBOARD* keyboard;
	BUBCOM80::MEMBUS* membus;
	BUBCOM80::RTC* rtc;

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
	void reset() override;
	void run() override;
	double get_frame_rate() override;

#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index) override;
#endif

	// draw screen
	void draw_screen() override;

	// sound generation
	void initialize_sound(int rate, int samples) override;
	uint16_t* create_sound(int* extra_frames) override;
	int get_sound_buffer_ptr() override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r) override;
#endif

	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
	void play_tape(int drv, const _TCHAR* file_path) override;
	void rec_tape(int drv, const _TCHAR* file_path) override;
	void close_tape(int drv) override;
	bool is_tape_inserted(int drv) override;
	void open_bubble_casette(int drv, const _TCHAR *path, int bank) override;
	void close_bubble_casette(int drv) override;
	bool is_bubble_casette_inserted(int drv) override;
	bool is_bubble_casette_protected(int drv) override;
	void is_bubble_casette_protected(int drv, bool flag) override;
	bool is_frame_skippable() override;

	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	//DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
