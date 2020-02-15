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


#define HAS_MB89311

#if defined(_FMTOWNS_1)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 1"
#define CONFIG_NAME		"fmtowns_1"
#define MAX_DRIVE       1
#define _HAS_HDD        1
#undef HAS_MB89311

#elif defined(_FMTOWNS_2)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 2"
#define CONFIG_NAME		"fmtowns_2"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#undef HAS_MB89311

#elif defined(_FMTOWNS_2F)
#define DEVICE_NAME		"FUJITSU FM-Towns 2F"
#define CONFIG_NAME		"fmtowns_2f"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define _FMTOWNS1_2ND_GEN

#elif defined(_FMTOWNS_2H)
#define DEVICE_NAME		"FUJITSU FM-Towns 2H"
#define CONFIG_NAME		"fmtowns_2h"
#define MAX_DRIVE       2
#define _HAS_HDD        2
#define _FMTOWNS1_2ND_GEN

#elif defined(_FMTOWNS_20F)
#define DEVICE_NAME		"FUJITSU FM-Towns 20F"
#define CONFIG_NAME		"fmtowns_20f"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define TYPE_TOWNS_X0   1
#define _FMTOWNS1_3RD_GEN

#elif defined(_FMTOWNS_40H)
#define DEVICE_NAME		"FUJITSU FM-Towns 40H"
#define CONFIG_NAME		"fmtowns_20h"
#define MAX_DRIVE       2
#define _HAS_HDD        2
#define TYPE_TOWNS_X0   1
#define _FMTOWNS1_3RD_GEN

#elif defined(_FMTOWNS2_UX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX20"
#define CONFIG_NAME		"fmtowns2_ux20"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1
#define _FMTOWNS_UX_VARIANTS

#elif defined(_FMTOWNS2_UX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX40"
#define CONFIG_NAME		"fmtowns2_ux40"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1
#define _FMTOWNS_UX_VARIANTS

#elif defined(_FMTOWNS2_CX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX20"
#define CONFIG_NAME		"fmtowns2_cx20"
#define MAX_DRIVE       2
#undef _HAS_HDD
#define TYPE_TOWNS2_CX  1
#define _FMTOWNS2_CX_VARIANTS

#elif defined(_FMTOWNS2_CX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2_cx40"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1
#define _FMTOWNS2_CX_VARIANTS

#elif defined(_FMTOWNS2_CX100)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2_cx100"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1
#define _FMTOWNS2_CX_VARIANTS
#elif defined(_FMTOWNS2_UG10)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG10"
#define CONFIG_NAME		"fmtowns2_ug1"
#define MAX_DRIVE       2
#undef  _HAS_HDD        
#define WITH_386SX      1
#define _FMTOWNS_UG_VARIANTS
#elif defined(_FMTOWNS2_UG20)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG20"
#define CONFIG_NAME		"fmtowns2_ug20"
#define MAX_DRIVE       2
#undef  _HAS_HDD        
#define WITH_386SX      1
#define _FMTOWNS_UG_VARIANTS
#elif defined(_FMTOWNS2_UG40)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG40"
#define CONFIG_NAME		"fmtowns2_ug40"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#define WITH_386SX      1
#define _FMTOWNS_UG_VARIANTS
#elif defined(_FMTOWNS2_UG80)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG80"
#define CONFIG_NAME		"fmtowns2_ug80"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#define WITH_386SX      1
#define _FMTOWNS_UG_VARIANTS

#endif

#if defined(WITH_386SX)
#define MEMORY_ADDR_MAX 0x001000000 /* 16MB */
#else
#define MEMORY_ADDR_MAX 0x100000000 /* 4GiB */
#endif
#define MEMORY_BANK_SIZE 1024

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4 // OK?
#define LINES_PER_FRAME 	784  // OK?

#define CPU_CLOCKS		16000000

#undef FIXED_FRAMEBUFFER_SIZE
#define SCREEN_WIDTH		    1024
#define SCREEN_HEIGHT		    768
#define WINDOW_WIDTH_ASPECT	    1024
#define WINDOW_HEIGHT_ASPECT    768

#if defined(_HAS_HDD)
#define MAX_SCSI		8
#define USE_HARD_DISK   _HAS_HDD
#endif
#define USE_COMPACT_DISC 1

#define MAX_MEMCARD		2
#define I8259_MAX_CHIPS		2

#define SINGLE_MODE_DMA
#define MB8877_NO_BUSY_AFTER_SEEK
#define IO_ADDR_MAX		0x10000
#define SCSI_HOST_AUTO_ACK
//#define SCSI_HOST_WIDE
#define _SCSI_DEBUG_LOG
//#define SCSI_DEV_IMMEDIATE_SELECT
#define _CDROM_DEBUG_LOG

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
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_JOYSTICK_TYPE	2
#define JOYSTICK_TYPE_DEFAULT	0
#define USE_MOUSE
#define USE_MOUSE_TYPE      3
#define USE_CUSTOM_SCREEN_ZOOM_FACTOR 1.25

#if defined(USE_SOUND_FILES)
#define USE_SOUND_VOLUME	5
#else
#define USE_SOUND_VOLUME	6
#endif

#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_I386
#define HAS_I386

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("Beep"), _T("CD-DA"), _T("FM OPN2"), _T("ADPCM"), 
#if defined(USE_SOUND_FILES)
	_T("FDD SEEK"),
#endif
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
	_T("RUN"),
	_T("SELECT"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class I8251;
class I8253;
class I8259;
class I386;
class NOISE;

class IO;
class RF5C68;      // DAC
class YM2612;      // OPNB
//class MB87078;     // VOLUME
class AD7820KR;    // A/D Converter.
class PCM1BIT;

class MB8877;      // FDC
class MSM58321;    // RTC
class RF5C68;      // ADPCM

class SCSI_HOST;
class SCSI_DEV;
class SCSI_HDD;
class SCSI_CDROM;

namespace FMTOWNS {
	class ADPCM;
	class CDC;
	class FLOPPY;
	class JOYSTICK;
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
	class TOWNS_CDROM;
	class TOWNS_DMAC;    // DMAC
	class TOWNS_MEMORY;
	class TOWNS_SCSI_HOST;
	class TOWNS_SPRITE;
	class TOWNS_VRAM;

	class JOYSTICK; // Mouse and Joystick.
}

class VM : public VM_TEMPLATE
{
protected:
	// devices
	
	I8251* sio;
	I8253* pit0;
	I8253* pit1;
	
	I8259* pic;
	
	I386* cpu; // i386DX/SX/486DX/486SX?/Pentium with FPU?

	IO*       io;
	MB8877*   fdc;
	MSM58321* rtc;
	FMTOWNS::TOWNS_DMAC* dma;
	FMTOWNS::TOWNS_DMAC* extra_dma;
	NOISE*    seek_sound;
	NOISE*    head_up_sound;
	NOISE*    head_down_sound;
	
	RF5C68*   rf5c68;
//	MB87078*  e_volumes;
	AD7820KR* adc;
	PCM1BIT*  beep;
	YM2612*   opn2;
	
	FMTOWNS::ADPCM*          adpcm;
	FMTOWNS::TOWNS_CRTC*     crtc;
	FMTOWNS::FLOPPY*         floppy;
	FMTOWNS::JOYSTICK*       joystick;
	FMTOWNS::KEYBOARD*       keyboard;
	FMTOWNS::TIMER*          timer;
	FMTOWNS::TOWNS_VRAM*     vram;
	FMTOWNS::TOWNS_SPRITE*   sprite;
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
	FMTOWNS::TOWNS_SCSI_HOST* cdc_scsi;
	//SCSI_HOST* cdc_scsi;
	FMTOWNS::TOWNS_CDROM*    cdrom;
	
	FMTOWNS::SCSI* scsi;
	FMTOWNS::TOWNS_SCSI_HOST* scsi_host;
	//SCSI_HOST* scsi_host;
	SCSI_HDD*      scsi_hdd[8]; //

	int adc_in_ch;
	int line_in_ch;
	int modem_in_ch;
	int mic_in_ch;

	int beep_mix_ch;
	int cdc_mix_ch;
	int opn2_mix_ch;
	int pcm_mix_ch;
	int line_mix_ch;
	int modem_mix_ch;
	int mic_mix_ch;
/*
	scrntype_t *d_renderbuffer[2][2]; // [bank][layer]
	uint32_t renderbuffer_size[2][2];
*/
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
	uint32_t is_floppy_disk_accessed();
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	bool is_frame_skippable();

	void open_compact_disc(int drv, const _TCHAR* file_path);
	void close_compact_disc(int drv);
	bool is_compact_disc_inserted(int drv);
	uint32_t is_compact_disc_accessed();
#if defined(USE_HARD_DISK)
	void open_hard_disk(int drv, const _TCHAR* file_path);
	void close_hard_disk(int drv);
	bool is_hard_disk_inserted(int drv);
	uint32_t is_hard_disk_accessed();
#endif	
	void set_machine_type(uint16_t machine_id, uint16_t cpu_id);

	void clear_sound_in();
	int get_sound_in_data(int ch, int32_t* dst, int expect_samples, int expect_rate, int expect_channels);
	int sound_in(int ch, int32_t* src, int samples);

	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

