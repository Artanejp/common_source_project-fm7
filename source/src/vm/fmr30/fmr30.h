/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ virtual machine ]
*/

#ifndef _FMR30_H_
#define _FMR30_H_

#if defined(HAS_I86)
#define DEVICE_NAME		"FUJITSU FMR-30 (i86)"
#define CONFIG_NAME		"fmr30_i86"
#elif defined(HAS_I286)
#define DEVICE_NAME		"FUJITSU FMR-30 (i286)"
#define CONFIG_NAME		"fmr30_i286"
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define MAX_DRIVE		2
#define MAX_MEMCARD		2
#define I86_PSEUDO_BIOS
#define SINGLE_MODE_DMA
#define HAS_MB89311
#define MB8877_NO_BUSY_AFTER_SEEK
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_FLOPPY_DISK		2
#define USE_FLOPPY_TYPE_BIT 0x0003 /* 3.5, 3.5 */
#define USE_HARD_DISK		7
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE
#if defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88)
#define USE_CPU_I86
#else
#define USE_CPU_I286
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Noise (FDD)"),
};
#endif
class EMU;
class DEVICE;
class EVENT;

class I8237;
class I8251;
class I8253;
class I8259;
#if defined(HAS_I86)
class I86;
#elif defined(HAS_I286)
class I286;
#endif
class IO;
class MB8877;
class SCSI_HDD;
class SCSI_HOST;
class SN76489AN;

namespace FMR50 {
	class BIOS;
}
namespace FMR30 {
	class CMOS;
	class FLOPPY;
	class KEYBOARD;
	class MEMBUS;
	class RTC;
	class SCSI;
	class SERIAL;
	class SYSTEM;
	class TIMER;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils *state_entry;
	// devices
	//EVENT* event;

	I8237* dma;
	I8251* sio_kb;
	I8251* sio_sub;
	I8251* sio_ch1;
	I8251* sio_ch2;
	I8253* pit;
	I8259* pic;
#if defined(HAS_I86)
	I86* cpu;
#elif defined(HAS_I286)
	I286* cpu;
#endif
	IO* io;
	MB8877* fdc;
	SCSI_HDD* scsi_hdd[7];
	SCSI_HOST* scsi_host;
	SN76489AN* psg;

	FMR50::BIOS* bios;
	FMR30::CMOS* cmos;
	FMR30::FLOPPY* floppy;
	FMR30::KEYBOARD* keyboard;
	FMR30::MEMBUS* memory;
	FMR30::RTC* rtc;
	FMR30::SCSI* scsi;
	FMR30::SERIAL* serial;
	FMR30::SYSTEM* system;
	FMR30::TIMER* timer;

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

	// notify key
	void key_down(int code, bool repeat) override;
	void key_up(int code) override;

	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
	void open_hard_disk(int drv, const _TCHAR* file_path) override;
	void close_hard_disk(int drv) override;
	bool is_hard_disk_inserted(int drv) override;
	uint32_t is_hard_disk_accessed() override;
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
