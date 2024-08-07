/*
	ASCII MSX1 Emulator 'yaMSX1'
	ASCII MSX2 Emulator 'yaMSX2'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya
	modified by umaiboux

	[ virtual machine ]
*/

#ifndef _MSX_H_
#define _MSX_H_

#if defined(_PX7)
#define DEVICE_NAME		"PIONEER PX-7"
#define CONFIG_NAME		"px7"
#elif defined(_MSX1)
#define DEVICE_NAME		"ASCII MSX1"
#define CONFIG_NAME		"msx1"
#elif defined(_MSX2)
#define DEVICE_NAME		"ASCII MSX2"
#define CONFIG_NAME		"msx2"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3579545
#if defined(_MSX2)
#define SCREEN_WIDTH		((256 + 15)*2)	// V99X8_WIDTH
#define SCREEN_HEIGHT		((212 + 15)*2)	// V99X8_HEIGHT
#else
#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		384
#endif
#define TMS9918A_VRAM_SIZE	0x4000
#define TMS9918A_LIMIT_SPRITES
#if defined(_PX7)
#define TMS9918A_SUPER_IMPOSE
#else
#define MAX_DRIVE		2
#define SUPPORT_MEDIA_TYPE_1DD
#define Z80_PSEUDO_BIOS
#endif
#define HAS_AY_3_8910
// for Flappy Limited '85
#define AY_3_891X_PORT_MODE	0x80

// device informations for win32
#define USE_CART		2
#define USE_TAPE		1
#if defined(_PX7)
#define USE_LASER_DISC		1
#define USE_MOVIE_PLAYER
#else
#define USE_FLOPPY_DISK		2
#endif
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#if defined(_PX7)
#define USE_SOUND_VOLUME	5
#else
//#define USE_SOUND_VOLUME	5
#define USE_SOUND_VOLUME	4
#endif
#define SUPPORT_TV_RENDER
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Beep"), _T("CMT (Signal)"),
#if defined(_PX7)
	_T("LD-700"),
#endif
	 _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8255;
class IO;
#if defined(_PX7)
class LD700;
#endif
class NOT;
//class YM2203;
class AY_3_891X;
class PCM1BIT;
#if defined(_MSX2)
class RP5C01;
class V99X8;
#else
class TMS9918A;
#endif
class Z80;

namespace MSX {
	class JOYSTICK;
	class KEYBOARD;
	class MEMORY;
#if defined(_MSX2)
	class RTCIF;
#endif
	class SLOT0;
	class SLOT1;
	class SLOT2;
	class SLOT3;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;

	// devices
	//EVENT* event;

	DATAREC* drec;
	I8255* pio;
	IO* io;
#if defined(_PX7)
	LD700* ldp;
#endif
	NOT* not_remote;
//	YM2203* psg;
	AY_3_891X* psg;
	PCM1BIT* pcm;
#if defined(_MSX2)
	RP5C01* rtc;
	V99X8* vdp;
#else
	TMS9918A* vdp;
#endif
	Z80* cpu;

	MSX::JOYSTICK* joystick;
	MSX::KEYBOARD* keyboard;
	MSX::MEMORY* memory;
#ifdef _MSX2
	MSX::RTCIF* rtcif;
#endif
	MSX::SLOT0 *slot0;
	MSX::SLOT1 *slot1;
	MSX::SLOT2 *slot2;
	MSX::SLOT3 *slot3;

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
#if defined(_PX7)
	void movie_sound_callback(uint8_t *buffer, long size)  override;
#endif
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
	void push_apss_forward(int drv) override {}
	void push_apss_rewind(int drv) override {}
#if defined(_PX7)
	void open_laser_disc(int drv, const _TCHAR* file_path)  override;
	void close_laser_disc(int drv)  override;
	bool is_laser_disc_inserted(int drv)  override;
	uint32_t is_laser_disc_accessed()  override;
#else
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank)  override;
	void close_floppy_disk(int drv)  override;
	bool is_floppy_disk_inserted(int drv)  override;
	void is_floppy_disk_protected(int drv, bool value)  override;
	bool is_floppy_disk_protected(int drv)  override;
	uint32_t is_floppy_disk_accessed()  override;
#endif
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
