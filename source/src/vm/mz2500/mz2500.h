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
#define FRAMES_PER_SEC		55.49
#define LINES_PER_FRAME 	448
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		6000000
#define CPU_CLOCKS_LOW		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define HAS_MB8876
#define HAS_RP5C15
#define DATAREC_SOUND
#define DATAREC_SOUND_RIGHT
// FIXME: need to adjust speed for BASIC-M25 Demonstration
//#define DATAREC_FAST_FWD_SPEED	10
//#define DATAREC_FAST_REW_SPEED	10
#define SCSI_HOST_AUTO_ACK

// memory wait
#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT
//#define Z80_NO_EVENT_IN_OPECODE

// device informations for win32
#define USE_SPECIAL_RESET
#define USE_BOOT_MODE		3
#define USE_FLOPPY_DISK		4
#define USE_TAPE		1
#define USE_HARD_DISK		2
#define USE_SOCKET
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	4
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_VOLUME	7
#define USE_JOYSTICK
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("OPN (FM)"), _T("OPN (PSG)"), _T("Beep"), _T("CMT (Signal)"), _T("CMT (Voice)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

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
class SASI_HDD;
class SCSI_HOST;
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
class PRINTER;
class SERIAL;
class TIMER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8253* pit;
	I8255* pio_i;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	RP5C01* rtc;
	SASI_HDD* sasi_hdd;
	SCSI_HOST* sasi_host;
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
	PRINTER* printer;
	SERIAL* serial;
	TIMER* timer;
	
	// monitor type cache
	int boot_mode;
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
	double get_frame_rate();
	
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
	
	// socket
	void notify_socket_connected(int ch);
	void notify_socket_disconnected(int ch);
	uint8_t* get_socket_send_buffer(int ch, int* size);
	void inc_socket_send_buffer_ptr(int ch, int size);
	uint8_t* get_socket_recv_buffer0(int ch, int* size0, int* size1);
	uint8_t* get_socket_recv_buffer1(int ch);
	void inc_socket_recv_buffer_ptr(int ch, int size);
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
	void open_hard_disk(int drv, const _TCHAR* file_path);
	void close_hard_disk(int drv);
	bool is_hard_disk_inserted(int drv);
	uint32_t is_hard_disk_accessed();
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
