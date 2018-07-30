/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.28 -

	[ virtual machine ]
*/

#ifndef _FMTOWNS_H_
#define _FMTOWNS_H_

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
#define MAX_SCSI		8
#define MAX_MEMCARD		2
#if defined(HAS_I286)
#define I86_PSEUDO_BIOS
#else
#define I386_PSEUDO_BIOS
#endif
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define MB8877_NO_BUSY_AFTER_SEEK
#define IO_ADDR_MAX		0x10000
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_CPU_TYPE		2
#define USE_FLOPPY_DISK     4 // ??
#define NOTIFY_KEY_DOWN
#define USE_SHIFT_NUMPAD_KEY
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SOUND_FILES		1
#define USE_SOUND_FILES_FDD
#if defined(USE_SOUND_FILES)
#define USE_SOUND_VOLUME	5
#else
#define USE_SOUND_VOLUME	6
#endif
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_I386

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("FM SSG"), _T("FM OPNB"), _T("PCM"), _T("CD-DA"),
#if defined(USE_SOUND_FILES)
	_T("FDD SEEK"),
#endif
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8253;
class I8259;
class I386;

class IO;
class TOWNS_CRTC;
class TOWNS_VRAM;
class TOWNS_SPRITE;
class TOWNS_JOYSTICK; // Mouse and Joystick.

class RF5C68;      // PCM
class YM2612;      // OPNB
class MB87078;     // VOLUME
//class AD7820KR;  // A/D Converter.
class MB8877;      // FDC
class MSM58321;    // RTC
class PCM1BIT;     // BUZZER
class UPD71071;    // DMAC
class TOWNS_CDROM; // CDROM Controller
class TOWNS_SERIALROM; // SERIAL ROM. May be 32 bytes.
class CMOS;
class FLOPPY;
class KEYBOARD;
class MEMORY;
//class SERIAL;
class SCSI;
class SCSI_HOST;
class SCSI_HDD;
class SCSI_CDROM;
class TIMER;

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	
	// devices
	//EVENT* event;
	
	TOWNS_CRTC *crtc;
	
	I8251* sio;
	I8253* pit0;
	I8253* pit1;
	
	I8259* pic0;
	I8259* pic1;
	
	I386* cpu; // i386DX/SX/486DX/486SX?/Pentium with FPU?

	IO* io;
	MB8877* fdc;
	MSM58321* rtc;
	PCM1BIT* pcm;
	SCSI_HOST* scsi_host;
	UPD71071* dma;

	
	//BIOS* bios;
	CMOS* cmos;

	FLOPPY* floppy;
	KEYBOARD* keyboard;
	MEMORY* memory;
	
	SCSI* scsi;
	SCSI_HDD *hdd[4]; // 
	SCSI_CDROM *cdrom; // 
//	SERIAL* serial;
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
	bool is_frame_skippable();
	
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
