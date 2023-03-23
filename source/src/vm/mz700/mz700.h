/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ virtual machine ]
*/

#ifndef _MZ700_H_
#define _MZ700_H_

#if defined(_MZ700)
#if defined(_PAL)
#define DEVICE_NAME		"SHARP MZ-700 (PAL)"
#define CONFIG_NAME		"mz700pal"
#else
#define DEVICE_NAME		"SHARP MZ-700 (NTSC)"
#define CONFIG_NAME		"mz700"
#endif
#elif defined(_MZ800)
#define DEVICE_NAME		"SHARP MZ-800"
#define CONFIG_NAME		"mz800"
#define _PAL
#elif defined(_MZ1500)
#define DEVICE_NAME		"SHARP MZ-1500"
#define CONFIG_NAME		"mz1500"
#endif

// device informations for virtual machine
#if defined(_PAL)
#define CPU_CLOCKS		3546895
//#define PHI_CLOCKS		(CPU_CLOCKS * 5)
#define PHI_CLOCKS		(CPU_CLOCKS * 1136.0 / 228.0)
#define LINES_PER_FRAME		312
#define FRAMES_PER_SEC		(PHI_CLOCKS / 1136.0 / LINES_PER_FRAME)
#else
#define CPU_CLOCKS		3579545
#define PHI_CLOCKS		(CPU_CLOCKS * 4.0)
#define LINES_PER_FRAME		262
#define FRAMES_PER_SEC		(PHI_CLOCKS / 912.0 / LINES_PER_FRAME)
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define HAS_MB8876
#if defined(_MZ1500)
#define MZ1P17_SW1_4_ON
#endif

// device informations for win32
#if defined(_MZ700)
#define USE_DIPSWITCH
#elif defined(_MZ800)
#define USE_BOOT_MODE		2
#endif
#define USE_TAPE		1
#define USE_FLOPPY_DISK		2
#define USE_QUICK_DISK		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#if defined(_MZ700) || defined(_MZ1500)
#define USE_AUTO_KEY_NUMPAD
#define USE_VM_AUTO_KEY_TABLE
#define USE_JOYSTICK
#define USE_JOYSTICK_TYPE	3
#define USE_JOY_BUTTON_CAPTIONS
#endif
#if defined(_MZ800)
#define USE_MONITOR_TYPE	2
#endif
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#if defined(_MZ700)
#define USE_SOUND_VOLUME	4
#elif defined(_MZ800)
#define USE_SOUND_VOLUME	5
#elif defined(_MZ1500)
#define USE_SOUND_VOLUME	6
#endif
#if defined(_MZ1500)
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#endif
#define SUPPORT_TV_RENDER
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80

#if defined(_MZ700) || defined(_MZ1500)
static const int vm_auto_key_table_base[][2] = {
	// thanks Mr.Koucha Youkan
	{0xa1,	0x300 | 0xbe},	// '｡'    *** MODIFIED ***
	{0xa2,	0x300 | 0x38},	// '｢'    *** MODIFIED ***
	{0xa3,	0x300 | 0x39},	// '｣'    *** MODIFIED ***
	{0xa4,	0x300 | 0xbc},	// '､'    *** MODIFIED ***
	{0xa5,	0x300 | 0x30},	// '･'    *** MODIFIED ***
	{0xa6,	0x200 | 0xdb},	// 'ｦ'    *** MODIFIED ***
	{0xa7,	0x300 | 0x31},	// 'ｧ'
	{0xa8,	0x300 | 0x32},	// 'ｨ'
	{0xa9,	0x300 | 0x33},	// 'ｩ'
	{0xaa,	0x300 | 0x34},	// 'ｪ'
	{0xab,	0x300 | 0x35},	// 'ｫ'
	{0xac,	0x300 | 0xbd},	// 'ｬ'    *** MODIFIED ***
	{0xad,	0x300 | 0xde},	// 'ｭ'    *** MODIFIED ***
	{0xae,	0x300 | 0xdc},	// 'ｮ'    *** MODIFIED ***
	{0xaf,	0x300 | 0x43},	// 'ｯ'
	{0xb0,	0x200 | 0xe2},	// 'ｰ'    *** MODIFIED ***
	{0xb1,	0x200 | 0x31},	// 'ｱ'
	{0xb2,	0x200 | 0x32},	// 'ｲ'
	{0xb3,	0x200 | 0x33},	// 'ｳ'
	{0xb4,	0x200 | 0x34},	// 'ｴ'
	{0xb5,	0x200 | 0x35},	// 'ｵ'
	{0xb6,	0x200 | 0x51},	// 'ｶ'
	{0xb7,	0x200 | 0x57},	// 'ｷ'
	{0xb8,	0x200 | 0x45},	// 'ｸ'
	{0xb9,	0x200 | 0x52},	// 'ｹ'
	{0xba,	0x200 | 0x54},	// 'ｺ'
	{0xbb,	0x200 | 0x41},	// 'ｻ'
	{0xbc,	0x200 | 0x53},	// 'ｼ'
	{0xbd,	0x200 | 0x44},	// 'ｽ'
	{0xbe,	0x200 | 0x46},	// 'ｾ'
	{0xbf,	0x200 | 0x47},	// 'ｿ'
	{0xc0,	0x200 | 0x5a},	// 'ﾀ'
	{0xc1,	0x200 | 0x58},	// 'ﾁ'
	{0xc2,	0x200 | 0x43},	// 'ﾂ'
	{0xc3,	0x200 | 0x56},	// 'ﾃ'
	{0xc4,	0x200 | 0x42},	// 'ﾄ'
	{0xc5,	0x200 | 0x36},	// 'ﾅ'
	{0xc6,	0x200 | 0x37},	// 'ﾆ'
	{0xc7,	0x200 | 0x38},	// 'ﾇ'
	{0xc8,	0x200 | 0x39},	// 'ﾈ'
	{0xc9,	0x200 | 0x30},	// 'ﾉ'
	{0xca,	0x200 | 0x59},	// 'ﾊ'
	{0xcb,	0x200 | 0x55},	// 'ﾋ'
	{0xcc,	0x200 | 0x49},	// 'ﾌ'
	{0xcd,	0x200 | 0x4f},	// 'ﾍ'
	{0xce,	0x200 | 0x50},	// 'ﾎ'
	{0xcf,	0x200 | 0x48},	// 'ﾏ'
	{0xd0,	0x200 | 0x4a},	// 'ﾐ'
	{0xd1,	0x200 | 0x4b},	// 'ﾑ'
	{0xd2,	0x200 | 0x4c},	// 'ﾒ'
	{0xd3,	0x200 | 0xbb},	// 'ﾓ'
	{0xd4,	0x200 | 0xbd},	// 'ﾔ'    *** MODIFIED ***
	{0xd5,	0x200 | 0xde},	// 'ﾕ'    *** MODIFIED ***
	{0xd6,	0x200 | 0xdc},	// 'ﾖ'    *** MODIFIED ***
	{0xd7,	0x200 | 0x4e},	// 'ﾗ'    *** MODIFIED ***
	{0xd8,	0x200 | 0x4d},	// 'ﾘ'    *** MODIFIED ***
	{0xd9,	0x200 | 0xbc},	// 'ﾙ'    *** MODIFIED ***
	{0xda,	0x200 | 0xbe},	// 'ﾚ'    *** MODIFIED ***
	{0xdb,	0x200 | 0xbf},	// 'ﾛ'    *** MODIFIED ***
	{0xdc,	0x200 | 0xc0},	// 'ﾜ'    *** MODIFIED ***
	{0xdd,	0x200 | 0x78},	// 'ﾝ'    *** MODIFIED ***
	{0xde,	0x200 | 0xba},	// 'ﾞ'    *** MODIFIED ***
	{0xdf,	0x200 | 0xdd},	// 'ﾟ'    *** MODIFIED ***
	{-1,	-1},
};
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(_MZ800)
	_T("PSG"),
#elif defined(_MZ1500)
	_T("PSG #1"), _T("PSG #2"),
#endif
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (CMT)"), _T("Noise (FDD)"),
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
	_T("Run"),
	_T("Select"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class AND;
class DATAREC;
class I8253;
class I8255;
class IO;
class MB8877;
class PCM1BIT;
class Z80;
class Z80SIO;

namespace MZ700 {
	class CMOS;
	class EMM;
	class FLOPPY;
#if defined(_MZ700) || defined(_MZ1500)
	class JOYSTICK;
#endif
	class KANJI;
	class KEYBOARD;
	class MEMORY;
#if defined(_MZ1500)
	class PSG;
#endif
	class QUICKDISK;
	class RAMFILE;
}

#if defined(_MZ800) || defined(_MZ1500)
class NOT;
class SN76489AN;
class Z80PIO;
#endif

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;

	// devices
	//EVENT* event;

	AND* and_int;
	DATAREC* drec;
	MB8877* fdc;
	I8253* pit;
	I8255* pio;
	IO* io;
	PCM1BIT* pcm;
	Z80SIO* sio_qd;	// QD
	Z80* cpu;

	MZ700::CMOS* cmos;
	MZ700::EMM* emm;
	MZ700::FLOPPY* floppy;
	MZ700::KANJI* kanji;
	MZ700::KEYBOARD* keyboard;
	MZ700::MEMORY* memory;
	MZ700::QUICKDISK* qd;
	MZ700::RAMFILE* ramfile;

#if defined(_MZ800) || defined(_MZ1500)
	AND* and_snd;
#if defined(_MZ800)
	NOT* not_pit;
	SN76489AN* psg;
#elif defined(_MZ1500)
	DEVICE* printer;
	NOT* not_reset;
	NOT* not_strobe;
	SN76489AN* psg_l;
	SN76489AN* psg_r;
#endif
	Z80PIO* pio_int;
	Z80SIO* sio_rs;	// RS-232C

#if defined(_MZ1500)
	MZ700::PSG* psg;
#endif
#endif
#if defined(_MZ700) || defined(_MZ1500)
	MZ700::JOYSTICK* joystick;
#endif


#if defined(_MZ700)
	int dipswitch;
#elif defined(_MZ800)
	int boot_mode;
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
	void reset() override;
	void run() override;
	double get_frame_rate() override
	{
		return FRAMES_PER_SEC;
	}
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index) override;
#endif
	// draw screen
	void draw_screen() override;

	// sound generation
	void initialize_sound(int rate, int samples) override;
	uint16_t* create_sound(int* extra_frames) override;
	int get_sound_buffer_ptr() override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r) override;
#endif

	// user interface
	void play_tape(int drv, const _TCHAR* file_path) override;
	void rec_tape(int drv, const _TCHAR* file_path) override;
	void close_tape(int drv) override;
	bool is_tape_inserted(int drv) override;
	bool is_tape_playing(int drv) override;
	bool is_tape_recording(int drv) override;
	int get_tape_position(int drv) override;
	const _TCHAR* get_tape_message(int drv) override;
	void push_play(int drv) override;
	void push_stop(int drv) override;
	void push_fast_forward(int drv) override;
	void push_fast_rewind(int drv) override;
	void push_apss_forward(int drv)  override {}
	void push_apss_rewind(int drv)  override {}
	void open_quick_disk(int drv, const _TCHAR* file_path) override;
	void close_quick_disk(int drv) override;
	bool is_quick_disk_inserted(int drv) override;
	uint32_t is_quick_disk_accessed() override;
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
	bool is_frame_skippable() override;

	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

	void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
