/*
	SONY SMC-70 Emulator 'eSMC-70'
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ virtual machine ]
*/

#ifndef _SMC777_H_
#define _SMC777_H_

#if defined(_SMC70)
#define DEVICE_NAME		"SONY SMC-70"
#define CONFIG_NAME		"smc70"
#elif defined(_SMC777)
#define DEVICE_NAME		"SONY SMC-777"
#define CONFIG_NAME		"smc777"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CHARS_PER_LINE		128
#define HD46505_HORIZ_FREQ	15734
#define CPU_CLOCKS		4027975
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		2
#define MB8877_NO_BUSY_AFTER_SEEK
#define SUPPORT_MEDIA_TYPE_1DD

// device informations for win32
#if defined(_SMC70)
#define USE_BOOT_MODE		3
#define BOOT_MODE_DEFAULT	1
#endif
#define USE_SPECIAL_RESET	1
#define USE_FLOPPY_DISK		2
#define USE_TAPE		1
#define USE_KEY_LOCKED
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_NUMPAD
#define AUTO_KEY_US
#define USE_VM_AUTO_KEY_TABLE
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#if defined(_SMC777)
#define USE_SOUND_VOLUME	5
#else
#define USE_SOUND_VOLUME	4
#endif
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

static const int vm_auto_key_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	{0xa1,	0x300 | 0xbf},	// '｡' -> '/'
	{0xa2,	0x300 | 0xba},	// '｢' -> ':'
	{0xa3,	0x300 | 0xdd},	// '｣' -> ']'
	{0xa4,	0x300 | 0xbe},	// '､' -> '.'
	{0xa5,	0x300 | 0xe2},	// '･' -> '_'
	{0xa6,	0x200 | 0xbf},	// 'ｦ' -> '/'
	{0xa7,	0x300 | 0x31},	// 'ｧ' -> '1'
	{0xa8,	0x300 | 0x32},	// 'ｨ' -> '2'
	{0xa9,	0x300 | 0x33},	// 'ｩ' -> '3'
	{0xaa,	0x300 | 0x34},	// 'ｪ' -> '4'
	{0xab,	0x300 | 0x35},	// 'ｫ' -> '5'
	{0xac,	0x300 | 0x4e},	// 'ｬ' -> 'N'
	{0xad,	0x300 | 0x4d},	// 'ｭ' -> 'M'
	{0xae,	0x300 | 0xbc},	// 'ｮ' -> ','
	{0xaf,	0x300 | 0x43},	// 'ｯ' -> 'C'
	{0xb0,	0x300 | 0xdb},	// 'ｰ' -> '['
	{0xb1,	0x200 | 0x31},	// 'ｱ' -> '1'
	{0xb2,	0x200 | 0x32},	// 'ｲ' -> '2'
	{0xb3,	0x200 | 0x33},	// 'ｳ' -> '3'
	{0xb4,	0x200 | 0x34},	// 'ｴ' -> '4'
	{0xb5,	0x200 | 0x35},	// 'ｵ' -> '5'
	{0xb6,	0x200 | 0x51},	// 'ｶ' -> 'Q'
	{0xb7,	0x200 | 0x57},	// 'ｷ' -> 'W'
	{0xb8,	0x200 | 0x45},	// 'ｸ' -> 'E'
	{0xb9,	0x200 | 0x52},	// 'ｹ' -> 'R'
	{0xba,	0x200 | 0x54},	// 'ｺ' -> 'T'
	{0xbb,	0x200 | 0x41},	// 'ｻ' -> 'A'
	{0xbc,	0x200 | 0x53},	// 'ｼ' -> 'S'
	{0xbd,	0x200 | 0x44},	// 'ｽ' -> 'D'
	{0xbe,	0x200 | 0x46},	// 'ｾ' -> 'F'
	{0xbf,	0x200 | 0x47},	// 'ｿ' -> 'G'
	{0xc0,	0x200 | 0x5a},	// 'ﾀ' -> 'Z'
	{0xc1,	0x200 | 0x58},	// 'ﾁ' -> 'X'
	{0xc2,	0x200 | 0x43},	// 'ﾂ' -> 'C'
	{0xc3,	0x200 | 0x56},	// 'ﾃ' -> 'V'
	{0xc4,	0x200 | 0x42},	// 'ﾄ' -> 'B'
	{0xc5,	0x200 | 0x36},	// 'ﾅ' -> '6'
	{0xc6,	0x200 | 0x37},	// 'ﾆ' -> '7'
	{0xc7,	0x200 | 0x38},	// 'ﾇ' -> '8'
	{0xc8,	0x200 | 0x39},	// 'ﾈ' -> '9'
	{0xc9,	0x200 | 0x30},	// 'ﾉ' -> '0'
	{0xca,	0x200 | 0x59},	// 'ﾊ' -> 'Y'
	{0xcb,	0x200 | 0x55},	// 'ﾋ' -> 'U'
	{0xcc,	0x200 | 0x49},	// 'ﾌ' -> 'I'
	{0xcd,	0x200 | 0x4f},	// 'ﾍ' -> 'O'
	{0xce,	0x200 | 0x50},	// 'ﾎ' -> 'P'
	{0xcf,	0x200 | 0x48},	// 'ﾏ' -> 'H'
	{0xd0,	0x200 | 0x4a},	// 'ﾐ' -> 'J'
	{0xd1,	0x200 | 0x4b},	// 'ﾑ' -> 'K'
	{0xd2,	0x200 | 0x4c},	// 'ﾒ' -> 'L'
	{0xd3,	0x200 | 0xbb},	// 'ﾓ' -> ';'
	{0xd4,	0x200 | 0x4e},	// 'ﾔ' -> 'N'
	{0xd5,	0x200 | 0x4d},	// 'ﾕ' -> 'M'
	{0xd6,	0x200 | 0xbc},	// 'ﾖ' -> ','
	{0xd7,	0x200 | 0xbd},	// 'ﾗ' -> '-'
	{0xd8,	0x200 | 0xde},	// 'ﾘ' -> '^'
	{0xd9,	0x200 | 0xdc},	// 'ﾙ' -> '\'
	{0xda,	0x200 | 0xc0},	// 'ﾚ' -> '@'
	{0xdb,	0x200 | 0xdb},	// 'ﾛ' -> '['
	{0xdc,	0x200 | 0xbe},	// 'ﾜ' -> '.'
	{0xdd,	0x200 | 0xe2},	// 'ﾝ' -> '_'
	{0xde,	0x200 | 0xba},	// 'ﾞ' -> ':'
	{0xdf,	0x200 | 0xdd},	// 'ﾟ' -> ']'
	{-1,	-1},
};

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(_SMC777)
	_T("PSG"),
#endif
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class HD46505;
class MB8877;
#if defined(_SMC70)
class MSM58321;
#endif
class PCM1BIT;
#if defined(_SMC777)
class SN76489AN;
#endif
class Z80;

namespace SMC777 {
	class MEMORY;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;

	// devices
	//EVENT* event;

	DATAREC* drec;
	HD46505* crtc;
	MB8877* fdc;
#if defined(_SMC70)
	MSM58321* rtc;
#endif
	PCM1BIT* pcm;
#if defined(_SMC777)
	SN76489AN* psg;
#endif
	Z80* cpu;

	SMC777::MEMORY* memory;

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
	void special_reset(int num);
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
