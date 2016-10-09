/*
	EPSON HC-80 Emulator 'eHC-80'

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ virtual machine ]
*/

#ifndef _HC80_H_
#define _HC80_H_

#define DEVICE_NAME		"EPSON HC-80"
#define CONFIG_NAME		"hc80"

// device informations for virtual machine
#define FRAMES_PER_SEC		64
#define LINES_PER_FRAME		64
#define CPU_CLOCKS		2457600
#define SCREEN_WIDTH		480
#define SCREEN_HEIGHT		64
#define MAX_DRIVE		4

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_DEVICE_TYPE		3
// Nonintelligent ram disk
#define DEVICE_TYPE_DEFAULT	2
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_ACCESS_LAMP
#define USE_SOUND_FILES		2
//#define USE_SOUND_FILES_FDD
//#if defined(USE_SOUND_FILES)
//#define USE_SOUND_VOLUME	2
//#else
#define USE_SOUND_VOLUME	1
//#endif
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"),
#if defined(USE_SOUND_FILES)
	//_T("FDD SEEK"),
#endif
};
#endif

class EMU;
class DEVICE;
class EVENT;

class BEEP;
class I8251;
class PTF20;
class Z80;

class IO;
class MEMORY;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	BEEP* beep;
	I8251* sio;
	PTF20* tf20;
	Z80* cpu;
	
	IO* io;
	MEMORY* memory;
	
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
	void special_reset();
	void run();
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	uint32_t get_access_lamp_status();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
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
