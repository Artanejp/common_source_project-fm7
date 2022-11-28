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
#define HAS_I8254
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define HAS_MB89311
#define MB8877_NO_BUSY_AFTER_SEEK
#define IO_ADDR_MAX		0x10000
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_FLOPPY_DISK		2
#define USE_HARD_DISK		7
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SOUND_VOLUME	2
#define USE_DEBUGGER
#define USE_STATE

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

class BIOS;
class CMOS;
class FLOPPY;
class KEYBOARD;
class MEMORY;
class RTC;
class SCSI;
class SERIAL;
class SYSTEM;
class TIMER;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
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
	
	BIOS* bios;
	CMOS* cmos;
	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RTC* rtc;
	SCSI* scsi;
	SERIAL* serial;
	SYSTEM* system;
	TIMER* timer;
	
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
