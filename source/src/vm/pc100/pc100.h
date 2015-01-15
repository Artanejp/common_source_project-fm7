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
//720
#define MAX_DRIVE		4
#define HAS_I86
#define I8259_MAX_CHIPS		1
#define MSM58321_START_DAY	-9
#define MSM58321_START_YEAR	1980
#define UPD765A_NO_ST0_AT_FOR_SEEK
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x8000
#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_MONITOR_TYPE	2
#define USE_SCREEN_ROTATE
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class AND;
class BEEP;
class I8251;
class I8255;
class I8259;
class I286;
class IO;
class MEMORY;
class MSM58321;
class PCM1BIT;
class UPD765A;

class CRTC;
class IOCTRL;
class KANJI;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and;
	BEEP* beep;
	I8251* sio;
	I8255* pio0;
	I8255* pio1;
	I8259* pic;	// includes 2chips
	I286* cpu;
	IO* io;
	MEMORY* memory;
	MSM58321* rtc;
	PCM1BIT* pcm;
	UPD765A* fdc;
	
	CRTC* crtc;
	IOCTRL* ioctrl;
	KANJI* kanji;
	
	// memory
	uint8 ram[0xc0000];	// Main RAM 768KB
	uint8 ipl[0x8000];	// IPL 32KB
	
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
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
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
