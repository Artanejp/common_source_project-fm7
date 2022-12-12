/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ virtual machine ]
*/

#ifndef _MZ2800_H_
#define _MZ2800_H_

#define DEVICE_NAME		"SHARP MZ-2800"
#define CONFIG_NAME		"mz2800"

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4
#define LINES_PER_FRAME 	440
#define CHARS_PER_LINE		108
#define CPU_CLOCKS		8000000
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define MAX_DRIVE		4
#define SINGLE_MODE_DMA
#define HAS_RP5C15
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_FLOPPY_DISK		4
#define USE_HARD_DISK		4
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_NUMPAD
#define USE_SCREEN_FILTER
#define USE_SOUND_VOLUME	4
#define USE_JOYSTICK
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
	_T("OPN (FM)"), _T("OPN (PSG)"), _T("Beep"), _T("Noise (FDD)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8253;
class I8255;
class I8259;
class I286;
class IO;
class MB8877;
class NOT;
class PCM1BIT;
class RP5C01;
class SASI_HDD;
class SCSI_HOST;
class UPD71071;
class YM2203;
class Z80PIO;
class Z80SIO;

class CRTC;
class FLOPPY;
class JOYSTICK;
class KEYBOARD;
class MEMORY;
class MOUSE;
class PRINTER;
class RESET;
class SASI;
class SERIAL;
class SYSPORT;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	I8253* pit;
	I8255* pio0;
	I8259* pic;
	I286* cpu;
	IO* io;
	MB8877* fdc;
	NOT* not_busy;
	PCM1BIT* pcm;
	RP5C01* rtc;
	SASI_HDD* sasi_hdd[(USE_HARD_DISK >> 1) + (USE_HARD_DISK & 1)];
	SCSI_HOST* sasi_host;
	UPD71071* dma;
	YM2203* opn;
	Z80PIO* pio1;
	Z80SIO* sio;
	
	CRTC* crtc;
	FLOPPY* floppy;
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY* memory;
	MOUSE* mouse;
	PRINTER* printer;
	RESET* rst;
	SASI* sasi;
	SERIAL* serial;
	SYSPORT* sysport;
	
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
	void cpu_reset();
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
