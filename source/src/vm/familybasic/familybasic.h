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
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		1789772
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		240
// pixel aspect should be 8:7
#define WINDOW_HEIGHT_ASPECT	210
#define HAS_N2A03

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_BOOT_MODE		3
#define USE_TAPE
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#define USE_SOUND_VOLUME	3
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("APU"), _T("CMT (Signal)"), _T("Noise (CMT)"),
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

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class M6502;

class MEMORY;
class APU;
class PPU;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	M6502* cpu;
	
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
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
	bool is_frame_skippable();
	
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
