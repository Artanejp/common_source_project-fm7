/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ virtual machine ]
*/

#ifndef _FAMILYBASIC_H_
#define _FAMILYBASIC_H_

#define DEVICE_NAME		"Nintendo Family BASIC"
#define CONFIG_NAME		"familybasic"

// device informations for virtual machine
#define FRAMES_PER_SEC		59.98
//#define LINES_PER_FRAME	262
#define LINES_PER_FRAME		525 // 262.5*2
#define CPU_CLOCKS		1789772
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		240
// pixel aspect should be 8:7
#define WINDOW_HEIGHT_ASPECT	210
#define HAS_N2A03

// device informations for win32
#define USE_BOOT_MODE		6
#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_SOUND_VOLUME	4
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("APU"), _T("VRC7"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

#ifdef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Button #1"),
	_T("Button #2"),
	_T("Select"),
	_T("Start"),
};
#endif

typedef struct header_s {
	uint8_t id[3];	// 'NES'
	uint8_t ctrl_z;	// control-z
	uint8_t dummy;
	uint8_t num_8k_vrom_banks;
	uint8_t flags_1;
	uint8_t flags_2;
	uint8_t reserved[8];
	uint32_t num_16k_rom_banks()
	{
		return (dummy != 0) ? dummy : 256;
	}
	uint32_t num_8k_rom_banks()
	{
		return num_16k_rom_banks() * 2;
	}
	uint8_t mapper()
	{
		return (flags_1 >> 4) | (flags_2 & 0xf0);
	}
} header_t;

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class M6502;
class YM2413;

class MEMORY;
class APU;
class PPU;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	M6502* cpu;
	YM2413* opll;
	
	MEMORY* memory;
	APU* apu;
	PPU* ppu;
	
	int boot_mode;
	
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
	
	// user interface
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
	void push_play(int drv);
	void push_stop(int drv);
	void push_fast_forward(int drv);
	void push_fast_rewind(int drv);
	void push_apss_forward(int drv) {}
	void push_apss_rewind(int drv) {}
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
