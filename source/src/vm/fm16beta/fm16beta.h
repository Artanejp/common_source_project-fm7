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
#define SINGLE_MODE_DMA
//#define MB8877_NO_BUSY_AFTER_SEEK

// device informations for win32
#define USE_FLOPPY_DISK		4
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE
#if defined(HAS_I286)
#define USE_CPU_I286
#else
#define USE_CPU_I186
#endif
#define USE_CPU_MC6809

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

namespace FM16BETA {
	class CMOS;
	class KEYBOARD;
	class MAINBUS;
	class SUB;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils *state_entry;

	// devices
	//EVENT* event;

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

	FM16BETA::CMOS* cmos;
	FM16BETA::MAINBUS* mainbus;
	FM16BETA::KEYBOARD* keyboard;
	FM16BETA::SUB* subbus;

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
	void notify_power_off() override
	{
	}

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

	// notify key
	void key_down(int code, bool repeat) override;
	void key_up(int code) override;

	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
	bool is_frame_skippable() override;

	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

	//void update_config();
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	//DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
