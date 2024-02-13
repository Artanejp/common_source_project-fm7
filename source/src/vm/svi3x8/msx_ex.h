/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/msx.h

	modified by tanam
	Date   : 2018.12.09-

	[ virtual machine ]
*/

#ifndef _MSX_EX_SVI3X8_H_
#define _MSX_EX_SVI3X8_H_

#if defined(_SVI3X8)
#define DEVICE_NAME		"SPECTRAVIDEO SVI-3x8"
#define CONFIG_NAME		"svi3x8"
#endif

#if defined(_SVI3X8)
#define _MSX1_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define FDD_PATCH_SLOT	0x8B

#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3579545
#if defined(_MSX1_VARIANTS)
#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		384
#define WINDOW_WIDTH_ASPECT	576
#endif
#define TMS9918A_VRAM_SIZE	0x4000
#define TMS9918A_LIMIT_SPRITES
//#if defined(FDD_PATCH_SLOT)
#define MAX_DRIVE		2
//#define SUPPORT_MEDIA_TYPE_1DD
//#define Z80_PSEUDO_BIOS
//#endif
#define HAS_AY_3_8910
// for Flappy Limited '85
#define AY_3_891X_PORT_MODE	0x80

// device informations for win32
#define USE_CART		2
#define USE_TAPE		1
//#if defined(FDD_PATCH_SLOT)
#define USE_FLOPPY_DISK		2
//#endif
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_SOUND_VOLUME	7
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
//#define USE_PRINTER
//#define USE_PRINTER_TYPE	4

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Beep"), _T("CMT (Signal)"),
	_T("Cart#1"), _T("Cart#2"), _T("MSX-MUSIC"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

//class DATAREC;
class I8255;
class IO;
class NOT;
class AY_3_891X;
class PCM1BIT;
class TMS9918A;
class Z80;

namespace SVI_3X8 {
	class JOYSTICK;
	class KEYBOARD;
	class MEMORY_EX;
}
namespace MSX {
	class SLOT_MAINROM;
	class SLOT_CART;
#if defined(USE_PRINTER)
	class PRINTER;
#endif
}
//#if defined(FDD_PATCH_SLOT)
//class SLOT_FDD_PATCH;
//#endif

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;

	// devices
	EVENT* event;

//	DATAREC* drec;
	I8255* pio;
	IO* io;
	NOT* not_remote;
	AY_3_891X* psg;
	PCM1BIT* pcm;
	TMS9918A* vdp;
	Z80* cpu;

	SVI_3X8::JOYSTICK* joystick;
	SVI_3X8::KEYBOARD* keyboard;
	SVI_3X8::MEMORY_EX* memory;
	MSX::SLOT_MAINROM *slot_mainrom;
	MSX::SLOT_CART *slot_cart[1];
//#ifdef USE_PRINTER
//	MSX::PRINTER* printer;
//#endif

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
	void reset()  override;
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

	// user interface
	void open_cart(int drv, const _TCHAR* file_path)  override;
	void close_cart(int drv)  override;
	bool is_cart_inserted(int drv)  override;
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
	void push_apss_forward(int drv)  override {}
	void push_apss_rewind(int drv)  override {}

#if defined(FDD_PATCH_SLOT)
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank)  override;
	void close_floppy_disk(int drv)  override;
	bool is_floppy_disk_inserted(int drv)  override;
	void is_floppy_disk_protected(int drv, bool value)  override;
	bool is_floppy_disk_protected(int drv)  override;
	uint32_t is_floppy_disk_accessed()  override;
#endif

	bool is_frame_skippable()  override;

	void update_config()  override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	DEVICE* get_device(int id) override;
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
