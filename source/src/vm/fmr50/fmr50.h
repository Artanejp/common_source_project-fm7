/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.28 -

	[ virtual machine ]
*/

#ifndef _FMR50_H_
#define _FMR50_H_

#if defined(_FMR50)
	#if defined(HAS_I286)
		#define DEVICE_NAME		"FUJITSU FMR-50 (i286)"
		#define CONFIG_NAME		"fmr50_i286"
	#elif defined(HAS_I386)
		#define DEVICE_NAME		"FUJITSU FMR-50 (i386)"
		#define CONFIG_NAME		"fmr50_i386"
	#elif defined(HAS_I486)
		#define DEVICE_NAME		"FUJITSU FMR-50 (i486)"
		#define CONFIG_NAME		"fmr50_i486"
	#elif defined(HAS_PENTIUM)
		#define DEVICE_NAME		"FUJITSU FMR-250"
		#define CONFIG_NAME		"fmr250"
	#endif
#elif defined(_FMR60)
	#if defined(HAS_I286)
		#define DEVICE_NAME		"FUJITSU FMR-60"
		#define CONFIG_NAME		"fmr60"
	#elif defined(HAS_I386)
		#define DEVICE_NAME		"FUJITSU FMR-70"
		#define CONFIG_NAME		"fmr70"
	#elif defined(HAS_I486)
		#define DEVICE_NAME		"FUJITSU FMR-80"
		#define CONFIG_NAME		"fmr80"
	#elif defined(HAS_PENTIUM)
		#define DEVICE_NAME		"FUJITSU FMR-280"
		#define CONFIG_NAME		"fmr280"
	#endif
#endif
#if defined(HAS_I486)
	#if !(defined(HAS_I486SX) || defined(HAS_I486DX))
		#define HAS_I486SX
//		#define HAS_I486DX
	#endif
#else
	#if (defined(HAS_I486SX) || defined(HAS_I486DX))
		#define HAS_I486
	#endif
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#if defined(_FMR60)
#define LINES_PER_FRAME 	784
#define CHARS_PER_LINE		98
#else
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		54
#endif
//#define CPU_CLOCKS		12000000
#define CPU_CLOCKS		8000000
#if defined(_FMR60)
#define SCREEN_WIDTH		1120
#define SCREEN_HEIGHT		750
#define WINDOW_HEIGHT_ASPECT	840
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#endif
#define MAX_DRIVE		4
#define MAX_MEMCARD		2
#define I86_PSEUDO_BIOS
#define SINGLE_MODE_DMA
#define MB8877_NO_BUSY_AFTER_SEEK
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_CPU_TYPE		2
#define USE_FLOPPY_DISK		4
#define USE_FLOPPY_TYPE_BIT 0x0003 /* 3.5, 3.5, 5.0, 5.0 */

#define USE_HARD_DISK		7
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE
#if defined(HAS_I286)
#define USE_CPU_I286
#else
#define USE_CPU_I386
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("Noise (FDD)"),
};
#endif
class EMU;
class DEVICE;
class EVENT;

class HD46505;
#ifdef _FMR60
class HD63484;
#endif
class I8251;
class I8253;
class I8259;
#if defined(HAS_I286)
class I286;
#else
class I386;
#endif
class IO;
class MB8877;
class MSM58321;
class PCM1BIT;
class SCSI_HDD;
class SCSI_HOST;
class UPD71071;

namespace FMR50 {
	class BIOS;
	class CMOS;
	class FLOPPY;
	class KEYBOARD;
	class MEMORY;
	//class SERIAL;
	class SCSI;
	class TIMER;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils *state_entry;

	// devices
	//EVENT* event;

	HD46505* crtc;
#if defined(_FMR60)
	HD63484* acrtc;
#endif
	I8251* sio;
	I8253* pit0;
	I8253* pit1;
	I8259* pic;
#if defined(HAS_I286)
	I286* cpu;
#else
	I386* cpu;
#endif
	IO* io;
	MB8877* fdc;
	MSM58321* rtc;
	PCM1BIT* pcm;
	SCSI_HDD* scsi_hdd[7];
	SCSI_HOST* scsi_host;
	UPD71071* dma;

	FMR50::BIOS* bios;
	FMR50::CMOS* cmos;
	FMR50::FLOPPY* floppy;
	FMR50::KEYBOARD* keyboard;
	FMR50::MEMORY* memory;
	FMR50::SCSI* scsi;
//	FMR50::SERIAL* serial;
	FMR50::TIMER* timer;

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

	//void update_config();
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	//DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
