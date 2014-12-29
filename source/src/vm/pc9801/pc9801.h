/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.15-

	[ virtual machine ]
*/

#ifndef _PC9801_H_
#define _PC9801_H_

#if defined(_PC9801)
#define DEVICE_NAME		"NEC PC-9801"
#define CONFIG_NAME		"pc9801"
#elif defined(_PC9801E)
#define DEVICE_NAME		"NEC PC-9801E/F/M"
#define CONFIG_NAME		"pc9801e"
#elif defined(_PC9801U)
#define DEVICE_NAME		"NEC PC-9801U"
#define CONFIG_NAME		"pc9801u"
#elif defined(_PC9801VF)
#define DEVICE_NAME		"NEC PC-9801VF"
#define CONFIG_NAME		"pc9801vf"
#elif defined(_PC9801VM)
#define DEVICE_NAME		"NEC PC-9801VM"
#define CONFIG_NAME		"pc9801vm"
#elif defined(_PC98DO)
#define DEVICE_NAME		"NEC PC-98DO"
#define CONFIG_NAME		"pc98do"
#else
#endif

#if defined(_PC9801) || defined(_PC9801E)
#define SUPPORT_CMT_IF
#define SUPPORT_2HD_FDD_IF
#define SUPPORT_2DD_FDD_IF
#define SUPPORT_320KB_FDD_IF
#define SUPPORT_OLD_BUZZER
#elif defined(_PC9801VF) || defined(_PC9801U)
#define SUPPORT_2DD_FDD_IF
#else
#define SUPPORT_2HD_2DD_FDD_IF
#endif

#if !(defined(_PC9801) || defined(_PC9801U))
#define SUPPORT_2ND_VRAM
#endif
#if !(defined(_PC9801) || defined(_PC9801E))
#define SUPPORT_16_COLORS
#endif

#if defined(_PC98DO)
#define MODE_PC98	0
#define MODE_PC88_V1S	1
#define MODE_PC88_V1H	2
#define MODE_PC88_V2	3
#define MODE_PC88_N	4
//#define SUPPORT_PC88_DICTIONARY
#define SUPPORT_PC88_HIGH_CLOCK
//#define SUPPORT_PC88_JOYSTICK
#define PC88_EXRAM_BANKS	4
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		56.4
#define LINES_PER_FRAME 	440
#if defined(_PC9801)
#define CPU_CLOCKS		4992030
#define PIT_CLOCK_5MHZ
#elif defined(_PC9801E) || defined(_PC9801U) || defined(_PC9801VF)
#define CPU_CLOCKS		7987248
#define PIT_CLOCK_8MHZ
#else
#define CPU_CLOCKS		9984060
#define PIT_CLOCK_5MHZ
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define UPD765A_NO_ST1_EN_OR_FOR_RESULT7
#define UPD7220_MSB_FIRST
#define UPD7220_HORIZ_FREQ	24830
#if defined(_PC9801) || defined(_PC9801E)
#define HAS_I86
#else
#define HAS_V30
#endif
#if defined(_PC98DO)
#define HAS_UPD4990A
//#define HAS_YM2608
#define Z80_MEMORY_WAIT
#endif
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x800
#define IO_ADDR_MAX		0x10000
#if !defined(SUPPORT_OLD_BUZZER)
#define PCM1BIT_HIGH_QUALITY
#endif
#define OVERRIDE_SOUND_FREQ_48000HZ	55467
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#if defined(_PC98DO)
#define USE_BOOT_MODE		5
#define USE_DIPSWITCH
#endif
#if defined(_PC9801E) || defined(_PC9801VM) || defined(_PC98DO)
#define USE_CPU_TYPE		2
#endif
#define USE_FD1
#define USE_FD2
#if defined(_PC9801) || defined(_PC9801E)
// for 640KB drives
#define USE_FD3
#define USE_FD4
// for 320KB drives
#define USE_FD5
#define USE_FD6
#elif defined(_PC98DO)
// for PC-8801 drives
#define USE_FD3
#define USE_FD4
#endif
#if defined(SUPPORT_CMT_IF)
#define USE_TAPE
#define TAPE_BINARY_ONLY
#elif defined(_PC98DO)
#define USE_TAPE
#endif
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#if defined(_PC98DO)
// slow enough for N88-“ú–{ŒêBASIC
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_MONITOR_TYPE	2
#define USE_SCANLINE
#else
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#endif
#define USE_CRT_FILTER
#define USE_ACCESS_LAMP
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

#if defined(SUPPORT_OLD_BUZZER)
class BEEP;
#endif
class I8237;
class I8251;
class I8253;
class I8255;
class I8259;
#if defined(HAS_V30)
class I86;
#else
class I286;
#endif
class IO;
class LS244;
class MEMORY;
#if defined(HAS_I86) || defined(HAS_V30)
class NOT;
#endif
#if !defined(SUPPORT_OLD_BUZZER)
class PCM1BIT;
#endif
class UPD1990A;
class UPD7220;
class UPD765A;
class YM2203;

#if defined(SUPPORT_CMT_IF)
class CMT;
#endif
class DISPLAY;
class FLOPPY;
class FMSOUND;
class JOYSTICK;
class KEYBOARD;
class MOUSE;
class PRINTER;

#if defined(SUPPORT_320KB_FDD_IF)
// 320kb fdd drives
class PC80S31K;
class Z80;
#endif

#if defined(_PC98DO)
class BEEP;
class PC80S31K;
class PC88;
class Z80;
#endif

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* event;
	
#if defined(SUPPORT_OLD_BUZZER)
	BEEP* beep;
#else
	PCM1BIT* beep;
#endif
	I8237* dma;
#if defined(SUPPORT_CMT_IF)
	I8251* sio_cmt;
#endif
	I8251* sio_rs;
	I8251* sio_kbd;
	I8253* pit;
#if defined(SUPPORT_320KB_FDD_IF)
	I8255* pio_fdd;
#endif
	I8255* pio_mouse;
	I8255* pio_sys;
	I8255* pio_prn;
	I8259* pic;
#if defined(HAS_V30)
	I86* cpu;
#else
	I286* cpu;
#endif
	IO* io;
	LS244* dmareg1;
	LS244* dmareg2;
	LS244* dmareg3;
	LS244* dmareg0;
	LS244* rtcreg;
	MEMORY* memory;
#if defined(HAS_I86) || defined(HAS_V30)
	NOT* not;
#endif
	UPD1990A* rtc;
#if defined(SUPPORT_2HD_FDD_IF)
	UPD765A* fdc_2hd;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	UPD765A* fdc_2dd;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	UPD765A* fdc;
#endif
	UPD7220* gdc_chr;
	UPD7220* gdc_gfx;
	YM2203* opn;
	
#if defined(SUPPORT_CMT_IF)
	CMT* cmt;
#endif
	DISPLAY* display;
	FLOPPY* floppy;
	FMSOUND* fmsound;
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MOUSE* mouse;
	PRINTER* printer;
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	I8255* pio_sub;
	PC80S31K *pc80s31k;
	UPD765A* fdc_sub;
	Z80* cpu_sub;
#endif
	
	// memory
	uint8 ram[0xa0000];
	uint8 ipl[0x18000];
	uint8 sound_bios[0x4000];
#if defined(_PC9801) || defined(_PC9801E)
	uint8 fd_bios_2hd[0x1000];
	uint8 fd_bios_2dd[0x1000];
#endif
	bool pit_clock_8mhz;
	
#if defined(_PC98DO)
	EVENT* pc88event;
	
	PC88* pc88;
	BEEP* pc88beep;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
	YM2203* pc88opn;
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	Z80* pc88cpu_sub;
	
	int boot_mode;
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
#if defined(SUPPORT_CMT_IF) || defined(_PC98DO)
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
#endif
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
