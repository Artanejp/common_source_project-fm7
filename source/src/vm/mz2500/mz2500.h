/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ virtual machine ]
*/

#ifndef _MZ2500_H_
#define _MZ2500_H_

#define DEVICE_NAME		"SHARP MZ-2500"
#define CONFIG_NAME		"mz2500"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		6000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define HAS_MB8876
#define HAS_RP5C15
#define DATAREC_SOUND
#define PCM1BIT_HIGH_QUALITY

// memory wait
#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_FD1
#define USE_FD2
#define USE_FD3
#define USE_FD4
#define USE_TAPE
#define USE_SOCKET
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_MONITOR_TYPE	4
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8253;
class I8255;
class IO;
class MB8877;
class PCM1BIT;
class RP5C01;
class W3100A;
class YM2203;
class Z80;
class Z80PIO;
class Z80SIO;

class CALENDAR;
class CMT;
class CRTC;
class FLOPPY;
class INTERRUPT;
class JOYSTICK;
class KEYBOARD;
class MEMORY;
class MOUSE;
class MZ1E26;
class MZ1E30;
class MZ1R13;
class MZ1R37;
class TIMER;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8253* pit;
	I8255* pio_i;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	RP5C01* rtc;
	W3100A* w3100a;
	YM2203* opn;
	Z80* cpu;
	Z80PIO* pio;
	Z80SIO* sio;
	
	CALENDAR* calendar;
	CMT* cmt;
	CRTC* crtc;
	FLOPPY* floppy;
	INTERRUPT* interrupt;
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MOUSE* mouse;
	MZ1E26* mz1e26;
	MZ1E30* mz1e30;
	MZ1R13* mz1r13;
	MZ1R37* mz1r37;
	TIMER* timer;
	
	// monitor type cache
	int monitor_type;
	
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
	int access_lamp();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// socket
	void network_connected(int ch);
	void network_disconnected(int ch);
	uint8* get_sendbuffer(int ch, int* size);
	void inc_sendbuffer_ptr(int ch, int size);
	uint8* get_recvbuffer0(int ch, int* size0, int* size1);
	uint8* get_recvbuffer1(int ch);
	void inc_recvbuffer_ptr(int ch, int size);
	
	// user interface
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
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
