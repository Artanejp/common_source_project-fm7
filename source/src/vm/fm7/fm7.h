/*
	FUJITSU FM16pi Emulator 'eFM16pi'

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ virtual machine ]
*/

#ifndef _FM7_H_
#define _FM7_H_

#define DEVICE_NAME		"FUJITSU FM7"
#define CONFIG_NAME		"fm7"

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
#define MEMORY_ADDR_MAX		0x10000
#define MEMORY_BANK_SIZE	0x1000
//#define IO_ADDR_MAX		0x10000

// device informations for win32
#define USE_FD1
#define USE_FD2
#define NOTIFY_KEY_DOWN
#define NOTIFY_KEY_UP
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_POWER_OFF
#define USE_ACCESS_LAMP
//#define USE_DEBUGGER

#define ENABLE_OPENCL // If OpenCL renderer is enabled, define here.

#include "../../common.h"


class EMU;
class DEVICE;
class EVENT;

class BEEP;
class MC6809;
class YM2203;
class MB8877;
class MEMORY;

#if 0
class Z80;
#endif
class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
	MC6809* maincpu;
	MB8877* fdc;
	MEMORY* mainmemory;
        YM2203* opn;
        YM2203* whg;
        YM2203* thg;
        YM2203* psg; // Is right? AY-3-8910 is right device.
        BEEP* beep;
#if 0
        Z80* z80cpu;
#endif
#if 0 // WILL Implement
        FM7_CMT* cmt;
        FM7_OPNJOY* joystick_opn;
        FM7_LPT* printer;
        FM7_MOUSE* mouse_opn;
        FM7_MAINIRQ* main_interrupt;
        FM7_SHAREDBUS* mainsub_bus;
        FM7_DPALET* ttl_palette;
        
        FM7_APALET* analog_palette;
        FM7_MMR* mmr;
        FM7_WINDOW *fm7_window;
        FM7_DMA* dma;
#endif	
	MC6809* subcpu;
        MEMORY* submemory;
#if 0 // WILL Implement
        FM7_DISPLAY* display;
        FM7_KBD* keyboard;
        FM7_SUBIRQ* sub_interrupt;
   
        FM7_RTC* rtc;
        FM7_ALU* alu;
#endif	
   
	int machine_version; // 0 = FM8 / 1 = FM7 / 2 = FM77AV / 3 = FM77AV40, etc...
        Uint32 bootmode;   
        Uint32 connected_opns;
        bool cycle_steal = true;
        bool clock_low = false;
        int mainfreq_type;
        Uint32 mainfreq_low;
        Uint32 mainfreq_high;
        Uint32 mainfreq_mmr;
        Uint32 mainfreq_high_mmr;
 
        Uint32 fdd_type[MAX_DRIVES];
        BOOL   fdd_connect[MAX_DRIVES];

        FILEIO* cmt_fileio;
        bool cmt_enabled = true; // 77AV40SX is disabled.
        bool cmt_play;
        bool cmt_rec;
        Uint32 cmt_bufptr;
   
	// memory
	// MAIN
	uint8 mainram_b1[0x10000]; // 0x10000, RAM
	uint8 mainram_b2[0x10000]; // 0x20000, RAM
	uint8 mainram_b3a[0x8000]; // 0x30000, RAM
        uint8 basicrom[0x7c00]; // 0x38000, ROM
        uint8 mainram_b3b[0x7c00]; // 0x38000, RAM
        uint8 shadowram[0x80]; // 0x3fc00, RAM
        uint8 sharedram[0x80]; // 0x3fc80, Shared RAM
        uint8 mainio[0x100]; // 0xfd00 - 0xfdff I/O
        uint8 boot_bas[0x1f0]; // 0xfe00, BOOT(BAS)
        uint8 boot_dos[0x1f0];
        uint8 boot_bubl[0x1f0];
        uint8 boot_ram[0x1f0];
        uint8 main_vector[0x10]; // 0xfff0, VECTOR(main)
        uint8 initiate[0x2000]; // Initiate RAM.

        uint8 dictrom[0x40000];
	uint8 kanjirom1[0x20000];
	uint8 kanjirom2[0x20000];
        // SUB
	uint8 vram_b1[0xc000];
        uint8 vram_b2[0xc000];
	uint8 vram_b3[0xc000];
        uint8 subchar[0x1000];
        uint8 subwork[0x0380];
        //uint8 sharedram_sub[0x80];
        uint8 subio[0x400]; // Really?
        uint8 cgrom[0x2000];
        uint8 submon_a[0x2000];
        uint8 submon_b[0x2000];
        uint8 submon_c[0x2800];
        uint8 extsubrom[0xc000];
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
	void notify_power_off();
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
