/*
	FUJITSU FM7 Emulator 'eFM7'

	Author : K.Ohta
	Date   : 2015.01.01-

	[ virtual machine ]
*/

#ifndef _FM7_H_
#define _FM7_H_

#define USE_TAPE 1
#define USE_TAPE_PTR
#define USE_SCANLINE
#define USE_DIPSWITCH
#define USE_CPU_TYPE 2
#define USE_SPECIAL_RESET
#define USE_LED_DEVICE			3
#define USE_MINIMUM_RENDERING 1
#define USE_MOUSE
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_PRINTER
#define USE_PRINTER_TYPE 4
#define USE_AY_3_8910_AS_PSG

#define INDEPENDENT_CAPS_KANA_LED
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_STATE
#define USE_DEBUGGER
#define DATAREC_SOUND
#define USE_DIG_RESOLUTION

#define CPU_TYPE_DEFAULT 0

#if defined(_FM8)
#define DEVICE_NAME		"FUJITSU FM-8"
#define CONFIG_NAME		"fm8"
#define CAPABLE_Z80
#define DIPSWITCH_DEFAULT 0x00000000 
#define USE_BUBBLE 2
#define BASE_BUBBLE_NUM 0
#define MAX_DRIVE  4

#undef CPU_TYPE_DEFAULT
#define CPU_TYPE_DEFAULT 1
#define USE_GREEN_DISPLAY
#define USE_MONITOR_TYPE 2

#elif defined(_FM7)
#define DEVICE_NAME		"FUJITSU FM-7"
#define CONFIG_NAME		"fm7"
#define CAPABLE_Z80
#define DIPSWITCH_DEFAULT 0x000000000 
#define MAX_DRIVE  4
#define CAPABLE_JCOMMCARD 1
#define USE_GREEN_DISPLAY
#define USE_MONITOR_TYPE 2

#elif defined(_FMNEW7)
#define DEVICE_NAME		"FUJITSU FM-NEW7"
#define CONFIG_NAME		"fmnew7"
#define CAPABLE_Z80
#define DIPSWITCH_DEFAULT 0x000000000 
#define MAX_DRIVE  4
#define CAPABLE_JCOMMCARD 1
#define USE_GREEN_DISPLAY
#define USE_MONITOR_TYPE 2

#elif defined(_FM77) || defined(_FM77L2)
# if defined(_FM77)
#define DEVICE_NAME		"FUJITSU FM-77"
#define CONFIG_NAME		"fm77"
#define DIPSWITCH_DEFAULT 0x00000001
# else
#define DEVICE_NAME		"FUJITSU FM-77L2"
#define CONFIG_NAME		"fm77l2"
#define DIPSWITCH_DEFAULT 0x00000003 
# endif
//#define USE_DRIVE_TYPE
#define _FM77_VARIANTS
#define CAPABLE_Z80
# ifndef FM77_EXRAM_BANKS
#   define FM77_EXRAM_BANKS	3
# endif
#define MAX_DRIVE  4
#define CAPABLE_JCOMMCARD 1
#define HAS_2HD
#define USE_GREEN_DISPLAY
#define USE_MONITOR_TYPE 2

#elif defined(_FM77L4)
#define DEVICE_NAME		"FUJITSU FM-77L4"
#define CONFIG_NAME		"fm77l4"
#define HAS_TEXTVRAM
#define HAS_2HD
#define HAS_CYCLESTEAL
#define HAS_400LINECARD
//#define CAPABLE_KANJI_CLASS2
#define _FM77_VARIANTS
# define CAPABLE_Z80
# ifndef FM77_EXRAM_BANKS
#  define FM77_EXRAM_BANKS	3
# endif
#define DIPSWITCH_DEFAULT 0x00000003 
#define MAX_DRIVE  4
#define CAPABLE_JCOMMCARD 1
#define USE_GREEN_DISPLAY
#define USE_MONITOR_TYPE 2

#elif defined(_FM77AV)
#define DEVICE_NAME		"FUJITSU FM77AV"
#define CONFIG_NAME		"fm77av"
#define _FM77AV_VARIANTS
#define DIPSWITCH_DEFAULT 0x80000001 
#define MAX_DRIVE  2
#define CAPABLE_JCOMMCARD 1
#define CAPABLE_DICTROM

#elif defined(_FM77AV20)
#define DEVICE_NAME		"FUJITSU FM77AV20"
#define CONFIG_NAME		"fm77av20"
#define _FM77AV_VARIANTS
#define HAS_2DD_2D
#define CAPABLE_DICTROM
//#define USE_DRIVE_TYPE 2
#define CAPABLE_KANJI_CLASS2
#define DIPSWITCH_DEFAULT 0x80000001 
#define MAX_DRIVE  2

#elif defined(_FM77AV20EX)
#define DEVICE_NAME		"FUJITSU FM77AV20EX"
#define CONFIG_NAME		"fm77av20ex"
#define _FM77AV_VARIANTS
#define HAS_2DD_2D
#define HAS_DMA
//#define USE_DRIVE_TYPE 2
#define CAPABLE_DICTROM
#define CAPABLE_KANJI_CLASS2
#define DIPSWITCH_DEFAULT 0x80000001 
#define MAX_DRIVE  4

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
#define DIPSWITCH_DEFAULT 0x8000000d 
#define MAX_DRIVE  4

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
#define DIPSWITCH_DEFAULT 0x8000000d 
#define MAX_DRIVE  4

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
# ifndef FM77_EXRAM_BANKS
#  define FM77_EXRAM_BANKS	12
# endif
#define DIPSWITCH_DEFAULT 0x8000000d 
#define MAX_DRIVE  4

#endif

#if defined(_FM8)
#define USE_SOUND_TYPE		2
#else
#define USE_MOUSE_TYPE		3
#define USE_SOUND_TYPE		8
#endif

#ifdef _FM77AV_VARIANTS
#define HAS_MMR
#define HAS_CYCLESTEAL
#elif defined(_FM77_VARIANTS)
#define HAS_MMR
#define HAS_CYCLESTEAL
#define CAPABLE_Z80
#endif

#define USE_BOOT_MODE         8

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
#define SOUND_TYPE_DEFAULT	0
#elif defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
// PSG ONLY
#define SOUND_TYPE_DEFAULT	0
#elif defined(_FM77AV_VARIANTS)
// OPN
#define SOUND_TYPE_DEFAULT	1
#endif

#if defined(_FM8)
#define USE_SOUND_VOLUME	5
#else
#define USE_SOUND_VOLUME	11
#endif
#define SUPPORT_TV_RENDER

#define IGNORE_DISK_CRC_DEFAULT		true
// device informations for virtual machine

// TODO: check refresh rate
//#define FRAMES_PER_SEC		60.0962 /* Related to display.cpp */ 
#define FRAMES_PER_SEC		59.94 /* Related to display.cpp */ 
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77L4)
#define LINES_PER_FRAME 	400
#else
#define LINES_PER_FRAME 	200
#endif

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
  defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
#define CPU_CLOCKS		2016000 /* 2016 */
#else
#define CPU_CLOCKS      1798000   /* FM8,7 - 77AV/20? */
#endif


/*
 * If you wish to use software-scanline, define below.
 * If not, undef.
 */
#define FIXED_FRAMEBUFFER_SIZE

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77L4)
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#undef SCREEN_FAKE_WIDTH
#undef SCREEN_FAKE_HEIGHT
#else /* Not have 400 line */

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		200
# if defined(FIXED_FRAMEBUFFER_SIZE)
#  define SCREEN_FAKE_WIDTH   640
#  define SCREEN_FAKE_HEIGHT  400
# else
#  define SCREEN_FAKE_WIDTH   640
#  define SCREEN_FAKE_HEIGHT  200
# endif

# if !defined(FIXED_FRAMEBUFFER_SIZE)
#  define SCREEN_Y_ZOOM       2.0
# endif

#endif
#define WINDOW_HEIGHT_ASPECT 480
#define HAS_MC6809              
#define MB8877_MAX_CHIPS	1
#define MB8877_DELAY_AFTER_SEEK	60000
//#define IO_ADDR_MAX		0x10000

// device informations for win32
#if defined(HAS_2HD)
#define USE_FLOPPY_DISK 4
#else
#define USE_FLOPPY_DISK 2
#endif
#define BASE_FLOPPY_DISK_NUM 0

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
#define MB8877_NO_BUSY_AFTER_SEEK

//#define ENABLE_OPENCL // If OpenCL renderer is enabled, define here.

//#include "../../config.h"
#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(_FM8)
	_T("PSG(Hack)"),
	_T("Beep"),
	_T("CMT (Signal)"),
	_T("Noise (320KB FDD)"),
	_T("Noise (CMT)"),
#else
# if !defined(_FM77AV_VARIANTS)
	_T("PSG"),
# endif
	_T("OPN (FM)"), _T("OPN (PSG)"), _T("WHG (FM)"), _T("WHG (PSG)"), _T("THG (FM)"), _T("THG (PSG)"),
	_T("Beep"), _T("CMT (Signal)"),
# if defined(_FM77AV_VARIANTS)
	_T("Keyboard"),
# endif
	_T("Noise (320KB FDD)"),
# if defined(HAS_2HD)
	_T("Noise (1MB FDD/2HD)"),
# endif
	_T("Noise (CMT)"),
#endif	
};
#endif
#ifdef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Button #1(1st)"),
	_T("Button #2(1st)"),
	_T("Button #1(2nd)"),
	_T("Button #2(2nd)"),
};
#endif

/*
 * NOTE:
 * Below defines are for EVENT::run_cpu().
 * Please see event.cpp for details.
 * 20180317 -- K.Ohta.
 */

#define USE_CPU_MC6809
#if defined(WITH_Z80)
#define USE_CPU_Z80
#endif

class EMU;
class DEVICE;
class EVENT;
class FILEIO;

#if defined(_FM77AV_VARIANTS)
class BEEP;
#endif
#if defined(_FM77L4)
class HD46505;
#endif
class PCM1BIT;
class MC6809;
class YM2203;
class MB8877;
class MEMORY;
class DATAREC;
class I8251;
#if defined(USE_AY_3_8910_AS_PSG) && !defined(_FM77AV_VARIANTS)
class AY_3_891X;
#endif
#if defined(USE_BUBBLE)
class BUBBLECASETTE;
#endif
class DISPLAY;
#if defined(_FM77AV_VARIANTS)
class MB61VH010;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
class HD6844;
#endif
class AND;
class FM7_MAINMEM;
#if defined(_FM8)
class FM8_MAINIO;
#else
class FM7_MAINIO;
#endif

class KEYBOARD;
class KANJIROM;
class JOYSTICK;

#ifdef WITH_Z80
class Z80;
class OR;
#endif
#ifdef CAPABLE_JCOMMCARD
class FM7_JCOMMCARD;
#endif

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DEVICE* dummycpu;
	MC6809* maincpu;
	FM7_MAINMEM* mainmem;
#if defined(_FM8)
	FM8_MAINIO* mainio;
#else
	FM7_MAINIO* mainio;
#endif
	MB8877* fdc;
#if defined(HAS_2HD)
	MB8877 *fdc_2HD;
#endif
#if defined(_FM8)
	// FM8
# if defined(USE_AY_3_8910_AS_PSG)
	AY_3_891X *psg;
# else
	YM2203* psg;
# endif
#else
	// FM7 -
	YM2203* opn[3];
# if !defined(_FM77AV_VARIANTS)
#  if defined(USE_AY_3_8910_AS_PSG)
	AY_3_891X *psg;
#else
	YM2203* psg; // Is right? AY-3-8910 is right device.
#  endif
# endif
#endif
#if defined(USE_BUBBLE)
	BUBBLECASETTE *bubble_casette[USE_BUBBLE];
#endif
	I8251 *uart[3];
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	AND *g_rs232c_dtr;
#endif
	
	//BEEP* beep;
	PCM1BIT* pcm1bit;
	DATAREC *drec;
	JOYSTICK *joystick;
	
#ifdef  WITH_Z80
	Z80* z80cpu;
	AND *g_nmi;
	OR *g_intr;
	AND *g_intr_irq;
	AND *g_intr_firq;
	AND *g_mainstat;
#endif
	AND *g_substat_display;
	AND *g_substat_mainhalt;
	
	DEVICE* printer;
	DEVICE* inteli_mouse; 
   
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
#if defined(CAPABLE_JCOMMCARD)
	MC6809 *jsubcpu;
	FM7_JCOMMCARD *jcommcard;
	AND *g_jsubhalt;
#endif
#if defined(_FM77L4)
	HD46505 *l4crtc;
#endif
	bool connect_320kfdc;
	bool connect_1Mfdc;
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
	void connect_bus(void);

	void update_dipswitch(void);
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	void set_vm_frame_rate(double fps);
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	bool get_caps_locked();
	bool get_kana_locked();
	uint32_t get_led_status(); 
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
	
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
	
	bool is_frame_skippable();
	void push_play(int drv);
	void push_stop(int drv);
	void push_fast_forward(int drv);
	void push_fast_rewind(int drv);
	void push_apss_forward(int drv);
	void push_apss_rewind(int drv);
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
#if defined(USE_BUBBLE)
	void open_bubble_casette(int drv, const _TCHAR *path, int bank);
	void close_bubble_casette(int drv);
	bool is_bubble_casette_inserted(int drv);
	bool is_bubble_casette_protected(int drv);
	void is_bubble_casette_protected(int drv, bool flag);
#endif
#if defined(USE_DIG_RESOLUTION)
	void get_screen_resolution(int *w, int *h);
#endif
#if defined(USE_MINIMUM_RENDERING)	
	bool is_screen_changed(void);
#endif	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	void set_cpu_clock(DEVICE *cpu, uint32_t clocks);
	// devices
	DEVICE* get_device(int id);
//	DEVICE* dummy;
//	DEVICE* first_device;
//	DEVICE* last_device;
};

#endif
