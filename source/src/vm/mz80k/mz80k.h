/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#ifndef _MZ80K_H_
#define _MZ80K_H_

#if defined(_MZ1200)
#define DEVICE_NAME		"SHARP MZ-1200"
#define CONFIG_NAME		"mz1200"
#elif defined(_MZ80A)
#define DEVICE_NAME		"SHARP MZ-80A"
#define CONFIG_NAME		"mz80a"
#else
#define DEVICE_NAME		"SHARP MZ-80K/C"
#define CONFIG_NAME		"mz80k"
#endif

#ifdef _MZ80A
#define SUPPORT_MZ80AIF
#else
#define SUPPORT_MZ80FIO
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		2000000
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define WINDOW_HEIGHT_ASPECT	240
#if defined(SUPPORT_MZ80AIF)
#define HAS_MB8866
#define MAX_DRIVE		4
#elif defined(SUPPORT_MZ80FIO)
#define HAS_T3444M
#define MAX_DRIVE		4
#endif
#define PRINTER_STROBE_RISING_EDGE

// device informations for win32
#define USE_DIPSWITCH
#define USE_TAPE		1
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NO_CAPS
#if defined(_MZ80K) || defined(_MZ1200)
//#define USE_AUTO_KEY_NUMPAD
#define USE_VM_AUTO_KEY_TABLE
#endif
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
#define USE_SOUND_VOLUME	4
#else
#define USE_SOUND_VOLUME	3
#endif
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#define USE_DEBUGGER
#define USE_STATE
#if defined(SUPPORT_MZ80AIF)
#define USE_DRIVE_TYPE		2
#endif
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
#define USE_FLOPPY_DISK		4
#endif
// COLOR GAL 5 - 2019.01.24 Suga
#if defined(_MZ80K)
#define USE_MONITOR_TYPE	8
#endif
#if defined(_MZ1200)
#define USE_MONITOR_TYPE	4
#endif
#define USE_CPU_Z80

#if defined(_MZ80K) || defined(_MZ1200)
static const int vm_auto_key_table_base[][2] = {
	{0xa1,	0x200 | 0x74},	// '｡': KANA + F5
	{0xa2,	0x200 | 0x70},	// '｢': KANA + F1
	{0xa3,	0x200 | 0x71},	// '｣': KANA + F2
	{0xa4,	0x200 | 0x72},	// '､': KANA + F3
	{0xa5,	0x200 | 0x73},	// '･': KANA + F4
	{0xa6,	0x200 | 0xdb},	// 'ｦ': KANA + '['
	{0xa7,	0x200 | 0x6f},	// 'ｧ': KANA + NumPad '/'
	{0xa8,	0x200 | 0x67},	// 'ｨ': KANA + NumPad '7'
	{0xa9,	0x200 | 0x64},	// 'ｩ': KANA + NumPad '4'
	{0xaa,	0x200 | 0x61},	// 'ｪ': KANA + NumPad '1'
	{0xab,	0x200 | 0x75},	// 'ｫ': KANA + F6
	{0xac,	0x200 | 0xfa},	// 'ｬ': KANA + NumPad '*'
	{0xad,	0x200 | 0x68},	// 'ｭ': KANA + NumPad '8'
	{0xae,	0x200 | 0x65},	// 'ｮ': KANA + NumPad '5'
	{0xaf,	0x200 | 0x62},	// 'ｯ': KANA + NumPad '2'
	{0xb0,	0x200 | 0x76},	// 'ｰ': KANA + F7
	{0xb9,	0x200 | 0xde},	// 'ｹ': KANA + '^'
	{0xcd,	0x200 | 0xc0},	// 'ﾍ': KANA + '@'
	{0xd1,	0x200 | 0xdc},	// 'ﾑ': KANA + '\'
	{0xdb,	0x200 | 0xba},	// 'ﾛ': KANA + ':'
	{0xde,	0x200 | 0xe2},	// 'ﾞ': KANA + '_'
	{0xdf,	0x200 | 0xdd},	// 'ﾟ': KANA + ']'
	{-1,	-1},
};
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CMT (Signal)"),
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	_T("Noise (FDD)"),
#endif
	_T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

#if defined(_MZ1200) || defined(_MZ80A)
class AND;
#endif
class DATAREC;
class I8253;
class I8255;
class LS393;
class PCM1BIT;
class Z80;

namespace MZ80 {
	class KEYBOARD;
	class MEMORY;
	class PRINTER;
}
#if defined(SUPPORT_MZ80AIF)
class MB8877;
class IO;
namespace MZ80 {
	class MZ80AIF;
}
#elif defined(SUPPORT_MZ80FIO)
class IO;
class T3444A;
namespace MZ80 {
	class MZ80FIO;
}
#endif

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	//EVENT* event;
	
#if defined(_MZ1200) || defined(_MZ80A)
	AND* and_int;
#endif
	DATAREC* drec;
	I8253* ctc;
	I8255* pio;
	LS393* counter;
	PCM1BIT* pcm;
	Z80* cpu;
	
	MZ80::KEYBOARD* keyboard;
	MZ80::MEMORY* memory;
	MZ80::PRINTER* printer;
	
#if defined(SUPPORT_MZ80AIF)
	MB8877* fdc;
	IO* io;
	MZ80::MZ80AIF* mz80aif;
#elif defined(SUPPORT_MZ80FIO)
	T3444A* fdc;
	IO* io;
	MZ80::MZ80FIO* mz80fio;
#endif
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU_TEMPLATE* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	double get_frame_rate()
	{
		return FRAMES_PER_SEC;
	}
	
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
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#endif
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
	
	double get_current_usec();
	uint64_t get_current_clock_uint64();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
