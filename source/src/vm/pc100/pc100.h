/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.12 -

	[ virtual machine ]
*/

#ifndef _PC100_H_
#define _PC100_H_

#define DEVICE_NAME		"NEC PC-100"
#define CONFIG_NAME		"pc100"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	544
#define CPU_CLOCKS		6988800
#define SCREEN_WIDTH		720
#define SCREEN_HEIGHT		512
#define WINDOW_HEIGHT_ASPECT	540
//720
#define MAX_DRIVE		4
#define I8259_MAX_CHIPS		1
#define MSM58321_START_DAY	-9
#define MSM58321_START_YEAR	1980
#define UPD765A_NO_ST0_AT_FOR_SEEK
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x8000
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_DRIVE_TYPE		2
#define USE_FLOPPY_DISK		2
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	2
#define USE_SCREEN_FILTER
#define USE_SOUND_VOLUME	3
#define USE_MOUSE
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep #1"), _T("Beep #2"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class AND;
class BEEP;
class I8251;
class I8255;
class I8259;
class I86;
class IO;
class MEMORY;
class MSM58321;
class PCM1BIT;
class UPD765A;

class CRTC;
class IOCTRL;
class KANJI;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and_drq;
	BEEP* beep;
	I8251* sio;
	I8255* pio0;
	I8255* pio1;
	I8259* pic;	// includes 2chips
	I86* cpu;
	IO* io;
	MEMORY* memory;
	MSM58321* rtc;
	PCM1BIT* pcm;
	UPD765A* fdc;
	
	CRTC* crtc;
	IOCTRL* ioctrl;
	KANJI* kanji;
	
	// memory
	uint8_t ram[0xc0000];	// Main RAM 768KB
	uint8_t ipl[0x8000];	// IPL 32KB
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
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
