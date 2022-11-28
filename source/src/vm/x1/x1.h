/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

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
#define VDP_CLOCK		28636363
//#ifdef _X1TURBO_FEATURE
//24KHz
//#define FRAMES_PER_SEC	55.49
//#define LINES_PER_FRAME	448
//#define CHARS_PER_LINE	56
//#define HD46505_CHAR_CLOCK	(VDP_CLOCK * 1.5 / 32.0)
//#else
// 15KHz
#define FRAMES_PER_SEC		61.94
#define LINES_PER_FRAME 	258
#define CHARS_PER_LINE		56
#define HD46505_CHAR_CLOCK	(VDP_CLOCK / 32.0)
//#endif
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define IO_ADDR_MAX		0x10000
#define HAS_AY_3_8910
#define Z80_IO_WAIT
#ifdef _X1TURBO_FEATURE
#define SINGLE_MODE_DMA
#endif
#define DATAREC_FAST_FWD_SPEED	16
#define DATAREC_FAST_REW_SPEED	16
#define PRINTER_STROBE_RISING_EDGE
#define SCSI_HOST_AUTO_ACK

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
#define USE_KEYBOARD_TYPE	2
// Keyboard mode B
#define KEYBOARD_TYPE_DEFAULT	1
#define USE_DRIVE_TYPE		3
#endif
#define USE_FLOPPY_DISK		4
#define BASE_FLOPPY_DISK_NUM	0
#define USE_HARD_DISK		4
#define USE_TAPE		1
#ifdef _X1TWIN
#define USE_CART		1
#endif
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	2
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#define USE_SOUND_TYPE		3
// CZ-8BS1 x1
#define SOUND_TYPE_DEFAULT	1
#if defined(_X1TWIN)
#define USE_SOUND_VOLUME	7
#else
#define USE_SOUND_VOLUME	6
#endif
#define USE_JOYSTICK
#ifdef _X1TWIN
#define USE_JOYSTICK_TYPE	4
#define JOYSTICK_TYPE_DEFAULT	0
#define USE_JOY_BUTTON_CAPTIONS
#endif
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
	_T("PSG"), _T("CZ-8BS1 #1"), _T("CZ-8BS1 #2"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
#if defined(_X1TWIN)
	_T("PC Engine PSG"),
#endif
};
#endif

#ifdef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Button #1"),
	_T("Button #2"),
	_T("Select"),
	_T("Run"),
	_T("Button #3"),
	_T("Button #4"),
	_T("Button #5"),
	_T("Button #6"),
};
#endif

// from X-millenium

static const uint16_t ANKFONT7f_9f[0x21 * 8] = {
	0x0000, 0x3000, 0x247f, 0x6c24, 0x484c, 0xce4b, 0x0000, 0x0000,

	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080,
	0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0,
	0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8,
	0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc,
	0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe,
	0x0101, 0x0202, 0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080,

	0x0000, 0x0000, 0x0000, 0x0000, 0x00ff, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x00ff, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x10ff, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x10f0, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x101f, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x10ff, 0x1010, 0x1010, 0x1010,
	0x0000, 0x0000, 0x0000, 0x0000, 0x10f0, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x00f0, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x1010, 0x1010, 0x001f, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x101f, 0x1010, 0x1010, 0x1010,
	0x0000, 0x0000, 0x0000, 0x0000, 0x4080, 0x2020, 0x1010, 0x1010,
	0x1010, 0x1010, 0x0810, 0x0408, 0x0003, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x2010, 0x4020, 0x0080, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0403, 0x0808, 0x1010, 0x1010,
	0x8080, 0x4040, 0x2020, 0x1010, 0x0808, 0x0404, 0x0202, 0x0101
};

static const uint16_t ANKFONTe0_ff[0x20 * 8] = {
	0x0000, 0x7e3c, 0xffff, 0xdbdb, 0xffff, 0xe7db, 0x7eff, 0x003c,
	0x0000, 0x423c, 0x8181, 0xa5a5, 0x8181, 0x99a5, 0x4281, 0x003c,
	0x0000, 0x3810, 0x7c7c, 0xfefe, 0xfefe, 0x106c, 0x7c38, 0x0000,
	0x0000, 0x6c00, 0xfefe, 0xfefe, 0xfefe, 0x7c7c, 0x1038, 0x0000,
	0x0000, 0x1010, 0x3838, 0x7c7c, 0x7cfe, 0x387c, 0x1038, 0x0010,
	0x0000, 0x3810, 0x7c7c, 0x5438, 0xfefe, 0x6cfe, 0x7c10, 0x0000,
	0x0101, 0x0303, 0x0707, 0x0f0f, 0x1f1f, 0x3f3f, 0x7f7f, 0xffff,
	0x8080, 0xc0c0, 0xe0e0, 0xf0f0, 0xf8f8, 0xfcfc, 0xfefe, 0xffff,
	0x8181, 0x4242, 0x2424, 0x1818, 0x1818, 0x2424, 0x4242, 0x8181,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f,
	0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f,
	0x81ff, 0x8181, 0x8181, 0x8181, 0x8181, 0x8181, 0x8181, 0xff81,

	0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa,
	0x1000, 0x1010, 0xf01e, 0x1010, 0x1010, 0x1010, 0x7e10, 0x00c0,
	0x1000, 0x2418, 0x7c42, 0x1090, 0x781c, 0x5410, 0xfe54, 0x0000,
	0x1000, 0x1010, 0xfe10, 0x1010, 0x3030, 0x565c, 0x9090, 0x0010,
	0x1000, 0x1210, 0xf412, 0x3034, 0x5030, 0x9654, 0x1090, 0x0000,
	0x0800, 0x8808, 0x5292, 0x1454, 0x2020, 0x5020, 0x465c, 0x0040,
	0x0000, 0xe23c, 0x8282, 0x82fa, 0x7a82, 0x4242, 0x0044, 0x0000,
	0x0000, 0x443c, 0x8242, 0xf282, 0x8282, 0x8282, 0x3844, 0x0000,
	0x0800, 0x5e18, 0xa468, 0xe4be, 0xbea4, 0xb2a2, 0x0a5a, 0x0002,
	0x0000, 0x2628, 0x4042, 0xe23c, 0x2222, 0x4222, 0x4442, 0x0004,
	0x0800, 0x0808, 0xda7a, 0x5454, 0x46e4, 0xe442, 0x08c4, 0x0000,
	0x0000, 0x7e40, 0x8848, 0x28be, 0x2828, 0x3e28, 0x08e8, 0x0008,
	0x0000, 0x723c, 0x9252, 0x9292, 0x8292, 0x84fc, 0x8484, 0x0000,
	0x0000, 0x1010, 0x2010, 0x2020, 0x6040, 0x8c50, 0x8286, 0x0000,
	0x0000, 0x4040, 0x784e, 0x88c0, 0x388e, 0x0848, 0x7e08, 0x0000,
	0x0000, 0x7c00, 0x0000, 0x0000, 0x10fe, 0x1010, 0x1010, 0x0010
};

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class I8255;
class IO;
class MB8877;
class SASI_HDD;
class SCSI_HOST;
class YM2151;
//class YM2203;
class AY_3_891X;
class Z80;
class Z80CTC;
class Z80SIO;
#ifdef _X1TURBO_FEATURE
class Z80DMA;
#endif

class DISPLAY;
class EMM;
class FLOPPY;
class IOBUS;
class JOYSTICK;
class MEMORY;
class MOUSE;
class PSUB;
class SASI;
class CZ8RB;

class MCS48;
class UPD1990A;
class SUB;
class KEYBOARD;

#ifdef _X1TWIN
class HUC6280;
class PCE;
#endif

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices for x1
	EVENT* event;
	
	DATAREC* drec;
	DEVICE* printer;
	HD46505* crtc;
	I8255* pio;
	IO* io;
	MB8877* fdc;
	SASI_HDD* sasi_hdd[(USE_HARD_DISK >> 1) + (USE_HARD_DISK & 1)];
	SCSI_HOST* sasi_host;
	YM2151* opm1;
	YM2151* opm2;
//	YM2203* psg;
	AY_3_891X* psg;
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
	IOBUS* iobus;
	JOYSTICK* joy;
	MEMORY* memory;
	MOUSE* mouse;
	PSUB* psub;
	SASI* sasi;
	CZ8RB* cz8rb;
	
	MCS48* cpu_sub;
	UPD1990A* rtc_sub;
	I8255* pio_sub;
	SUB* sub;
	
	MCS48* cpu_kbd;
	KEYBOARD* kbd;
	
	bool pseudo_sub_cpu;
	int sound_type;
	
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
	bool get_caps_locked();
	bool get_kana_locked();
	
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
	void push_apss_forward(int drv);
	void push_apss_rewind(int drv);
	bool is_frame_skippable();
#ifdef _X1TWIN
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
#endif
	
	void update_config();
#ifdef _X1TURBO_FEATURE
	void update_dipswitch();
#endif
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
