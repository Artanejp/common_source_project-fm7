/*
	CASIO FX-9000P Emulator 'eFX-9000P'

	Author : Takeda.Toshiya
	Date   : 2022.03.25-

	[ virtual machine ]
*/

#ifndef _FX9000P_H_
#define _FX9000P_H_

#define DEVICE_NAME		"CASIO FX-9000P"
#define CONFIG_NAME		"fx9000p"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		261
#define CHARS_PER_LINE		44
//#define HD46505_HORIZ_FREQ	15734
#define CPU_CLOCKS		2750000
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		128

// device informations for win32
#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	8
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
	_T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class MEMORY;
class Z80;

namespace FX9000P {
	class IO;
}

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;

	// devices
	EVENT* event;

	DATAREC* drec;
	HD46505* crtc;
	MEMORY* memory;
	Z80* cpu;

	FX9000P::IO* io;

	// memory
	uint8_t basic[0x3000];	// 0000h-2FFFh: BASIC ROM
	uint8_t op1[0x1000];	// 5000h-5FFFh: OP-1 ROM
	uint8_t vram[0x1000];	// 7000h-7FFFh: VRAM
	uint8_t dram[0x4000];	// 8000h-BFFFh: DRAM 16KB (Slot #1)
	uint8_t cmos[0x3000];	// C000h-EFFFh: CMOS 12KB (Slot #2-#4)

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
	//void reset() override;
	double get_frame_rate() override
	{
		return FRAMES_PER_SEC;
	}

#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index)  override;
#endif

	// draw screen
	void draw_screen()  override;

	// sound generation
	void initialize_sound(int rate, int samples)  override;
	uint16_t* create_sound(int* extra_frames)  override;
	int get_sound_buffer_ptr()  override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r)  override;
#endif

	// notify key
	void key_down(int code, bool repeat)  override;
	void key_up(int code)  override;

	// user interface
	void play_tape(int drv, const _TCHAR* file_path)  override;
	void rec_tape(int drv, const _TCHAR* file_path)  override;
	void close_tape(int drv)  override;
	bool is_tape_inserted(int drv)  override;
	bool is_tape_playing(int drv)  override;
	bool is_tape_recording(int drv)  override;
	int get_tape_position(int drv)  override;
	const _TCHAR* get_tape_message(int drv)  override;
	void push_play(int drv)  override;
	void push_stop(int drv)  override;
	void push_fast_forward(int drv)  override;
	void push_fast_rewind(int drv)  override;
	void push_apss_forward(int drv) override {}
	void push_apss_rewind(int drv) override {}
	bool is_frame_skippable()  override;

	//void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
//	DEVICE* get_device(int id) override;
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
