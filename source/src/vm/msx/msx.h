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
#define YM2203_PORT_MODE	0x80

#define SCREEN_WIDTH_ASPECT SCREEN_WIDTH 
#define SCREEN_HEIGHT_ASPECT SCREEN_HEIGHT
#define WINDOW_WIDTH_ASPECT SCREEN_WIDTH
#define WINDOW_HEIGHT_ASPECT SCREEN_HEIGHT 

// device informations for win32
#define USE_CART1
#define USE_CART2
#define USE_TAPE
#define USE_TAPE_PTR
#if defined(_PX7)
#define USE_LASER_DISC
#else
#define USE_FD1
#define USE_FD2
#endif
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

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
class YM2203;
class PCM1BIT;
#if defined(_MSX2)
class RP5C01;
class V99X8;
#else
class TMS9918A;
#endif
class Z80;

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

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8255* pio;
	IO* io;
#if defined(_PX7)
	LD700* ldp;
#endif
	NOT* d_not;
	YM2203* psg;
	PCM1BIT* pcm;
#if defined(_MSX2)
	RP5C01* rtc;
	V99X8* vdp;
#else
	TMS9918A* vdp;
#endif
	Z80* cpu;
	
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
#ifdef _MSX2
	RTCIF* rtcif;
#endif
	SLOT0 *slot0;
	SLOT1 *slot1;
	SLOT2 *slot2;
	SLOT3 *slot3;
	
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
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
#if defined(_PX7)
	void movie_sound_callback(uint8 *buffer, long size);
#endif
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool tape_playing();
	bool tape_recording();
	int tape_position();
#if defined(_PX7)
	void open_laser_disc(const _TCHAR* file_path);
	void close_laser_disc();
	bool laser_disc_inserted();
#else
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
	//int access_lamp();
#endif
	bool now_skip();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
