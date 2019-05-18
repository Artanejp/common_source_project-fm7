/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ virtual machine ]
*/

#ifndef _CEFUCOM21_H_
#define _CEFUCOM21_H_

#define DEVICE_NAME		"Hino Electronics CEFUCOM-21"
#define CONFIG_NAME		"cefucom21"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		4000000
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		192

#define MC6847_ATTR_OFS		0x800
#define MC6847_ATTR_INV		0x01
#define MC6847_ATTR_AS		0x02
#define MC6847_ATTR_CSS		0x04
#define HAS_AY_3_8910
#define MEMORY_BANK_SIZE	0x400
#define IO_ADDR_MAX		0x100

// device informations for win32
#define USE_TAPE		1
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#define USE_AUTO_KEY_CAPS
#define USE_SOUND_VOLUME	3
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

static const uint8_t key_map[10][8] = {
	{0x31, 0x57, 0x53, 0x58, 0x26, 0x2e, 0xba, 0x00},
	{0x1b, 0x51, 0x41, 0x5a, 0x28, 0x0d, 0xbb, 0xbf},
	{0x33, 0x52, 0x46, 0x56, 0x25, 0xde, 0xdb, 0x00},
	{0x32, 0x45, 0x44, 0x43, 0x27, 0xdc, 0xdd, 0x20},
	{0x35, 0x59, 0x48, 0x4e, 0x72, 0x30, 0x50, 0x00},
	{0x34, 0x54, 0x47, 0x42, 0x73, 0xbd, 0xc0, 0x00},
	{0x36, 0x55, 0x4a, 0x4d, 0x71, 0x39, 0x4f, 0x00},
	{0x37, 0x49, 0x4b, 0xbc, 0x70, 0x38, 0x4c, 0xbe},
	{0x00, 0x12, 0x10, 0x11, 0x00, 0xf0, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("CMT (Signal)"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class AY_3_891X;
class DATAREC;
class I8255;
class IO;
class MC6847;
class MEMORY;
class NOT;
class RP5C01;
class Z80;
class Z80CTC;
class Z80PIO;

class MCU;
class PCU;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	AY_3_891X *mcu_psg;
	DATAREC *mcu_drec;
	IO *mcu_io;
	MC6847 *mcu_vdp;
	MEMORY *mcu_mem;
	NOT *mcu_not;
	Z80 *mcu_cpu;
	Z80PIO *mcu_pio;
	
	MCU *mcu;
	
	I8255 *pcu_pio1;
	I8255 *pcu_pio2;
	I8255 *pcu_pio3;
	IO *pcu_io;
	MEMORY *pcu_mem;
	RP5C01 *pcu_rtc;
	Z80 *pcu_cpu;
	Z80CTC *pcu_ctc1;
	Z80CTC *pcu_ctc2;
	Z80PIO *pcu_pio;
	
	PCU *pcu;
	
	uint8_t mcu_rom[0x8000];
	uint8_t mcu_ram[0x8000];
	uint8_t pcu_rom[0x8000];
	uint8_t pcu_ram[0x8000];
	uint8_t vram[0x1800];
	uint8_t cram[0x1000];
	
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
