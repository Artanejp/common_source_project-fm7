/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ virtual machine ]
*/

#ifndef _X1_H_
#define _X1_H_

#if defined(_X1TURBOZ)
#define DEVICE_NAME		"SHARP X1turboZ"
#define CONFIG_NAME		"x1turboz"
#elif defined(_X1TURBO)
#define DEVICE_NAME		"SHARP X1turbo"
#define CONFIG_NAME		"x1turbo"
#elif defined(_X1TWIN)
#define DEVICE_NAME		"SHARP X1twin"
#define CONFIG_NAME		"x1twin"
#else
#define DEVICE_NAME		"SHARP X1"
#define CONFIG_NAME		"x1"
#endif

#if defined(_X1TURBO) || defined(_X1TURBOZ)
#define _X1TURBO_FEATURE
#endif

// device informations for virtual machine (x1)
//#ifdef _X1TURBO_FEATURE
//24KHz
//#define FRAMES_PER_SEC	55.49
//#define LINES_PER_FRAME	448
//#define CHARS_PER_LINE	56
//#define HD46505_HORIZ_FREQ	24860
//#else
// 15KHz
#define FRAMES_PER_SEC		61.94
#define LINES_PER_FRAME 	258
#define CHARS_PER_LINE		56
#define HD46505_HORIZ_FREQ	15980
//#endif
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define IO_ADDR_MAX		0x10000
#define HAS_AY_3_8910
#define Z80_IO_WAIT
#ifdef _X1TURBO_FEATURE
#define SINGLE_MODE_DMA
#endif
#define DATAREC_FF_REW_SPEED	16
#define SUPPORT_VARIABLE_TIMING

#ifdef _X1TURBO_FEATURE
#define IPL_ROM_FILE_SIZE	0x8000
#define IPL_ROM_FILE_NAME	_T("IPLROM.X1T")
#define SUB_ROM_FILE_NAME	_T("SUBROM.X1T")
#define KBD_ROM_FILE_NAME	_T("KBDROM.X1T")
#else
#define IPL_ROM_FILE_SIZE	0x1000
#define IPL_ROM_FILE_NAME	_T("IPLROM.X1")
#define SUB_ROM_FILE_NAME	_T("SUBROM.X1")
#define KBD_ROM_FILE_NAME	_T("KBDROM.X1")
#endif
#define CRC32_MSM80C49_262	0x43EE7D6F	// X1turbo with CMT
#define CRC32_MSM80C49_277	0x75904EFB	// X1turbo (not supported yet)

#ifdef _X1TWIN
// device informations for virtual machine (pce)
#define PCE_FRAMES_PER_SEC	60
#define PCE_LINES_PER_FRAME	262
#define PCE_CPU_CLOCKS		7159090
#endif

// device informations for win32
#define USE_SPECIAL_RESET
#ifdef _X1TURBO_FEATURE
#define USE_DEVICE_TYPE		2
#endif
#define USE_FD1
#define USE_FD2
#define FD_BASE_NUMBER		0
#define USE_TAPE
#ifdef _X1TWIN
#define USE_CART1
#endif
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_MONITOR_TYPE	2
#define USE_CRT_FILTER
#define USE_SCANLINE
#define USE_SOUND_DEVICE_TYPE	3
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class MB8877;
class YM2151;
class YM2203;
class Z80;
class Z80CTC;
class Z80SIO;
#ifdef _X1TURBO_FEATURE
class Z80DMA;
#endif

class DISPLAY;
class EMM;
class FLOPPY;
class IO;
class JOYSTICK;
class MEMORY;
class MOUSE;
class PRINTER;
class PSUB;

class MCS48;
class UPD1990A;
class SUB;
class KEYBOARD;

#ifdef _X1TWIN
class HUC6280;
class PCE;
#endif

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices for x1
	EVENT* event;
	
	DATAREC* drec;
	HD46505* crtc;
	I8255* pio;
	MB8877* fdc;
	YM2151* opm1;
	YM2151* opm2;
	YM2203* psg;
	Z80* cpu;
	Z80CTC* ctc;
	Z80SIO* sio;
	Z80CTC* ctc1;
	Z80CTC* ctc2;
#ifdef _X1TURBO_FEATURE
	Z80DMA* dma;
#endif
	
	DISPLAY* display;
	EMM* emm;
	FLOPPY* floppy;
	IO* io;
	JOYSTICK* joy;
	MEMORY* memory;
	MOUSE* mouse;
	PRINTER* printer;
	PSUB* psub;
	
	MCS48* cpu_sub;
	UPD1990A* rtc_sub;
	I8255* pio_sub;
	SUB* sub;
	
	MCS48* cpu_kbd;
	KEYBOARD* kbd;
	
	bool pseudo_sub_cpu;
	int sound_device_type;
	
#ifdef _X1TWIN
	// device for pce
	EVENT* pceevent;
	
	HUC6280* pcecpu;
	PCE* pce;
#endif
	
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
	double frame_rate();
	
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
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool now_skip();
#ifdef _X1TWIN
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
#endif
	
	void update_config();
#ifdef _X1TURBO_FEATURE
	void update_dipswitch();
#endif
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
