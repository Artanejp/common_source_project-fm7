/*
	FUJITSU FM7 Emulator 'eFM7'

	Author : K.Ohta
	Date   : 2015.01.01-

	[ virtual machine ]
*/

#ifndef _FM7_H_
#define _FM7_H_

#define USE_TAPE
#define USE_SOUND_DEVICE_TYPE 8
#define USE_SCANLINE
#define USE_DIPSWITCH
#define USE_BOOT_MODE         4
#define USE_CPU_TYPE
#define USE_SPECIAL_RESET
//#define HAS_AY_3_8910
// 4:3
#define SCREEN_WIDTH_ASPECT 400 
#define SCREEN_HEIGHT_ASPECT 300

#define NOTIFY_KEY_DOWN
#define NOTIFY_KEY_UP
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_ACCESS_LAMP
//#define USE_DEBUGGER

#if defined(_FM8)
#define DEVICE_NAME		"FUJITSU FM8"
#define CONFIG_NAME		"fm8"
#define CAPABLE_Z80

#elif defined(_FM7)
#define DEVICE_NAME		"FUJITSU FM7"
#define CONFIG_NAME		"fm7"
#define CAPABLE_Z80

#elif defined(_FM77L4)
#define DEVICE_NAME		"FUJITSU FM77L4"
#define CONFIG_NAME		"fm77l4"
#define HAS_MMR
#define HAS_400LINECARD
#define HAS_TEXTVRAM
#define HAS_2HD
#define HAS_CYCLESTEAL
#define CAPABLE_Z80
#define CAPABLE_KANJI_CLASS2
#define USE_DRIVE_TYPE

#elif defined(_FM77AV)
#define DEVICE_NAME		"FUJITSU FM77AV"
#define CONFIG_NAME		"fm77av"
#define _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_CYCLESTEAL

#elif defined(_FM77AV20)
#define DEVICE_NAME		"FUJITSU FM77AV"
#define CONFIG_NAME		"fm77av20"
#define _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_2DD_2D
#define HAS_CYCLESTEAL
#define USE_DRIVE_TYPE

#elif defined(_FM77AV40)
#define DEVICE_NAME		"FUJITSU FM77AV"
#define CONFIG_NAME		"fm77av40"
#define _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_2DD_2D
#define HAS_CYCLESTEAL
#define CAPABLE_KANJI_CLASS2
#define USE_DRIVE_TYPE

#endif

// device informations for virtual machine

// TODO: check refresh rate
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	400
#define CPU_CLOCKS		2000000
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

//#define ENABLE_OPENCL // If OpenCL renderer is enabled, define here.

#include "../../config.h"
#include "../../common.h"
#include "../../fileio.h"

class EMU;
class DEVICE;
class EVENT;
class FILEIO;

class PCM1BIT;
class MC6809;
class YM2203;
class MB8877;
class MEMORY;
class DATAREC;

class DISPLAY;

class FM7_MAINMEM;
class FM7_MAINIO;
class KEYBOARD;
class KANJIROM;

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

	MB8877* fdc;
        YM2203* opn[3];
        YM2203* psg; // Is right? AY-3-8910 is right device.
        PCM1BIT* pcm1bit;
	DATAREC *drec;
   
#ifdef  WITH_Z80
        Z80* z80cpu;
#endif
        DEVICE* printer;
        DEVICE* mouse_opn;
	DEVICE* inteli_mouse; 
   
	DEVICE *dummycpu;
	MC6809* subcpu;
        MEMORY* submem;

        DISPLAY* display;
        KEYBOARD* keyboard;
   
	KANJIROM *kanjiclass1;
#ifdef CAPABLE_KANJI_CLASS2
	KANJIROM *kanjiclass2;
#endif
	int machine_version; // 0 = FM8 / 1 = FM7 / 2 = FM77AV / 3 = FM77AV40, etc...
        Uint32 bootmode;   
        Uint32 connected_opns;
        bool cycle_steal;
        bool clock_low;
        int mainfreq_type;
        Uint32 mainfreq_low;
        Uint32 mainfreq_high;
        Uint32 mainfreq_mmr;
        Uint32 mainfreq_high_mmr;
 
        Uint32 fdd_type[MAX_DRIVE];
        BOOL   fdd_connect[MAX_DRIVE];

        FILEIO* cmt_fileio;
        bool cmt_enabled; // 77AV40SX is disabled.
        bool cmt_play;
        bool cmt_rec;
        Uint32 cmt_bufptr;
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
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void write_protect_fd(int drv, bool flag);
	bool is_write_protect_fd(int drv);
	
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
	bool now_skip();
        int get_tape_ptr(void);
   
	void update_config();
	//void save_state(FILEIO* state_fio);
	//bool load_state(FILEIO* state_fio);

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
