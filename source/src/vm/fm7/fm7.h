/*
	FUJITSU FM7 Emulator 'eFM7'

	Author : K.Ohta
	Date   : 2015.01.01-

	[ virtual machine ]
*/

#ifndef _FM7_H_
#define _FM7_H_

#define USE_TAPE
#define USE_TAPE_PTR
#define USE_TAPE_BUTTON
#define USE_SCANLINE
#define USE_DIPSWITCH
#define USE_CPU_TYPE 2
#define USE_SPECIAL_RESET
#define SUPPORT_DUMMY_DEVICE_LED 3
#define USE_MINIMUM_RENDERING 1
#define USE_MOUSE
#define USE_JOYSTICK
#define USE_PRINTER
#define USE_PRINTER_TYPE 4

// 4:3
#define SCREEN_WIDTH_ASPECT 640 
#define SCREEN_HEIGHT_ASPECT 400
#define WINDOW_WIDTH_ASPECT 640 
#define WINDOW_HEIGHT_ASPECT 400
#define USE_CRT_MONITOR_4_3 1

#define NOTIFY_KEY_DOWN
//#define NOTIFY_KEY_UP
#define NOTIFY_KEY_DOWN_LR_SHIFT
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
#define USE_STATE
#define USE_DEBUGGER


#if defined(_FM8)
#define DEVICE_NAME		"FUJITSU FM-8"
#define CONFIG_NAME		"fm8"
#define CAPABLE_Z80

#elif defined(_FM7)
#define DEVICE_NAME		"FUJITSU FM-7"
#define CONFIG_NAME		"fm7"
#define CAPABLE_Z80

#elif defined(_FMNEW7)
#define DEVICE_NAME		"FUJITSU FM-NEW7"
#define CONFIG_NAME		"fmnew7"
#define CAPABLE_Z80

#elif defined(_FM77) || defined(_FM77L2)
# if defined(_FM77)
#define DEVICE_NAME		"FUJITSU FM-77"
#define CONFIG_NAME		"fm77"
# else
#define DEVICE_NAME		"FUJITSU FM-77L2"
#define CONFIG_NAME		"fm77l2"
# endif
//#define USE_DRIVE_TYPE
#define _FM77_VARIANTS
#define CAPABLE_Z80
#ifndef FM77_EXRAM_BANKS
#define FM77_EXRAM_BANKS	3
#endif

#elif defined(_FM77L4)
#define DEVICE_NAME		"FUJITSU FM-77L4"
#define CONFIG_NAME		"fm77l4"
#define HAS_MMR
#define HAS_TEXTVRAM
#define HAS_2HD
#define HAS_CYCLESTEAL
#define HAS_400LINECARD
//#define CAPABLE_KANJI_CLASS2
#define _FM77_VARIANTS
#define CAPABLE_Z80
#ifndef FM77_EXRAM_BANKS
#define FM77_EXRAM_BANKS	3
#endif

#elif defined(_FM77AV)
#define DEVICE_NAME		"FUJITSU FM77AV"
#define CONFIG_NAME		"fm77av"
#define _FM77AV_VARIANTS

#elif defined(_FM77AV20)
#define DEVICE_NAME		"FUJITSU FM77AV20"
#define CONFIG_NAME		"fm77av20"
#define _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_2DD_2D
#define CAPABLE_DICTROM
//#define USE_DRIVE_TYPE 2
#define CAPABLE_KANJI_CLASS2

#elif defined(_FM77AV20EX)
#define DEVICE_NAME		"FUJITSU FM77AV20EX"
#define CONFIG_NAME		"fm77av20ex"
#define _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_2DD_2D
#define HAS_DMA
//#define USE_DRIVE_TYPE 2
#define CAPABLE_DICTROM
#define CAPABLE_KANJI_CLASS2

#elif defined(_FM77AV40)
#define DEVICE_NAME		"FUJITSU FM77AV40"
#define CONFIG_NAME		"fm77av40"
#define _FM77AV_VARIANTS
#define HAS_2DD_2D
#define HAS_DMA
//#define USE_DRIVE_TYPE 2
#define CAPABLE_DICTROM
#define HAS_400LINE_AV
#define CAPABLE_KANJI_CLASS2
#ifndef FM77_EXRAM_BANKS
#define FM77_EXRAM_BANKS	12
#endif

#elif defined(_FM77AV40EX)
#define DEVICE_NAME		"FUJITSU FM77AV40EX"
#define CONFIG_NAME		"fm77av40ex"
#define _FM77AV_VARIANTS
#define HAS_2DD_2D
#define HAS_DMA
//#define USE_DRIVE_TYPE 2
#define CAPABLE_DICTROM
#define HAS_400LINE_AV
#define CAPABLE_KANJI_CLASS2
#ifndef FM77_EXRAM_BANKS
#define FM77_EXRAM_BANKS	12
#endif

#elif defined(_FM77AV40SX)
#define DEVICE_NAME		"FUJITSU FM77AV40SX"
#define CONFIG_NAME		"fm77av40sx"
#define _FM77AV_VARIANTS
#define HAS_2DD_2D
#define HAS_DMA
//#define USE_DRIVE_TYPE 2
#define CAPABLE_DICTROM
#define HAS_400LINE_AV
#define CAPABLE_KANJI_CLASS2
#ifndef FM77_EXRAM_BANKS
#define FM77_EXRAM_BANKS	12
#endif

#endif

#if !defined(_FM8)
#define USE_DEVICE_TYPE		3
#define USE_SOUND_DEVICE_TYPE   8
# ifdef _FM77AV_VARIANTS
#  define USE_MULTIPLE_SOUNDCARDS 4
# else // 7,77
#  define USE_MULTIPLE_SOUNDCARDS 5
# endif
#elif defined(_FM8)
#  define USE_MULTIPLE_SOUNDCARDS 2
#endif

#ifdef _FM77AV_VARIANTS

//#define CAPABLE_KANJI_CLASS2
#define HAS_MMR
#define HAS_CYCLESTEAL

#elif defined(_FM77_VARIANTS)

#define HAS_MMR
#define HAS_CYCLESTEAL
#define CAPABLE_Z80
#endif

#if defined(_FM77_VARIANTS)
#define USE_BOOT_MODE         4
#elif defined(_FM8)
#define USE_BOOT_MODE         4
#elif defined(_FM7) || defined(_FMNEW7)
#define USE_BOOT_MODE         3
#else
#define USE_BOOT_MODE         2
#endif

// 0 = PSG or NONE
// 1 = OPN (+PSG)
// 2 = WHG (+PSG)
// 3 = WHG + OPN (+PSG)
// 4 = THG  (+PSG)
// 5 = THG + OPN (+PSG)
// 6 = THG + WHG (+PSG)
// 7 = THG + WHG + OPN (+PSG)
#if defined(_FM8)
// WITHOUT PSG?
#define SOUND_DEVICE_TYPE_DEFAULT	0
#elif defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
// PSG ONLY
#define SOUND_DEVICE_TYPE_DEFAULT	0
#elif defined(_FM77AV_VARIANTS)
// OPN
#define SOUND_DEVICE_TYPE_DEFAULT	1
#endif
#define IGNORE_DISK_CRC_DEFAULT		true
// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		59.94
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77L4)
#define LINES_PER_FRAME 	400
#else
#define LINES_PER_FRAME 	200
#endif

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
#define CPU_CLOCKS		2016000
#elif defined(_FM8)
#define CPU_CLOCKS		1095000
#else
#define CPU_CLOCKS		2000000
#endif

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		4
#define HAS_MC6809              
#define MB8877_MAX_CHIPS	1
//#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define MAX_FD 2

#if defined(HAS_2HD)
#define USE_FD3
#define USE_FD4
#undef  MAX_FD
#define MAX_FD 4
#endif


#ifdef BUILD_Z80
# ifdef CAPABLE_Z80
#  define WITH_Z80
# endif
#endif

// DIP Switch description
#define SWITCH_CYCLESTEAL 0x00000001
#if defined(_FM8)
#define SWITCH_URA_RAM    0x00000002
#else
#define SWITCH_URA_RAM    0x00000000
#endif
#define SWITCH_INVERT_CMT 0x00000004

// Belows are optional Hardwares.
#if !defined(_FM77_VARIANTS) && !defined(_FM77AV_VARIANTS)
#define SWITCH_FDC 0x00010000
#else
#define SWITCH_FDC 0x00000000
#endif
#if !defined(_FM77_VARIANTS) && !defined(_FM77AV_VARIANTS)
#define SWITCH_Z80 0x00020000
#else
#define SWITCH_Z80 0x00000000
#endif
#if defined(_FM77_VARIANTS)
#define SWITCH_DICTCARD 0x00040000
#else
#define SWITCH_DICTCARD 0x00000000
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
#define SWITCH_EXTRA_RAM 0x00080000
#else
#define SWITCH_EXTRA_RAM 0x00000000
#endif

//#define ENABLE_OPENCL // If OpenCL renderer is enabled, define here.

//#include "../../config.h"
#include "../../common.h"
#include "../../fileio.h"

class EMU;
class DEVICE;
class EVENT;
class FILEIO;

#if defined(_FM77AV_VARIANTS)
class BEEP;
#endif
class PCM1BIT;
class MC6809;
class YM2203;
class MB8877;
class MEMORY;
class DATAREC;
#if defined(SUPPORT_DUMMY_DEVICE_LED)
class DUMMYDEVICE;
#endif

class DISPLAY;
#if defined(_FM77AV_VARIANTS)
class MB61VH010;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
class HD6844;
#endif
class FM7_MAINMEM;
class FM7_MAINIO;
class KEYBOARD;
class KANJIROM;
class JOYSTICK;

#if WITH_Z80
class Z80;
#endif

class VM {
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	MC6809* maincpu;
	FM7_MAINMEM* mainmem;
	FM7_MAINIO* mainio;
#if defined(SUPPORT_DUMMY_DEVICE_LED)
	DUMMYDEVICE* led_terminate;
#else
	DEVICE* led_terminate;
#endif
	MB8877* fdc;
	YM2203* opn[3];
#if !defined(_FM77AV_VARIANTS)
	YM2203* psg; // Is right? AY-3-8910 is right device.
#endif   
	//BEEP* beep;
	PCM1BIT* pcm1bit;
	DATAREC *drec;
	JOYSTICK *joystick;
	
#ifdef  WITH_Z80
	Z80* z80cpu;
#endif
	DEVICE* printer;
	DEVICE* inteli_mouse; 
   
	DEVICE *dummycpu;
	MC6809* subcpu;
#if defined(_FM77AV_VARIANTS)
	MB61VH010 *alu;
	BEEP *keyboard_beep;
#endif
#if defined(HAS_DMA)
	HD6844 *dmac;
#endif   
	DISPLAY* display;
	KEYBOARD* keyboard;
   
	KANJIROM *kanjiclass1;
#ifdef CAPABLE_KANJI_CLASS2
	KANJIROM *kanjiclass2;
#endif
	uint32 connected_opns;
	bool clock_low;
	bool connect_opn;
	bool connect_whg;
	bool connect_thg;
   
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
#if defined(SUPPORT_DUMMY_DEVICE_LED)
	uint32 get_led_status();
#endif
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	void initialize(void);
	void connect_bus(void);

	void update_dipswitch(void);
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
	void open_disk(int drv, const _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
	
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool tape_playing();
	bool tape_recording();
	int tape_position();
	
	bool now_skip();
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward();
	void push_apss_rewind();
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);

#if defined(USE_DIG_RESOLUTION)
	void get_screen_resolution(int *w, int *h);
#endif
#if defined(USE_MINIMUM_RENDERING)	
	bool screen_changed(void);
#endif	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	void set_cpu_clock(DEVICE *cpu, uint32 clocks);
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
