/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ virtual machine ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#ifndef _Z80TVGAME_H_
#define _Z80TVGAME_H_

#ifdef _USE_I8255
#define DEVICE_NAME		"Homebrew Z80 TV GAME SYSTEM (i8255)"
#define CONFIG_NAME		"z80tvgame_i8255"
#else
#define DEVICE_NAME		"Homebrew Z80 TV GAME SYSTEM (Z80PIO)"
#define CONFIG_NAME		"z80tvgame_z80pio"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		176
#define SCREEN_HEIGHT		210
#define WINDOW_WIDTH_ASPECT	280

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_CART			1
#define USE_SOUND_VOLUME	1
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

#ifdef _USE_I8255
class I8255;
#else
class Z80PIO;
#endif
class PCM1BIT;
class Z80;

namespace Z80TVGAME {
	class JOYSTICK;
	class MEMORY;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	//EVENT* event;
	
#ifdef _USE_I8255
	I8255* pio;
#else
	Z80PIO* pio;
#endif
	PCM1BIT* pcm;
	Z80* cpu;
	
	Z80TVGAME::JOYSTICK* joystick;
	Z80TVGAME::MEMORY* memory;
	
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
	void open_cart(int drv, const _TCHAR* file_path) override;
	void close_cart(int drv) override;
	bool is_cart_inserted(int drv) override;
	bool is_frame_skippable() override;
	
	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;
	
	void update_config() override;
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
