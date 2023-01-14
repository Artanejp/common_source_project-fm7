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
#define USE_SOUND_VOLUME	5
#elif defined(_MZ800)
#define USE_SOUND_VOLUME	6
#elif defined(_MZ1500)
#define USE_SOUND_VOLUME	7
#endif
#if defined(_MZ1500)
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#endif
#define USE_DEBUGGER
#define USE_STATE

#if defined(_MZ700) || defined(_MZ1500)
static const int vm_auto_key_table_base[][2] = {
	// thanks Mr.Koucha Youkan
	{0xa1,	0x300 | 0xbe},	// '¡'    *** MODIFIED ***
	{0xa2,	0x300 | 0x38},	// '¢'    *** MODIFIED ***
	{0xa3,	0x300 | 0x39},	// '£'    *** MODIFIED ***
	{0xa4,	0x300 | 0xbc},	// '¤'    *** MODIFIED ***
	{0xa5,	0x300 | 0x30},	// '¥'    *** MODIFIED ***
	{0xa6,	0x200 | 0xdb},	// '¦'    *** MODIFIED ***
	{0xa7,	0x300 | 0x31},	// '§'
	{0xa8,	0x300 | 0x32},	// '¨'
	{0xa9,	0x300 | 0x33},	// '©'
	{0xaa,	0x300 | 0x34},	// 'ª'
	{0xab,	0x300 | 0x35},	// '«'
	{0xac,	0x300 | 0xbd},	// '¬'    *** MODIFIED ***
	{0xad,	0x300 | 0xde},	// '­'    *** MODIFIED ***
	{0xae,	0x300 | 0xdc},	// '®'    *** MODIFIED ***
	{0xaf,	0x300 | 0x43},	// '¯'
	{0xb0,	0x200 | 0xe2},	// '°'    *** MODIFIED ***
	{0xb1,	0x200 | 0x31},	// '±'
	{0xb2,	0x200 | 0x32},	// '²'
	{0xb3,	0x200 | 0x33},	// '³'
	{0xb4,	0x200 | 0x34},	// '´'
	{0xb5,	0x200 | 0x35},	// 'µ'
	{0xb6,	0x200 | 0x51},	// '¶'
	{0xb7,	0x200 | 0x57},	// '·'
	{0xb8,	0x200 | 0x45},	// '¸'
	{0xb9,	0x200 | 0x52},	// '¹'
	{0xba,	0x200 | 0x54},	// 'º'
	{0xbb,	0x200 | 0x41},	// '»'
	{0xbc,	0x200 | 0x53},	// '¼'
	{0xbd,	0x200 | 0x44},	// '½'
	{0xbe,	0x200 | 0x46},	// '¾'
	{0xbf,	0x200 | 0x47},	// '¿'
	{0xc0,	0x200 | 0x5a},	// 'À'
	{0xc1,	0x200 | 0x58},	// 'Á'
	{0xc2,	0x200 | 0x43},	// 'Â'
	{0xc3,	0x200 | 0x56},	// 'Ã'
	{0xc4,	0x200 | 0x42},	// 'Ä'
	{0xc5,	0x200 | 0x36},	// 'Å'
	{0xc6,	0x200 | 0x37},	// 'Æ'
	{0xc7,	0x200 | 0x38},	// 'Ç'
	{0xc8,	0x200 | 0x39},	// 'È'
	{0xc9,	0x200 | 0x30},	// 'É'
	{0xca,	0x200 | 0x59},	// 'Ê'
	{0xcb,	0x200 | 0x55},	// 'Ë'
	{0xcc,	0x200 | 0x49},	// 'Ì'
	{0xcd,	0x200 | 0x4f},	// 'Í'
	{0xce,	0x200 | 0x50},	// 'Î'
	{0xcf,	0x200 | 0x48},	// 'Ï'
	{0xd0,	0x200 | 0x4a},	// 'Ð'
	{0xd1,	0x200 | 0x4b},	// 'Ñ'
	{0xd2,	0x200 | 0x4c},	// 'Ò'
	{0xd3,	0x200 | 0xbb},	// 'Ó'
	{0xd4,	0x200 | 0xbd},	// 'Ô'    *** MODIFIED ***
	{0xd5,	0x200 | 0xde},	// 'Õ'    *** MODIFIED ***
	{0xd6,	0x200 | 0xdc},	// 'Ö'    *** MODIFIED ***
	{0xd7,	0x200 | 0x4e},	// '×'    *** MODIFIED ***
	{0xd8,	0x200 | 0x4d},	// 'Ø'    *** MODIFIED ***
	{0xd9,	0x200 | 0xbc},	// 'Ù'    *** MODIFIED ***
	{0xda,	0x200 | 0xbe},	// 'Ú'    *** MODIFIED ***
	{0xdb,	0x200 | 0xbf},	// 'Û'    *** MODIFIED ***
	{0xdc,	0x200 | 0xc0},	// 'Ü'    *** MODIFIED ***
	{0xdd,	0x200 | 0x78},	// 'Ý'    *** MODIFIED ***
	{0xde,	0x200 | 0xba},	// 'Þ'    *** MODIFIED ***
	{0xdf,	0x200 | 0xdd},	// 'ß'    *** MODIFIED ***
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
	_T("Beep"), _T("CMT (Signal)"), _T("Noise (CMT)"), _T("Noise (FDD)"), _T("Noise (QD)"),
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

class CMOS;
class EMM;
class FLOPPY;
class KANJI;
class KEYBOARD;
class MEMORY;
class QUICKDISK;
class RAMFILE;

#if defined(_MZ800) || defined(_MZ1500)
class NOT;
class SN76489AN;
class Z80PIO;
#if defined(_MZ1500)
class PSG;
#endif
#endif
#if defined(_MZ700) || defined(_MZ1500)
class JOYSTICK;
#endif

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and_int;
	DATAREC* drec;
	I8253* pit;
	I8255* pio;
	IO* io;
	MB8877* fdc;
	PCM1BIT* pcm;
	Z80* cpu;
	Z80SIO* sio_qd;	// QD
	
	CMOS* cmos;
	EMM* emm;
	FLOPPY* floppy;
	KANJI* kanji;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RAMFILE* ramfile;
	QUICKDISK* qd;
	
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
	PSG* psg;
#endif
#endif
#if defined(_MZ700) || defined(_MZ1500)
	JOYSTICK* joystick;
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
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
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
	
	// user interface
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
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
	uint32_t is_quick_disk_accessed();
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
	bool is_frame_skippable();
	
	void update_config();
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
