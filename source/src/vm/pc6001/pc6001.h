/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ virtual machine ]
*/

#ifndef _PC6001_H_
#define _PC6001_H_

#if defined(_PC6001)
#define DEVICE_NAME		"NEC PC-6001"
#define CONFIG_NAME		"pc6001"
#define SUB_CPU_ROM_FILE_NAME	"SUBCPU.60"
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192
#define CPU_CLOCKS		3993600
#define HAS_AY_3_8910
#define TIMER_PERIOD	(8192. / CPU_CLOCKS * 1000000)
#elif defined(_PC6001MK2)
#define DEVICE_NAME		"NEC PC-6001mkII"
#define CONFIG_NAME		"pc6001mk2"
#define SUB_CPU_ROM_FILE_NAME	"SUBCPU.62"
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define CPU_CLOCKS		3993600
#define HAS_AY_3_8910
#define TIMER_PERIOD	(8192. / CPU_CLOCKS * 1000000)
#elif defined(_PC6001MK2SR)
#define DEVICE_NAME		"NEC PC-6001mkIISR"
#define CONFIG_NAME		"pc6001mk2sr"
#define SUB_CPU_ROM_FILE_NAME	"SUBCPU.68"
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define CPU_CLOCKS		3580000
#define TIMER_PERIOD	(2000/.999)
#elif defined(_PC6601)
#define DEVICE_NAME		"NEC PC-6601"
#define CONFIG_NAME		"pc6601"
#define SUB_CPU_ROM_FILE_NAME	"SUBCPU.66"
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define CPU_CLOCKS		4000000
#define HAS_AY_3_8910
#define TIMER_PERIOD	(8192. / CPU_CLOCKS * 1000000)
#elif defined(_PC6601SR)
#define DEVICE_NAME		"NEC PC-6601SR"
#define CONFIG_NAME		"pc6601sr"
#define SUB_CPU_1_ROM_FILE_NAME	"SUBCPU1.68"
#define SUB_CPU_2_ROM_FILE_NAME	"SUBCPU2.68"
#define SUB_CPU_3_ROM_FILE_NAME	"SUBCPU3.68"
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define CPU_CLOCKS		3580000
#define TIMER_PERIOD	(2000 / .999)
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define MAX_DRIVE		4
#define MC6847_ATTR_OFS		0
#define MC6847_VRAM_OFS		0x200
#define MC6847_ATTR_AG		0x80
#define MC6847_ATTR_AS		0x40
#define MC6847_ATTR_INTEXT	0x20
#define MC6847_ATTR_GM0		0x10
#define MC6847_ATTR_GM1		0x08
#define MC6847_ATTR_GM2		0x04
#define MC6847_ATTR_CSS		0x02
#define MC6847_ATTR_INV		0x01

// device informations for win32
#define USE_CART		1
#if defined(_PC6601) || defined(_PC6601SR)
#define USE_FLOPPY_DISK		4
#else
#define USE_FLOPPY_DISK		2
#endif
#define USE_TAPE		1
#define TAPE_PC6001
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#if !defined(_PC6001)
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#endif
#if defined(_PC6001)
#define USE_SOUND_VOLUME	4
#else
#define USE_SOUND_VOLUME	5
#endif
#define USE_JOYSTICK
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"),
#if !defined(_PC6001)
	_T("Voice"),
#endif
	_T("CMT (Signal)"), _T("Noise (FDD)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8255;
class IO;
#ifdef _PC6001
class MC6847;
#else
class UPD7752;
#endif
class NOISE;
class PC6031;
class PC80S31K;
class UPD765A;
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
class YM2203;
#else
class AY_3_891X;
#endif
class Z80;

class DATAREC;
class MCS48;

#ifdef _PC6001
class DISPLAY;
#endif
#if defined(_PC6601) || defined(_PC6601SR)
class FLOPPY;
#endif
class JOYSTICK;
class MEMORY;
//class PSUB;
class SUB;
class TIMER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	int vdata;
	
	// devices
	EVENT* event;
	
	DEVICE* printer;
	I8255* pio_sub;
	IO* io;
	NOISE* noise_seek;
	NOISE* noise_head_down;
	NOISE* noise_head_up;
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
	YM2203* psg;
#else
	AY_3_891X* psg;
#endif
	Z80* cpu;
#ifdef _PC6001
	MC6847* vdp;
	DISPLAY* display;
#else
	UPD7752* voice;
#endif
#if defined(_PC6601) || defined(_PC6601SR)
	FLOPPY* floppy;
#endif
	JOYSTICK* joystick;
	MEMORY* memory;
//	PSUB* psub;
	TIMER* timer;
	
	MCS48* cpu_sub;
	SUB* sub;
	DATAREC* drec;
	
	PC6031* pc6031;
	I8255* pio_fdd;
	I8255* pio_pc80s31k;
	PC80S31K *pc80s31k;
	UPD765A* fdc_pc80s31k;
	Z80* cpu_pc80s31k;
	
	bool support_sub_cpu;
	bool support_pc80s31k;
	
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
	
	// notify key
	void key_down(int code, bool repeat);
	void key_up(int code);
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
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
	
	int sr_mode;
};
#endif
