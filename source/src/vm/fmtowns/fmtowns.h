/*
	FUJITSU FMR-50 Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-
	[ virtual machine ]
*/
#pragma once


#undef WITH_386SX
#undef WITH_I486DX
#undef WITH_I486SX
#undef WITH_PENTIUM
#undef TYPE_TOWNS_X0
#undef TYPE_TOWNS2_UX
#undef TYPE_TOWNS2_CX


#define HAS_MB89311

#define RAM_SIZE_ORDER (1024*1024)

#if defined(_FMTOWNS_1)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 1"
#define CONFIG_NAME		"fmtowns1"
#define MAX_DRIVE       1
#define _HAS_HDD        4
#undef HAS_MB89311
#define USE_VARIABLE_MEMORY 6
#define MIN_RAM_SIZE 1

#elif defined(_FMTOWNS_2)
#define DEVICE_NAME		"FUJITSU FM-Towns Model 2"
#define CONFIG_NAME		"fmtowns2"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#undef HAS_MB89311
#define USE_VARIABLE_MEMORY 6
#define MIN_RAM_SIZE 1

#elif defined(_FMTOWNS_2F)
#define DEVICE_NAME		"FUJITSU FM-Towns 2F"
#define CONFIG_NAME		"fmtowns2F"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define _FMTOWNS1_2ND_GEN
#define USE_VARIABLE_MEMORY 8
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS_2H)
#define DEVICE_NAME		"FUJITSU FM-Towns 2H"
#define CONFIG_NAME		"fmtowns2H"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define _FMTOWNS1_2ND_GEN
#define USE_VARIABLE_MEMORY 8
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS_20F)
#define DEVICE_NAME		"FUJITSU FM-Towns 20F"
#define CONFIG_NAME		"fmtowns20F"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define TYPE_TOWNS_X0   1
#define _FMTOWNS1_3RD_GEN
#define USE_VARIABLE_MEMORY 8
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS_40H)
#define DEVICE_NAME		"FUJITSU FM-Towns 40H"
#define CONFIG_NAME		"fmtowns20H"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS_X0   1
#define _FMTOWNS1_3RD_GEN
#define USE_VARIABLE_MEMORY 8
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS2_UX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX20"
#define CONFIG_NAME		"fmtowns2UX20"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1
#define _FMTOWNS_UX_VARIANTS
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS2_UX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II UX40"
#define CONFIG_NAME		"fmtowns2UX40"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define WITH_386SX      1
#define TYPE_TOWNS2_UX  1
#define _FMTOWNS_UX_VARIANTS
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS2_CX20)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX20"
#define CONFIG_NAME		"fmtowns2CX20"
#define MAX_DRIVE       2
#undef _HAS_HDD
#define TYPE_TOWNS2_CX  1
#define _FMTOWNS2_CX_VARIANTS
#define USE_VARIABLE_MEMORY 15
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS2_CX40)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2CX40"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1
#define _FMTOWNS2_CX_VARIANTS
#define USE_VARIABLE_MEMORY 15
#define MIN_RAM_SIZE 2

#elif defined(_FMTOWNS2_CX100)
#define DEVICE_NAME		"FUJITSU FM-Towns II CX40"
#define CONFIG_NAME		"fmtowns2CX100"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define TYPE_TOWNS2_CX  1
#define USE_VARIABLE_MEMORY 15
#define MIN_RAM_SIZE 2
#define _FMTOWNS2_CX_VARIANTS

#elif defined(_FMTOWNS2_UG10)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG10"
#define CONFIG_NAME		"fmtowns2UG1"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define WITH_386SX      1
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#define _FMTOWNS_UG_VARIANTS

#elif defined(_FMTOWNS2_UG20)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG20"
#define CONFIG_NAME		"fmtowns2UG20"
#define MAX_DRIVE       2
#undef  _HAS_HDD
#define WITH_386SX      1
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#define _FMTOWNS_UG_VARIANTS

#elif defined(_FMTOWNS2_UG40)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG40"
#define CONFIG_NAME		"fmtowns2UG40"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define WITH_386SX      1
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#define _FMTOWNS_UG_VARIANTS

#elif defined(_FMTOWNS2_UG80)
#define DEVICE_NAME		"FUJITSU FM-Towns II UG80"
#define CONFIG_NAME		"fmtowns2UG80"
#define MAX_DRIVE       2
#define _HAS_HDD        1
#define WITH_386SX      1
#define USE_VARIABLE_MEMORY 9
#define MIN_RAM_SIZE 2

#define _FMTOWNS_UG_VARIANTS

#elif defined(_FMTOWNS2_HG20)
#define DEVICE_NAME		"FUJITSU FM-Towns II HG20"
#define CONFIG_NAME		"fmtowns2HG20"
#define MAX_DRIVE       2
#undef _HAS_HDD
#define USE_VARIABLE_MEMORY 15
#define MIN_RAM_SIZE 2

#define _FMTOWNS_HG_VARIANTS
#elif defined(_FMTOWNS2_HG40)
#define DEVICE_NAME		"FUJITSU FM-Towns II HG40"
#define CONFIG_NAME		"fmtowns2HG40"
#define MAX_DRIVE       2
#define _HAS_HDD          4

#define USE_VARIABLE_MEMORY 15
#define MIN_RAM_SIZE 2

#define _FMTOWNS_HG_VARIANTS

#elif defined(_FMTOWNS2_HR20)
#define DEVICE_NAME		"FUJITSU FM-Towns II HR20"
#define CONFIG_NAME		"fmtowns2HR20"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define _FMTOWNS_HR_VARIANTS

#define USE_VARIABLE_MEMORY 31
#define MIN_RAM_SIZE 2

#define WITH_I486SX
#elif defined(_FMTOWNS2_HR100)
#define DEVICE_NAME		"FUJITSU FM-Towns II HR100"
#define CONFIG_NAME		"fmtowns2HR100"
#define MAX_DRIVE       2
#define _HAS_HDD        4

#define USE_VARIABLE_MEMORY 31
#define MIN_RAM_SIZE 2

#define _FMTOWNS_HR_VARIANTS
#define WITH_I486SX

#elif defined(_FMTOWNS2_HR200)
#define DEVICE_NAME		"FUJITSU FM-Towns II HR200"
#define CONFIG_NAME		"fmtowns2HR200"
#define MAX_DRIVE       2
#define _HAS_HDD        4
#define _FMTOWNS_HR_VARIANTS
#define WITH_I486SX

#define USE_VARIABLE_MEMORY 31
#define MIN_RAM_SIZE 2

#endif

// ToDo: 486SX etc.

#define _MEMORY_BANK_SIZE 0x1000
#if defined(WITH_386SX)
#define _MEMORY_SPACE 0x01000000 /* 16MB */
#define _MEMORY_BUS_WIDTH 16
#else
#define _MEMORY_SPACE 0x100000000LL /* 4GiB */
#define _MEMORY_BUS_WIDTH 32
#endif

#define _IO_SPACE     0x10000
#define _IO_BUS_WIDTH 16

// device informations for virtual machine
#define FRAMES_PER_SEC		55.4 // OK?
#define LINES_PER_FRAME 	1024  // OK?

#define CPU_CLOCKS		16000000 // This maybe dummy value, see VM::VM().

#undef FIXED_FRAMEBUFFER_SIZE
#define SCREEN_WIDTH		    1024
#define SCREEN_HEIGHT		    1024 /* This is for full emulation. 20240104 K.O */
#define WINDOW_WIDTH_ASPECT	    1024
#define WINDOW_HEIGHT_ASPECT    768

#if defined(_HAS_HDD)
#define MAX_SCSI		8
#define USE_HARD_DISK   _HAS_HDD
#endif
#define USE_COMPACT_DISC 1

#define MAX_MEMCARD		2
//#define I8259_PC98_HACK

//#define SINGLE_MODE_DMA
//#define MB8877_NO_BUSY_AFTER_SEEK
#define SCSI_HOST_AUTO_ACK

//#define SCSI_HOST_WIDE
//#define _SCSI_DEBUG_LOG
#define SCSI_DEV_IMMEDIATE_SELECT
//#define _CDROM_DEBUG_LOG
//#define _IO_DEBUG_LOG

// device informations for win32
#define USE_NOTIFY_POWER_OFF
#define USE_CPU_TYPE			2
#define USE_FLOPPY_DISK			4 // ??
#define USE_CART				2
#define USE_SPECIAL_RESET		12 /* 'CD' 'F0' - 'F3' 'H0' - 'H4' 'ICM' 'DEBUG' */
#define USE_FLOPPY_TYPE_BIT		0x0003 /* 5.0, 5.0, 3.5, 3.5 */
#define USE_MACHINE_FEATURES	2
#define NOTIFY_KEY_DOWN
#define USE_ALT_F10_KEY
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_CRT_FILTER
#define USE_SOUND_FILES		1
#define USE_SOUND_FILES_FDD
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_MOUSE
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
#define BASE_FLOPPY_DISK_NUM 0
//#define USE_QUEUED_SCSI_TRANSFER

#include "../vm_limits.h"
#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

/* OVERRIDE EVENT PARAMETER(S) for high resolution */
#if defined(MAX_LINES)
#undef MAX_LINES
#endif
#define MAX_LINES (1280+128)

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
class MB87078;     // VOLUME
class AD7820KR;    // A/D Converter.
class PCM1BIT;

class MB8877;      // FDC
class MSM58321;    // RTC
class RF5C68;      // ADPCM

class SCSI_HOST;
class SCSI_DEV;
class SCSI_HDD;
class SCSI_CDROM;
class SERIALROM;

namespace FMTOWNS {
	class ADPCM;
	class CMOS;
//	class CDC;
	class FLOPPY;
	class JOYSTICK;
	class KEYBOARD;
	class TOWNS_SERIAL_ROM;
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
	class TOWNS_ICCARD;
	class TOWNS_MEMORY;
	//class TOWNS_SCSI_HOST;
	class TOWNS_SPRITE;
	class TOWNS_VRAM;
	class PLANEVRAM;
	class JOYPAD_2BTN;
	class JOYPAD_6BTN;
	class MOUSE;
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
	NOISE*      seek_sound;
	NOISE*      head_up_sound;
	NOISE*      head_down_sound;

	RF5C68*     rf5c68;
	MB87078*    e_volumes[2];
	AD7820KR*   adc;
	PCM1BIT*    beep;
	SCSI_HOST*  scsi_host;
	SCSI_HDD*   scsi_hdd[8]; //
	SERIALROM*  serialrom;
	YM2612*     opn2;

	FMTOWNS::ADPCM*          adpcm;
//	FMTOWNS::CDC*            cdc;
	FMTOWNS::CMOS*		     cmos;
	FMTOWNS::DICTIONARY*     dictionary;
	FMTOWNS::FLOPPY*         floppy;
	FMTOWNS::FONT_ROMS*      fontrom;
#if defined(HAS_20PIX_FONTS)
	FMTOWNS::FONT_ROM_20PIX* fontrom_20pix;
#endif
	FMTOWNS::JOYPAD_2BTN*    joypad_2btn[2];
	FMTOWNS::JOYPAD_6BTN*    joypad_6btn[2];
	FMTOWNS::JOYSTICK*       joystick;
	FMTOWNS::KEYBOARD*       keyboard;
	FMTOWNS::MOUSE*			 mouse[2];
	FMTOWNS::MSDOSROM*       msdosrom;
	FMTOWNS::PLANEVRAM*	     planevram;
	FMTOWNS::SCSI* scsi;

	FMTOWNS::SYSROM*         sysrom;
	FMTOWNS::TIMER*          timer;
	FMTOWNS::TOWNS_CDROM*    cdrom;
	FMTOWNS::TOWNS_CRTC*     crtc;
	FMTOWNS::TOWNS_DMAC*     dma;
	FMTOWNS::TOWNS_DMAC*     extra_dma;
	FMTOWNS::TOWNS_MEMORY*   memory;
	FMTOWNS::TOWNS_ICCARD*   iccard1;
	FMTOWNS::TOWNS_ICCARD*   iccard2;
	//	FMTOWNS::TOWNS_SCSI_HOST* cdc_scsi;
	//FMTOWNS::TOWNS_SCSI_HOST* scsi_host;
	FMTOWNS::TOWNS_SERIAL_ROM*     serialrom_if;
	FMTOWNS::TOWNS_SPRITE*   sprite;
	FMTOWNS::TOWNS_VRAM*     vram;

	bool boot_seq;

	int adc_in_ch;
	int line_in_ch;
	int modem_in_ch;
	int mic_in_ch;

/*
	scrntype_t *d_renderbuffer[2][2]; // [bank][layer]
	uint32_t renderbuffer_size[2][2];
*/
	virtual void process_boot_sequence(uint32_t val);
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
	void special_reset(int num) override;

#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index) override;
#endif

	// draw screen
	void draw_screen() override;
	void request_update_screen() override;

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
	// CARTs are IC CARD.Will implement something :-)
	void open_cart(int drv, const _TCHAR* file_path) override;
	void close_cart(int drv) override;
	bool is_cart_inserted(int drv) override;

	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) override;
	void close_floppy_disk(int drv) override;
	uint32_t is_floppy_disk_accessed() override;
	bool is_floppy_disk_inserted(int drv) override;
	void is_floppy_disk_protected(int drv, bool value) override;
	bool is_floppy_disk_protected(int drv) override;
	bool is_frame_skippable() override;

	void open_compact_disc(int drv, const _TCHAR* file_path) override;
	void close_compact_disc(int drv) override;
	bool is_compact_disc_inserted(int drv) override;
	uint32_t is_compact_disc_accessed() override;
#if defined(USE_HARD_DISK)
	void open_hard_disk(int drv, const _TCHAR* file_path) override;
	void close_hard_disk(int drv) override;
	bool is_hard_disk_inserted(int drv) override;
	uint32_t is_hard_disk_accessed() override;
#endif
	void set_machine_type(uint16_t machine_id, uint16_t cpu_id);
	void clear_sound_in();
	int get_sound_in_data(int ch, int32_t* dst, int expect_samples, int expect_rate, int expect_channels);
	int sound_in(int ch, int32_t* src, int samples);

	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;

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
