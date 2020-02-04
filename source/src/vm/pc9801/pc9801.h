/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.15-

	[ virtual machine ]
*/

#ifndef _PC9801_H_
#define _PC9801_H_

/*
	PC-9801		8086	5MHz
	PC-9801E/F/M	8086	5/8MHz

	PC-9801U/VF	V30	8MHz
	PC-9801VM	V30	8/10MHz
	PC-98DO		V30	8/10MHz
	PC-98DO+	V33	8/16MHz

	PC-9801VX	80286	8/10MHz
	PC-9801RX/DX	80286	12MHz

	PC-9801RA	80386	16MHz
	PC-9801DA	80386	20MHz

	PC-98XA		80286	8MHz
	PC-98XL		80286	8MHz/10MHz

	PC-98XL^2	80386	16MHz
	PC-98RL		80386	20MHz
*/

#if defined(_PC9801)
	#define DEVICE_NAME		"NEC PC-9801"
	#define CONFIG_NAME		"pc9801"
	#define HAS_I86
	#define CPU_CLOCKS		4992030
	#define PIT_CLOCK_5MHZ
#elif defined(_PC9801E)
	#define DEVICE_NAME		"NEC PC-9801E/F/M"
	#define CONFIG_NAME		"pc9801e"
	#define HAS_I86
	#define CPU_CLOCKS		7987248
	#define PIT_CLOCK_8MHZ
//	#define CPU_CLOCKS		4992030
//	#define PIT_CLOCK_5MHZ
	#define USE_CPU_TYPE		2
#elif defined(_PC9801U) || defined(_PC9801VF)
	#if defined(_PC9801U)
		#define DEVICE_NAME	"NEC PC-9801U"
		#define CONFIG_NAME	"pc9801u"
	#elif defined(_PC9801VF)
		#define DEVICE_NAME	"NEC PC-9801VF"
		#define CONFIG_NAME	"pc9801vf"
	#endif
	#define HAS_V30
	#define CPU_CLOCKS		7987248
	#define PIT_CLOCK_8MHZ
#elif defined(_PC9801VM) || defined(_PC98DO)
	#if defined(_PC9801VM)
		#define DEVICE_NAME	"NEC PC-9801VM"
		#define CONFIG_NAME	"pc9801vm"
	#elif defined(_PC98DO)
		#define DEVICE_NAME	"NEC PC-98DO"
		#define CONFIG_NAME	"pc98do"
	#endif
	#define HAS_V30
	#define CPU_CLOCKS		9984060
	#define PIT_CLOCK_5MHZ
//	#define CPU_CLOCKS		7987248
//	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		2
#elif defined(_PC98DOPLUS)
	#define DEVICE_NAME		"NEC PC-98DO+"
	#define CONFIG_NAME		"pc98do+"
	#define HAS_V33A
	#define CPU_CLOCKS		15974496
	#define PIT_CLOCK_8MHZ
//	#define CPU_CLOCKS		7987248
//	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		2
#elif defined(_PC9801VX) || defined(_PC98XL)
	#if defined(_PC9801VX)
		#define DEVICE_NAME	"NEC PC-9801VX"
		#define CONFIG_NAME	"pc9801vx"
	#elif defined(_PC98XL)
		#define DEVICE_NAME	"NEC PC-98XL"
		#define CONFIG_NAME	"pc98xl"
	#endif
	#define HAS_I286
	#define HAS_V30_SUB_CPU
	#define CPU_CLOCKS		9984060
	#define PIT_CLOCK_5MHZ
//	#define CPU_CLOCKS		7987248
//	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		3
#elif defined(_PC98XA)
	#define DEVICE_NAME		"NEC PC-98XA"
	#define CONFIG_NAME		"pc98xa"
	#define HAS_I286
	#define CPU_CLOCKS		7987248
	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		3
#elif defined(_PC98RL)
	#define DEVICE_NAME	"NEC PC-98RL"
	#define CONFIG_NAME	"pc98rl"
	#define HAS_I386
	#define HAS_V30_SUB_CPU
	#define CPU_CLOCKS		19968120
	#define PIT_CLOCK_5MHZ
//	#define CPU_CLOCKS		15974496
//	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		3
	#define SUPPORT_24BIT_ADDRESS
#elif defined(_PC9801RA)
	#define DEVICE_NAME	"NEC PC-9801RA"
	#define CONFIG_NAME	"pc9801ra"
	#define HAS_I386
	#define HAS_V30_SUB_CPU
//	#define HAS_I486
	#define CPU_CLOCKS		19968120
	#define PIT_CLOCK_5MHZ
//	#define CPU_CLOCKS		15974496
//	#define PIT_CLOCK_8MHZ
	#define USE_CPU_TYPE		3
	#define SUPPORT_32BIT_ADDRESS
#else
	// unknown machines
#endif

#define DIPSWITCH_POSITION_USE_V30         (0 + 8 - 1)
// DIPSW POSITION
// DIPSW1: 8-15
#define DIPSWITCH_POSITION_HIGH_RESO       (8 + 1 - 1)
#define DIPSWITCH_POSITION_SUPER_IMPOSE    (8 + 2 - 1)
#define DIPSWITCH_POSITION_PLASMA_DISPLAY  (8 + 3 - 1)
#define DIPSWITCH_POSITION_SWAP_FDD        (8 + 4 - 1)
#define DIPSWITCH_POSITION_RS232C_SW1      (8 + 5 - 1)
#define DIPSWITCH_POSITION_RS232C_SW2      (8 + 6 - 1)
#define DIPSWITCH_POSITION_HDD_SECTOR_SIZE (8 + 7 - 1)
#define DIPSWITCH_POSITION_EGC             (8 + 8 - 1)

// DIPSW-2
#define DIPSWITCH_POSITION_LT_MODE       (16 + 1 - 1)
#define DIPSWITCH_POSITION_TERMINAL_MODE (16 + 2 - 1)
#define DIPSWITCH_POSITION_TEXT_WIDTH80  (16 + 3 - 1)
#define DIPSWITCH_POSITION_TEXT_HEIGHT25 (16 + 4 - 1)
#define DIPSWITCH_POSITION_NOINIT_MEMSW  (16 + 5 - 1)
#define DIPSWITCH_POSITION_INTFDD_OFF    (16 + 6 - 1)
#define DIPSWITCH_POSITION_DISABLE_VFKEY (16 + 7 - 1)
#define DIPSWITCH_POSITION_GDC_FAST      (16 + 8 - 1)

// SW3
#define DIPSWITCH_POSITION_FDD_MODE1      (24 + 1 - 1)
#define DIPSWITCH_POSITION_FDD_MODE2      (24 + 2 - 1)
#define DIPSWITCH_POSITION_FDD_DMA_CH     (24 + 3 - 1)
#define DIPSWITCH_POSITION_FDD_MOTOR_CTL  (24 + 4 - 1)
#define DIPSWITCH_POSITION_DMA_FAST       (24 + 5 - 1)
#define DIPSWITCH_POSITION_RAM512K        (24 + 6 - 1)
#define DIPSWITCH_POSITION_CPU_MODE       (24 + 8 - 1)

#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM) || defined(HAS_PENTIUM_PRO) || \
	defined(HAS_PENTIUM_MMX) || defined(HAS_PENTIUM2) || defined(HAS_PENTIUM3) || defined(HAS_PENTIUM4) || \
	defined(HAS_MEDIAGX)
#define UPPER_I386 1
#endif

#if defined(_PC9801) || defined(_PC9801E)
	#define SUPPORT_CMT_IF
	#define SUPPORT_2HD_FDD_IF
	#define SUPPORT_2DD_FDD_IF
	#define SUPPORT_320KB_FDD_IF
	#define SUPPORT_OLD_BUZZER
#elif defined(_PC9801VF) || defined(_PC9801U)
	#define SUPPORT_2DD_FDD_IF
#else
	#define SUPPORT_2HD_2DD_FDD_IF
#endif

#if defined(_PC98XA) || defined(_PC98XL) || defined(_PC98RL)
	#define SUPPORT_HIRESO
#endif
#if !(defined(_PC9801) || defined(_PC9801U) || defined(SUPPORT_HIRESO))
	#define SUPPORT_2ND_VRAM
#endif
#if !(defined(_PC9801) || defined(_PC9801E))
	#define SUPPORT_16_COLORS
	#define SUPPORT_GRCG
#endif
#if !(defined(HAS_I86) || defined(HAS_V30))
	#if !defined(_PC98XA) && !defined(_PC98XL)
		#define SUPPORT_ITF_ROM
	#endif
	#if !defined(_PC98XA)
		#define SUPPORT_EGC
		#define HAS_UPD4990A
	#endif
	#if !defined(SUPPORT_HIRESO)/* && !(defined(UPPER_I386))*/
		#define SUPPORT_NEC_EMS
	#endif
	#define SUPPORT_SASI_IF
#endif

#if defined(HAS_I286)
	#define SUPPORT_24BIT_ADDRESS
#elif defined(UPPER_I386)
//	#define SUPPORT_32BIT_ADDRESS
	#if !defined(SUPPORT_HIRESO)
	#define SUPPORT_BIOS_RAM
	#endif
	#define SUPPORT_EGC
	// PC-9801-86
	#define SUPPORT_PC98_OPNA
	#define SUPPORT_PC98_86PCM
	#define SUPPORT_PC98_86PCM_IRQ
#endif
#if defined(SUPPORT_24BIT_ADDRESS)
	#define MEMORY_ADDR_MAX		0x01000000	// 16MB
#elif defined(SUPPORT_32BIT_ADDRESS)
	#define MEMORY_ADDR_MAX		0x100000000	// 4GiB
#else
	#define MEMORY_ADDR_MAX		0x000100000	// 1MB
#endif
//#if defined(SUPPORT_32BIT_ADDRESS)
//	#define SUPPORT_SYSTEM_16MB
//#endif
#define MEMORY_BANK_SIZE		0x800
#define IO_ADDR_MAX			0x10000

#if defined(_PC98DO) || defined(_PC98DOPLUS)
	#define PC8801_VARIANT
	#define PC8801SR_VARIANT
	#define MODE_PC98	0
	#define MODE_PC88_V1S	1
	#define MODE_PC88_V1H	2
	#define MODE_PC88_V2	3
	#define MODE_PC88_N	4
	#define MODE_PC88_V2CD	5
	#define SUPPORT_PC88_KANJI1
	#define SUPPORT_PC88_KANJI2
	//#define SUPPORT_PC88_DICTIONARY
	#define SUPPORT_PC88_HIGH_CLOCK
	//#define SUPPORT_PC88_JOYSTICK
	#define PC88_EXRAM_BANKS	4
	#define SUPPORT_PC88_OPN1
#if defined(_PC98DOPLUS)
	#define SUPPORT_PC88_OPNA
#endif
#endif

// device informations for virtual machine
#if !defined(SUPPORT_HIRESO)
	#define FRAMES_PER_SEC		56.42
	#define LINES_PER_FRAME 	440
	#define SCREEN_WIDTH		640
	#define SCREEN_HEIGHT		400
	#define WINDOW_HEIGHT_ASPECT	480
	#define UPD7220_UGLY_PC98_HACK 1
#else
	#define FRAMES_PER_SEC		79.09
	#define LINES_PER_FRAME 	784
	#define SCREEN_WIDTH		1120
	#define SCREEN_HEIGHT		750
	#define WINDOW_HEIGHT_ASPECT	840
	#undef  UPD7220_UGLY_PC98_HACK
#endif
#define MAX_DRIVE		2
#define UPD765A_NO_ST1_EN_OR_FOR_RESULT7

#if defined(_PC98DO) || defined(_PC98DOPLUS)
#define PC80S31K_NO_WAIT
#endif

#if !defined(SUPPORT_HIRESO)
#define UPD7220_HORIZ_FREQ	24830
#else
#define UPD7220_HORIZ_FREQ	32860
#endif
#define UPD7220_MSB_FIRST

#define UPD7220_A_VERSION 3
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#define Z80_MEMORY_WAIT
#endif
#define I8259_MAX_CHIPS		2
#define SINGLE_MODE_DMA
#define OVERRIDE_SOUND_FREQ_48000HZ	55467

// device informations for win32
#define USE_DIPSWITCH
#if defined(_PC9801) || defined(_PC9801E)
#define USE_FLOPPY_DISK		6
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
#define USE_BOOT_MODE		5
#define DIPSWITCH_MEMWAIT	0x01
#define DIPSWITCH_CMDSING	0x10
#define DIPSWITCH_DEFAULT	(/*DIPSWITCH_HMB20 + DIPSWITCH_GSX8800 + DIPSWITCH_PCG8100 + */DIPSWITCH_CMDSING)
#define DIPSWITCH_PALETTE	0x20
#define USE_FLOPPY_DISK		4
#else
#define USE_FLOPPY_DISK		2
#endif
#if defined(SUPPORT_SASI_IF) || defined(SUPPORT_SCSI_IF) || defined(SUPPORT_IDE_IF)
#define USE_HARD_DISK		2
	#if defined(HAS_I286)
	#define I86_PSEUDO_BIOS
	#else
	#define I386_PSEUDO_BIOS
	#endif
#endif
#if defined(SUPPORT_CMT_IF) || defined(_PC98DO) || defined(_PC98DOPLUS)
#define USE_TAPE		1
#define TAPE_BINARY_ONLY
#endif
#define USE_KEY_LOCKED
#if defined(_PC98DO) || defined(_PC98DOPLUS)
// slow enough for N88-“ú–{ŒêBASIC
#define USE_AUTO_KEY		8
#define USE_AUTO_KEY_RELEASE	10
#else
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#endif
#define USE_AUTO_KEY_NUMPAD
#define USE_MONITOR_TYPE	2
#define USE_SCANLINE
#define USE_SCREEN_FILTER
#define USE_SOUND_TYPE		5
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#if    defined(SUPPORT_PC98_OPNA) &&  defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 1 + 4 + 1 + 1)
#elif  defined(SUPPORT_PC98_OPNA) && !defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 1 + 2 + 1 + 1)
#elif !defined(SUPPORT_PC98_OPNA) &&  defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 1 + 4 + 1 + 1)
#elif !defined(SUPPORT_PC98_OPNA) && !defined(SUPPORT_PC88_OPNA)
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 1 + 2 + 1 + 1)
#endif
#else
#if defined(SUPPORT_PC98_OPNA)
#define USE_SOUND_VOLUME	(4 + 1 + 1 + 1 + 1)
#else
#define USE_SOUND_VOLUME	(2 + 1 + 1 + 1)
#endif
#endif
#define USE_JOYSTICK
#define USE_MOUSE
#define USE_PRINTER
#define USE_PRINTER_TYPE	3
#define USE_DEBUGGER
#define I8259_PC98_HACK

#define USE_STATE
#if defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88) 
#define USE_CPU_I86
#elif defined(HAS_V30)
#define USE_CPU_V30
#elif defined(UPPER_I386)
#define USE_CPU_I386
#else
#define USE_CPU_I286
#define USE_CPU_V30
#endif
#if defined(SUPPORT_320KB_FDD_IF) || defined(_PC98DO) || defined(_PC98DOPLUS)
#define USE_CPU_Z80
#endif
#if defined(HAS_V30_SUB_CPU)
#define USE_CPU_V30
#endif

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(SUPPORT_PC98_OPNA)
	_T("OPNA (FM)"), _T("OPNA (PSG)"), _T("OPNA (ADPCM)"), _T("OPNA (Rhythm)"), _T("PCM(PC-9801-86)"),
#else
	_T("OPN (FM)"), _T("OPN (PSG)"),
#endif
	_T("PC-9801-14"), _T("Beep"),
#if defined(_PC98DO) || defined(_PC98DOPLUS)
#if defined(SUPPORT_PC88_OPNA)
	_T("PC-88 (FM)"), _T("PC-88 (PSG)"), _T("PC-88 (ADPCM)"), _T("PC-88 (Rhythm)"),
#else
	_T("PC-88 (FM)"), _T("PC-88 (PSG)"),
#endif
	_T("PC-88 (Beep)"), 
#endif
	_T("Noise (FDD)"),
};
#endif


class EMU;
class DEVICE;
class EVENT;

#if defined(SUPPORT_OLD_BUZZER)
class BEEP;
#endif
class DISK;
#if defined(USE_HARD_DISK)
class HARDDISK;
#endif
class I8237;
class I8251;
class I8253;
class I8255;
class I8259;
#if defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88)
class I8086;
#elif defined(HAS_V30)
class V30;
#elif defined(HAS_I286)
class I80286;
#elif defined(UPPER_I386)
class I386;
#else
class I286;
#endif
#if defined(HAS_V30_SUB_CPU)
class V30;
#endif

class IO;
class LS244;
//class MEMORY;
class NOISE;
class NOT;
#if !defined(SUPPORT_OLD_BUZZER)
class PCM1BIT;
#endif
#if defined(SUPPORT_SASI_IF)
class SASI_HDD;
class SCSI_HOST;
#define SCSI_HOST_AUTO_ACK
#elif defined(SUPPORT_SCSI_IF)
class SCSI_HDD;
class SCSI_HOST;
#define SCSI_HOST_AUTO_ACK
#endif
class TMS3631;
class UPD1990A;
class UPD7220;
class UPD765A;
class YM2203;

namespace PC9801 {
#if defined(SUPPORT_CMT_IF)
	class CMT;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	class CPUREG;
#endif
	class DISPLAY;
	class DIPSWITCH;
	class DMAREG;
	class FLOPPY;
	class FMSOUND;
	class JOYSTICK;
	class KEYBOARD;
	class MEMBUS;
	class MOUSE;
#if defined(SUPPORT_SASI_IF)
	class SASI;
	class BIOS;
#endif
#if defined(SUPPORT_SCSI_IF)
	class SCSI;
#endif
#if defined(SUPPORT_IDE_IF)
	class IDE;
#endif
}

#if defined(SUPPORT_320KB_FDD_IF)
// 320kb fdd drives
class PC80S31K;
class Z80;
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
class PC80S31K;
namespace PC88DEV {
	class PC88;
}
class Z80;
#endif

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	//EVENT* event;
	
#if defined(SUPPORT_OLD_BUZZER)
	BEEP* beep;
#else
	PCM1BIT* beep;
#endif
	DEVICE* printer;
	I8237* dma;
#if defined(SUPPORT_CMT_IF)
	I8251* sio_cmt;
#endif
	I8251* sio_rs;
	I8251* sio_kbd;
	I8253* pit;
#if defined(SUPPORT_320KB_FDD_IF)
	I8255* pio_fdd;
#endif
	I8255* pio_mouse;
	I8255* pio_sys;
	I8255* pio_prn;
	I8259* pic;
#if defined(UPPER_I386)
	I386* cpu;
#elif  defined(HAS_V30)
	V30*  cpu;
#elif defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88)
	I8086 *cpu;
#else
	I80286* cpu;
#endif
#if defined(HAS_V30_SUB_CPU)
	V30*    v30cpu;
#endif
	IO* io;
	LS244* rtcreg;
	//MEMORY* memory;
	NOT* not_busy;
#if defined(HAS_I86) || defined(HAS_V30)
	NOT* not_prn;
#endif
#if defined(SUPPORT_SASI_IF)
	SASI_HDD* sasi_hdd;
	SCSI_HOST* sasi_host;
#endif
#if defined(SUPPORT_SCSI_IF)
	SCSI_HDD* scsi_hdd[2];
	SCSI_HOST* scsi_host;
#endif
	UPD1990A* rtc;
#if defined(SUPPORT_2HD_FDD_IF)
	UPD765A* fdc_2hd;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	UPD765A* fdc_2dd;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	UPD765A* fdc;
#endif
	NOISE* noise_seek;
	NOISE* noise_head_down;
	NOISE* noise_head_up;
	UPD7220* gdc_chr;
	UPD7220* gdc_gfx;
	YM2203* opn;
	
#if defined(SUPPORT_CMT_IF)
	PC9801::CMT* cmt;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	PC9801::CPUREG* cpureg;
#endif
	PC9801::DIPSWITCH* dipswitch;
	PC9801::DISPLAY* display;
	PC9801::DMAREG* dmareg;
	PC9801::FLOPPY* floppy;
	PC9801::FMSOUND* fmsound;
	PC9801::JOYSTICK* joystick;
	PC9801::KEYBOARD* keyboard;
	PC9801::MEMBUS* memory;
	PC9801::MOUSE* mouse;
#if defined(SUPPORT_SASI_IF)
	PC9801::SASI* sasi;
	PC9801::BIOS *sasi_bios;
#endif
#if defined(SUPPORT_SCSI_IF)
	PC9801::SCSI* scsi;
#endif
#if defined(SUPPORT_IDE_IF)
	PC9801::IDE* ide;
#endif
	
	// PC-9801-14
	TMS3631* tms3631;
	I8253* pit_14;
	I8255* pio_14;
	LS244* maskreg_14;
	
#if defined(SUPPORT_320KB_FDD_IF)
	// 320kb fdd drives
	I8255* pio_sub;
	PC80S31K *pc80s31k;
	UPD765A* fdc_sub;
	Z80* cpu_sub;
#endif
	
	// misc
	bool pit_clock_8mhz;
	int sound_type;
	
#if defined(_PC98DO) || defined(_PC98DOPLUS)
	EVENT* pc88event;
	
	PC88DEV::PC88* pc88;
	DEVICE* pc88prn;
	I8251* pc88sio;
	I8255* pc88pio;
	PCM1BIT* pc88pcm;
	UPD1990A* pc88rtc;
	YM2203* pc88opn1;
	Z80* pc88cpu;
	
	PC80S31K* pc88sub;
	I8255* pc88pio_sub;
	UPD765A* pc88fdc_sub;
	NOISE* pc88noise_seek;
	NOISE* pc88noise_head_down;
	NOISE* pc88noise_head_up;
	Z80* pc88cpu_sub;
	
	int boot_mode;
#endif
	
	// drives
	UPD765A *get_floppy_disk_controller(int drv);
	DISK *get_floppy_disk_handler(int drv);
	void initialize_ports();
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
	double get_frame_rate();
	
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
	bool get_caps_locked();
	bool get_kana_locked();
	
	// user interface
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#if defined(USE_HARD_DISK)
	void open_hard_disk(int drv, const _TCHAR* file_path);
	void close_hard_disk(int drv);
	bool is_hard_disk_inserted(int drv);
	uint32_t is_hard_disk_accessed();
#endif
#if defined(USE_TAPE)
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
#endif
	bool is_frame_skippable();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	void set_cpu_clock_with_switch(int speed_type); // 0 = High / 1 = Low / Others = (WIP)
	void set_wait(int dispmode, int clock); // Set waitfor memories and IOs.
	
	// devices
	DEVICE* get_device(int id);
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
