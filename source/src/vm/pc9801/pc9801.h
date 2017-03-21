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
#elif defined(_PC98DOPLUS)
#define DEVICE_NAME		"NEC PC-98DO+"
#define CONFIG_NAME		"pc98do+"
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

// PC-9801-86
//#define SUPPORT_PC98_OPNA

#if defined(_PC98DO) || defined(_PC98DOPLUS)
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
#if defined(_PC98DOPLUS)
#define SUPPORT_PC88_OPNA
#define SUPPORT_PC88_SB2
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
#elif defined(_PC98DOPLUS)
#define CPU_CLOCKS		15974496
#define PIT_CLOCK_8MHZ
#else
#define CPU_CLOCKS		9984060
#define PIT_CLOCK_5MHZ
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		2
#define UPD765A_NO_ST1_EN_OR_FOR_RESULT7
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#define PC80S31K_NO_WAIT
#endif
#define UPD7220_MSB_FIRST
#define UPD7220_HORIZ_FREQ	24830
#if defined(_PC9801) || defined(_PC9801E)
#define HAS_I86
#elif defined(_PC98DOPLUS)
#define HAS_V33A
#else
#define HAS_V30
#endif
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#define HAS_UPD4990A
#define Z80_MEMORY_WAIT
#endif
#if defined(SUPPORT_PC98_OPNA) || defined(SUPPORT_PC88_OPNA)
#define HAS_YM2608
#endif
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define MEMORY_ADDR_MAX		0x100000
#define MEMORY_BANK_SIZE	0x800
#define IO_ADDR_MAX		0x10000
#define OVERRIDE_SOUND_FREQ_48000HZ	55467
#define SUPPORT_VARIABLE_TIMING

// device informations for win32
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#define USE_BOOT_MODE		5
#define USE_DIPSWITCH
#endif
#if defined(_PC9801E) || defined(_PC9801VM) || defined(_PC98DO) || defined(_PC98DOPLUS)
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
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
// for PC-8801 drives
#define USE_FD3
#define USE_FD4
#endif
#if defined(SUPPORT_CMT_IF) || defined(_PC98DO) || defined(_PC98DOPLUS)
#define USE_TAPE1
#define TAPE_BINARY_ONLY
#endif
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#if defined(_PC98DO) || defined(_PC98DOPLUS)
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
#define USE_SCREEN_ROTATE
#define USE_SOUND_DEVICE_TYPE	5
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#if    defined(SUPPORT_PC98_OPNA) &&  defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 4 + 1 + 1)
#elif  defined(SUPPORT_PC98_OPNA) && !defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 2 + 1 + 1)
#elif !defined(SUPPORT_PC98_OPNA) &&  defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 4 + 1 + 1)
#elif !defined(SUPPORT_PC98_OPNA) && !defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 2 + 1 + 1)
#endif
#else
#if defined(SUPPORT_PC98_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 1)
#else
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 1)
#endif
#endif
#define USE_JOYSTICK
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(SUPPORT_PC98_OPNA)
	_T("PC-9801-86 (FM)"), _T("PC-9801-86 (PSG)"), _T("PC-9801-86 (ADPCM)"), _T("PC-9801-86 (Rhythm)"),
#else
	_T("PC-9801-26 (FM)"), _T("PC-9801-26 (PSG)"),
#endif
	_T("PC-9801-14"), _T("Beep"),
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#if defined(SUPPORT_PC88_OPNA)
	_T("PC-88 OPNA (FM)"), _T("PC-88 OPNA (PSG)"), _T("PC-88 OPNA (ADPCM)"), _T("PC-88 OPNA (Rhythm)"),
#else
	_T("PC-88 OPN (FM)"), _T("PC-88 OPN (PSG)"),
#endif
	_T("PC-88 Beep"), 
#endif
	_T("Noise (FDD)"),
};
#endif

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
#if defined(HAS_I86) || defined(HAS_V30)
class I86;
#else
class I286;
#endif
class IO;
class LS244;
class MEMORY;
class NOISE;
class NOT;
#if !defined(SUPPORT_OLD_BUZZER)
class PCM1BIT;
#endif
class TMS3631;
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

#if defined(SUPPORT_320KB_FDD_IF)
// 320kb fdd drives
class PC80S31K;
class Z80;
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
class PC80S31K;
class PC88;
class Z80;
#endif

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
	DEVICE* printer;
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
#if defined(HAS_I86) || defined(HAS_V30)
	I86 *cpu;
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
	NOT* not_busy;
#if defined(HAS_I86) || defined(HAS_V30)
	NOT* not_prn;
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
	NOISE* noise_seek;
	NOISE* noise_head_down;
	NOISE* noise_head_up;
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
	
	// PC-9801-14
	TMS3631* tms3631;
	I8253* pit_14;
	I8255* pio_14;
	LS244* maskreg_14;
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	I8255* pio_sub;
	PC80S31K *pc80s31k;
	UPD765A* fdc_sub;
	Z80* cpu_sub;
#endif
	
	// memory
	uint8_t ram[0xa0000];
	uint8_t ipl[0x18000];
	uint8_t sound_bios[0x4000];
#if defined(_PC9801) || defined(_PC9801E)
	uint8_t fd_bios_2hd[0x1000];
	uint8_t fd_bios_2dd[0x1000];
#endif
	bool pit_clock_8mhz;
	
	// sound
	int sound_device_type;
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	EVENT* pc88event;
	
	PC88* pc88;
	DEVICE* pc88prn;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
	YM2203* pc88opn;
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	NOISE* pc88noise_seek;
	NOISE* pc88noise_head_down;
	NOISE* pc88noise_head_up;
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#if defined(SUPPORT_CMT_IF) || defined(_PC98DO) || defined(_PC98DOPLUS)
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
#endif
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
