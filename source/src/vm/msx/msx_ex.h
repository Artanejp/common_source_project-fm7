/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/msx/msx.h

	modified by umaiboux
	Date   : 2016.03.xx-

	[ virtual machine ]
*/

#ifndef _MSX_EX_H_
#define _MSX_EX_H_

#if defined(_PX7)
#define DEVICE_NAME		"PIONEER PX-7"
#define CONFIG_NAME		"px7"
#elif defined(_MSX1)
#define DEVICE_NAME		"ASCII MSX1"
#define CONFIG_NAME		"msx1"
#elif defined(_MSX2)
#define DEVICE_NAME		"ASCII MSX2"
#define CONFIG_NAME		"msx2"
#elif defined(_MSX2P)
#define DEVICE_NAME		"ASCII MSX2+"
#define CONFIG_NAME		"msx2p"
#elif defined(_HX20)
#define DEVICE_NAME		"TOSHIBA HX-20+"
#define CONFIG_NAME		"hx20"
#elif defined(_FSA1)
#define DEVICE_NAME		"Panasonic FS-A1"
#define CONFIG_NAME		"fsa1"
#elif defined(_HBF1XDJ)
#define DEVICE_NAME		"SONY HB-F1XDJ"
#define CONFIG_NAME		"hbf1xdj"
#endif

#if defined(_PX7)

#define _MSX1_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define LDC_SLOT	0x02
#define CART2_SLOT	0x03
#define MAINROM_PLUS_RAM_32K

#elif defined(_HX20)

#define _MSX1_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define RAM64K_SLOT	0x83
#define FDD_PATCH_SLOT	0x8B
#define FIRMWARE32K1_SLOT 0x8F
#define FIRMWARE32K1_FILENAME _T("HX20FIRM.ROM")
#define MSX_PSG_STEREO

#elif defined(_FSA1)

#define _MSX2_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define RAM64K_SLOT	0x83
#define SUBROM_SLOT	0x87
#define FIRMWARE32K1_SLOT 0x8B
#define FIRMWARE32K1_FILENAME _T("FSA1FIRM1.ROM")
#define FIRMWARE32K2_SLOT 0x8F
#define FIRMWARE32K2_FILENAME _T("FSA1FIRM2.ROM")
#define MSX_KEYBOARD_50ON

#elif defined(_HBF1XDJ)

#define _MSX2P_VARIANTS
#define MAINROM_SLOT	0x80
//#define HALNOTE_SLOT	0x8C /* MSX-JE */
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define MAPPERRAM_SLOT	0x83
#define SUBROM_SLOT	0x87
#define FDD_PATCH_SLOT	0x8B
#define MSXMUSIC_SLOT	0x8F
#define MAPPERRAM_SIZE	(64*1024)
#define MAPPERRAM_MASK	0x03

#elif defined(_MSX2P)

#define _MSX2P_VARIANTS
#define MAINROM_SLOT	0x80
#define MSXMUSIC_SLOT	0x88
#define MSXDOS2_SLOT	0x8C
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define MAPPERRAM_SLOT	0x83
#define SUBROM_SLOT	0x87
#define FDD_PATCH_SLOT	0x8B
#define MAPPERRAM_SIZE	(2*1024*1024)
#define MAPPERRAM_MASK	0x7F
#define MSX_FDD_PATCH_WITH_2HD

#elif defined(_MSX2)

#define _MSX2_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define MAPPERRAM_SLOT	0x83
#define SUBROM_SLOT	0x87
#define MSXDOS2_SLOT	0x8B
#define FDD_PATCH_SLOT	0x8F
#define MAPPERRAM_SIZE	(128*1024)
#define MAPPERRAM_MASK	0x07

#elif defined(_MSX1)

#define _MSX1_VARIANTS
#define MAINROM_SLOT	0x00
#define CART1_SLOT	0x01
#define CART2_SLOT	0x02
#define MAPPERRAM_SLOT	0x83
#define MSXMUSIC_SLOT	0x87
#define FDD_PATCH_SLOT	0x8B
#define MAPPERRAM_SIZE	(128*1024)
#define MAPPERRAM_MASK	0x07

#endif

#if defined(_MSX_VDP_MESS)
#if defined(_MSX2P_VARIANTS)
#define V99X8 v9958_device
#elif defined(_MSX2_VARIANTS)
#define V99X8 v9938_device
#endif
#endif

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		3579545
#if !defined(_MSX1_VARIANTS)
#if defined(_MSX_VDP_MESS)
//#define SCREEN_WIDTH		(544)	// v99x8_device::HVISIBLE
//#define SCREEN_HEIGHT		(480)	//
//#define WINDOW_WIDTH_ASPECT	(544*9/8)
#define SCREEN_WIDTH		((256 + 15)*2)	// same as V99X8_WIDTH
#define SCREEN_HEIGHT		((212 + 15)*2)	// same as V99X8_HEIGHT
#define WINDOW_WIDTH_ASPECT	((288 + 17)*2)
#else
#define SCREEN_WIDTH		((256 + 15)*2)	// V99X8_WIDTH
#define SCREEN_HEIGHT		((212 + 15)*2)	// V99X8_HEIGHT
#define WINDOW_WIDTH_ASPECT	((288 + 17)*2)
#endif
#else
#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		384
#define WINDOW_WIDTH_ASPECT	576
#endif
#define TMS9918A_VRAM_SIZE	0x4000
#define TMS9918A_LIMIT_SPRITES
#if defined(LDC_SLOT)
#define TMS9918A_SUPER_IMPOSE
#endif
#if defined(FDD_PATCH_SLOT)
#define MAX_DRIVE		2
#define SUPPORT_MEDIA_TYPE_1DD
#define Z80_PSEUDO_BIOS
#endif
#define HAS_AY_3_8910
// for Flappy Limited '85
#define AY_3_891X_PORT_MODE	0x80

// device informations for win32
#define USE_CART		2
#define USE_TAPE		1
#if defined(LDC_SLOT)
#define USE_LASER_DISC		1
#define USE_MOVIE_PLAYER
#endif
#if defined(FDD_PATCH_SLOT)
#define USE_FLOPPY_DISK		2
#endif
#define USE_AUTO_KEY		6
#define USE_AUTO_KEY_RELEASE	10
#if defined(_PX7)
#define USE_SOUND_VOLUME	8
#else
#define USE_SOUND_VOLUME	7
#endif
#define USE_JOYSTICK
#define USE_DEBUGGER
#define USE_STATE
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#if defined(MSX_PSG_STEREO)
#define USE_SOUND_TYPE		2
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("Beep"), _T("CMT (Signal)"),
#if defined(_PX7)
	_T("LD-700"),
#endif
	_T("Cart#1"), _T("Cart#2"), _T("MSX-MUSIC"), _T("Noise (CMT)"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class I8255;
class IO;
#if defined(LDC_SLOT)
class LD700;
#endif
class NOT;
//class YM2203;
class AY_3_891X;
class PCM1BIT;
#if !defined(_MSX1_VARIANTS)
class RP5C01;
class V99X8;
#else
class TMS9918A;
#endif
class Z80;

class JOYSTICK;
class KEYBOARD;
class MEMORY_EX;
#if !defined(_MSX1_VARIANTS)
class RTCIF;
#endif
class SLOT_MAINROM;
class SLOT_CART;
class SOUND_CART;
class KANJIROM;
#if defined(MSX_PSG_STEREO)
class PSG_STEREO;
#endif
#if defined(USE_PRINTER)
class PRINTER;
#endif
#if defined(LDC_SLOT)
class SLOT_LDC;
#endif
#if defined(MAPPERRAM_SLOT)
class SLOT_MAPPERRAM;
#endif
#if defined(RAM64K_SLOT)
class SLOT_RAM64K;
#endif
#if defined(SUBROM_SLOT)
class SLOT_SUBROM;
#endif
#if defined(FIRMWARE32K1_SLOT)
class SLOT_FIRMWARE32K;
#endif
#if defined(FDD_PATCH_SLOT)
class SLOT_FDD_PATCH;
#endif
#if defined(MSXDOS2_SLOT)
class SLOT_MSXDOS2;
#endif
class YM2413;
#if defined(MSXMUSIC_SLOT)
class SLOT_MSXMUSIC;
#endif


class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	DATAREC* drec;
	I8255* pio;
	IO* io;
#if defined(LDC_SLOT)
	LD700* ldp;
#endif
	NOT* not_remote;
//	YM2203* psg;
	AY_3_891X* psg;
	PCM1BIT* pcm;
#if !defined(_MSX1_VARIANTS)
	RP5C01* rtc;
	V99X8* vdp;
#else
	TMS9918A* vdp;
#endif
	Z80* cpu;
	
	JOYSTICK* joystick;
	KEYBOARD* keyboard;
	MEMORY_EX* memory;
#if !defined(_MSX1_VARIANTS)
	RTCIF* rtcif;
#endif
	SLOT_MAINROM *slot_mainrom;
	SLOT_CART *slot_cart[2];
	SOUND_CART* sound_cart[2];
	KANJIROM* kanjirom;
#if defined(MSX_PSG_STEREO)
	PSG_STEREO* psg_stereo;
#endif
#ifdef USE_PRINTER
	PRINTER* printer;
#endif
#if defined(LDC_SLOT)
	SLOT_LDC *slot_ldc;
#endif
#if defined(MAPPERRAM_SLOT)
	SLOT_MAPPERRAM *slot_ram;
#endif
#if defined(RAM64K_SLOT)
	SLOT_RAM64K *slot_ram;
#endif
#if defined(SUBROM_SLOT)
	SLOT_SUBROM *slot_subrom;
#endif
#if defined(FIRMWARE32K1_SLOT)
	SLOT_FIRMWARE32K *slot_firmware32k1;
#endif
#if defined(FIRMWARE32K2_SLOT)
	SLOT_FIRMWARE32K *slot_firmware32k2;
#endif
#if defined(FDD_PATCH_SLOT)
	SLOT_FDD_PATCH *slot_fdd_patch;
#endif
#if defined(MSXDOS2_SLOT)
	SLOT_MSXDOS2 *slot_msxdos2;
#endif
	YM2413* ym2413;
#if defined(MSXMUSIC_SLOT)
	SLOT_MSXMUSIC *slot_msxmusic;
#endif
	
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
#if defined(LDC_SLOT)
	void movie_sound_callback(uint8_t *buffer, long size);
#endif
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
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
#if defined(LDC_SLOT)
	void open_laser_disc(int drv, const _TCHAR* file_path);
	void close_laser_disc(int drv);
	bool is_laser_disc_inserted(int drv);
	uint32_t is_laser_disc_accessed();
#endif
#if defined(FDD_PATCH_SLOT)
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#endif
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
