/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ virtual machine ]
*/

#ifndef _MULTI8_H_
#define _MULTI8_H_

#define DEVICE_NAME		"MITSUBISHI Electric MULTI 8"
#define CONFIG_NAME		"multi8"

// device informations for virtual machine
#define FRAMES_PER_SEC		60.58
#define LINES_PER_FRAME 	260
#define CHARS_PER_LINE		112
#define HD46505_HORIZ_FREQ	15750
#define CPU_CLOCKS		3993600
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define UPD765A_DONT_WAIT_SEEK
#define HAS_AY_3_8912

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_TAPE			1
#define TAPE_BINARY_ONLY
#define USE_FLOPPY_DISK		2
#define USE_KEY_LOCKED
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SCANLINE

#define USE_SOUND_VOLUME	3
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Beep"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class HD46505;
class I8251;
class I8253;
class I8255;
class I8259;
class IO;
class UPD765A;
//class YM2203;
class AY_3_891X;
class Z80;

namespace MULTI8 {
	class CMT;
	class DISPLAY;
	class FLOPPY;
	class KANJI;
	class KEYBOARD;
	class MEMORY;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils *state_entry;

	// devices
	//EVENT* event;

	BEEP* beep;
	HD46505* crtc;
	I8251* sio;
	I8253* pit;
	I8255* pio;
	I8259* pic;
	IO* io;
	UPD765A* fdc;
//	YM2203* psg;
	AY_3_891X* psg;
	Z80* cpu;

	MULTI8::CMT* cmt;
	MULTI8::DISPLAY* display;
	MULTI8::FLOPPY* floppy;
	MULTI8::KANJI* kanji;
	MULTI8::KEYBOARD* key;
	MULTI8::MEMORY* memory;

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
	double get_frame_rate()  override;

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
	bool get_caps_locked()  override;
	bool get_kana_locked()  override;

	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank)  override;
	void close_floppy_disk(int drv)  override;
	bool is_floppy_disk_inserted(int drv)  override;
	void is_floppy_disk_protected(int drv, bool value)  override;
	bool is_floppy_disk_protected(int drv)  override;
	uint32_t is_floppy_disk_accessed()  override;
	void play_tape(int drv, const _TCHAR* file_path)  override;
	void rec_tape(int drv, const _TCHAR* file_path)  override;
	void close_tape(int drv)  override;
	bool is_tape_inserted(int drv)  override;
	bool is_frame_skippable()  override;

	double get_current_usec()  override;
	uint64_t get_current_clock_uint64()  override;

	void update_config()  override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
