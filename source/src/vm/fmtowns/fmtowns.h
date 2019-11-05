/*
	FUJITSU FMR-50 Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-
	[ virtual machine ]
*/
#pragma once


#undef WITH_386SX
#undef WITH_486
#undef WITH_PENTIUM
#undef TYPE_TOWNS_X0
#undef TYPE_TOWNS2_UX
#undef TYPE_TOWNS2_CX

#if defined(_FMTOWNS_1)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 1"
#define CONFIG_NAME		"fmtowns_1"
#define MAX_DRIVE       1
#define _HAS_HDD        1

#elif defined(_FMTOWNS_2)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 2"
#define CONFIG_NAME		"fmtowns_2"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#elif defined(_FMTOWNS_2F)
#define DEVICE_NAME		"FUJITSU FM-Towns 2F"
#define CONFIG_NAME		"fmtowns_2f"
#define MAX_DRIVE       2
#undef  _HAS_HDD

#elif defined(_FMTOWNS_2H)
#define DEVICE_NAME		"FUJITSU FM-Towns 2H"
#define CONFIG_NAME		"fmtowns_2h"
#define MAX_DRIVE       2
#define _HAS_HDD        2

#elif defined(_FMTOWNS_20F)
#define DEVICE_NAME		"FUJITSU FM-Towns 20F"
#define CONFIG_NAME		"fmtowns_20f"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define TYPE_TOWNS_X0   1

#elif defined(_FMTOWNS_40H)
#define DEVICE_NAME		"FUJITSU FM-Towns 40H"
#define CONFIG_NAME		"fmtowns_20h"
#define MAX_DRIVE       2
#define _HAS_HDD        2
#define TYPE_TOWNS_X0   1

#elif defined(_FMTOWNS2_UX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX20"
#define CONFIG_NAME		"fmtowns2_ux20"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1

#elif defined(_FMTOWNS2_UX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX40"
#define CONFIG_NAME		"fmtowns2_ux20"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1

#elif defined(_FMTOWNS2_CX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX20"
#define CONFIG_NAME		"fmtowns2_cx20"
#define MAX_DRIVE       2
#undef _HAS_HDD
#define TYPE_TOWNS2_CX  1

#elif defined(_FMTOWNS2_CX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2_cx40"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1

#elif defined(_FMTOWNS2_CX100)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2_cx100"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4 // OK?
#define LINES_PER_FRAME 	784  // OK?

#define CPU_CLOCKS		16000000

#if defined(_FMR60)
#define SCREEN_WIDTH		1024
#define SCREEN_HEIGHT		768
#define WINDOW_HEIGHT_ASPECT	840
#else
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#endif

#if defined(_HAS_HDD)
#define MAX_SCSI		8
#endif

#define MAX_MEMCARD		2
#define I8259_MAX_CHIPS		2

#define SINGLE_MODE_DMA
#define MB8877_NO_BUSY_AFTER_SEEK
#define IO_ADDR_MAX		0x10000
#define SCSI_HOST_AUTO_ACK

// device informations for win32
#define USE_CPU_TYPE		2
#define USE_FLOPPY_DISK     4 // ??
#define NOTIFY_KEY_DOWN
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
class RF5C68;      // DAC
class YM2612;      // OPNB
class MB87078;     // VOLUME
class AD7820KR;    // A/D Converter.
class PCM1BIT;

class MB8877;      // FDC
class MSM58321;    // RTC
class RF5C68;      // ADPCM
class UPD71071;    // DMAC

class SCSI_HOST;
class SCSI_DEV;
class SCSI_HDD;
class SCSI_CDROM;

namespace FMTOWNS {
	class ADPCM;
	class CDC;
	class FLOPPY;
	class KEYBOARD;
	class SERIAL_ROM;
	class SCSI;
	class TIMER;
	
	class SYSROM;
	class MSDOSROM;
	class FONT_ROMS;
	class DICTIONARY;
#if defined(HAS_20PIX_FONTS)
	class FONT_ROM_20PIX;
#endif
	class TOWNS_CRTC;
	class TOWNS_VRAM;
	class TOWNS_MEMORY;

	class TOWNS_CDROM;
	class SPRITE;
	class JOYSTICK; // Mouse and Joystick.
}

class VM : public VM_TEMPLATE
{
protected:
	// devices
	
	I8251* sio;
	I8253* pit0;
	I8253* pit1;
	
	I8259* pic0;
	I8259* pic1;
	
	I386* cpu; // i386DX/SX/486DX/486SX?/Pentium with FPU?

	IO*       io;
	MB8877*   fdc;
	MSM58321* rtc;
	UPD71071* dma;
	UPD71071* extra_dma;
	RF5C68*   dac;
	MB87078*  e_volumes;
	AD7820KR* adc;
	RF5C68*   adpcm;
	PCM1BIT*  beep;
	
	FMTOWNS::TOWNS_CRTC*     crtc;
	FMTOWNS::FLOPPY*         floppy;
	FMTOWNS::KEYBOARD*       keyboard;
	FMTOWNS::TIMER*          timer;
	FMTOWNS::TOWNS_VRAM*     sprite;
	FMTOWNS::TOWNS_MEMORY*   memory;
	FMTOWNS::DICTIONARY*     dictionary;
	FMTOWNS::SYSROM*         sysrom;
	FMTOWNS::MSDOSROM*       msdosrom;
	FMTOWNS::FONT_ROMS*      fontrom;
#if defined(HAS_20PIX_FONTS)
	FMTOWNS::FONT_ROM_20PIX* fontrom_20pix;
#endif
	FMTOWNS::SERIAL_ROM*     serialrom;
	FMTOWNS::CDC*            cdc;
	SCSI_HOST*               cdc_scsi;
	FMTOWNS::TOWNS_CDROM*    cdrom;
	
	FMTOWNS::SCSI* scsi;
	SCSI_HOST*     scsi_host;
	SCSI_HDD*      hdd[4]; // 

	scrntype_t *d_renderbuffer[2][2]; // [bank][layer]
	uint32_t renderbuffer_size[2][2];
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

