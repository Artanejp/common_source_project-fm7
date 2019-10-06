/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ virtual machine ]
*/

#ifndef _FAMILYBASIC_H_
#define _FAMILYBASIC_H_

#define DEVICE_NAME		"Nintendo Family BASIC"
#define CONFIG_NAME		"familybasic"

// device informations for virtual machine
#define FRAMES_PER_SEC		59.98
//#define LINES_PER_FRAME	262
#define LINES_PER_FRAME		525 // 262.5*2
#define CPU_CLOCKS		1789772
#define SCREEN_WIDTH		256
#define SCREEN_HEIGHT		240
// pixel aspect should be 8:7
#define WINDOW_HEIGHT_ASPECT	210
#define HAS_N2A03

// device informations for win32
#define SUPPORT_TV_RENDER
#define USE_BOOT_MODE		6
#define USE_TAPE		1
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	8
#define USE_AUTO_KEY_NO_CAPS
#define USE_SOUND_VOLUME	4
#define USE_JOYSTICK
#define USE_JOY_BUTTON_CAPTIONS
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_N2A03
#define USE_VM_AUTO_KEY_TABLE
#define USE_TWO_STROKE_AUTOKEY_HANDAKUON
#define USE_TWO_STROKE_AUTOKEY_DAKUON

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("APU"), _T("VRC7"), _T("CMT (Signal)"), _T("Noise (CMT)"),
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
	_T("Select"),
	_T("Start"),
};
#endif

#ifdef USE_VM_AUTO_KEY_TABLE
static const int vm_auto_key_table_base[][2] = {
	// A,I,U,E,O -> 1,2,3,4,5
	{0xb1, 0x200 | 0x31},
	{0xb2, 0x200 | 0x32},
	{0xb3, 0x200 | 0x33},
	{0xb4, 0x200 | 0x34},
	{0xb5, 0x200 | 0x35},
	// KA,KI,KU,KE,KO -> Q,W,E,R,T
	{0xb6, 0x200 | ((int)'Q')},
	{0xb7, 0x200 | ((int)'W')},
	{0xb8, 0x200 | ((int)'E')},
	{0xb9, 0x200 | ((int)'R')},
	{0xba, 0x200 | ((int)'T')},
	// SA,SI,SU,SE,SO -> A,S,D,F,G
	{0xbb, 0x200 | ((int)'A')},
	{0xbc, 0x200 | ((int)'S')},
	{0xbd, 0x200 | ((int)'D')},
	{0xbe, 0x200 | ((int)'F')},
	{0xbf, 0x200 | ((int)'G')},
	// TA,TI,TU,TE,TO -> Z, X, C, V, B
	{0xc0, 0x200 | ((int)'Z')},
	{0xc1, 0x200 | ((int)'X')},
	{0xc2, 0x200 | ((int)'C')},
	{0xc3, 0x200 | ((int)'V')},
	{0xc4, 0x200 | ((int)'B')},
	// NA,NI,NU,NE,NO -> 6,7,8,9,0
	{0xc5, 0x200 | 0x36},
	{0xc6, 0x200 | 0x37},
	{0xc7, 0x200 | 0x38},
	{0xc8, 0x200 | 0x39},
	{0xc9, 0x200 | 0x30},
	// HA,HI,HU,HE,HO -> Y,U,I,O,P
	{0xca, 0x200 | ((int)'Y')},
	{0xcb, 0x200 | ((int)'U')},
	{0xcc, 0x200 | ((int)'I')},
	{0xcd, 0x200 | ((int)'O')},
	{0xce, 0x200 | ((int)'P')},
	// MA,MI,MU,ME,MO -> H,J,K,L,;
	{0xcf, 0x200 | ((int)'H')},
	{0xd0, 0x200 | ((int)'J')},
	{0xd1, 0x200 | ((int)'K')},
	{0xd2, 0x200 | ((int)'L')},
	{0xd3, 0x200 | ((int)';')},
	// YA,YU,YO,WA,W0,NN -> N,M,,,.,/,_
	{0xd4, 0x200 | ((int)'N')},
	{0xd5, 0x200 | ((int)'M')},
	{0xd6, 0x200 | 0xbc},
	{0xdc, 0x200 | 0xbe},
	{0xa6, 0x200 | 0xbf},
	{0xdd, 0x200 | 0xe2},
	// RA,RI,RU,RE,RO -> -, ^, ¥, @, [
	{0xd7, 0x200 | 0xbd},
	{0xd8, 0x200 | 0xde},
	{0xd9, 0x200 | 0xdc},
	{0xda, 0x200 | 0xc0},
	{0xdb, 0x200 | 0xdb},
	// XA,XI,XU,XE,XO -> SHIFT+1, SHIFT+2, SHIFT+3, SHIFT+4, SHIFT+5
	{0xa7, 0x300 | 0x31},
	{0xa8, 0x300 | 0x32},
	{0xa9, 0x300 | 0x33},
	{0xaa, 0x300 | 0x34},
	{0xab, 0x300 | 0x35},
	// XYA,XYU,XYO -> SHIFT+N,SHIFT+M,SHIFT+,
	{0xac, 0x300 | ((int)'N')},
	{0xad, 0x300 | ((int)'M')},
	{0xae, 0x300 | 0xbc},
	// XTU -> SHIFT + C
	{0xaf, 0x300 | ((int)'C')},
	// _, MARU -> :, ]
	{0x5f, 0x200 | 0xbd},
	{0xa1, 0x200 | 0xdd},
	// KAGIKAKKO
	{0xa2, 0x300 | 0xbd},
	{0xa3, 0x300 | 0xdd},
	// 'ﾞ' -> Double Quotation
//	{0xde,	0x100 | 0x32},	
	{-1, -1}
};
#endif
#ifdef USE_TWO_STROKE_AUTOKEY_HANDAKUON
static const int kana_handakuon_keyboard_table[][6] = {
	// PA,PI,PU,PE,PO -> Y,U,I,O,P
	{0xca, 0x300 | ((int)'Y'), 0x00, 0x00, 0x00, 0x00},
	{0xcb, 0x300 | ((int)'U'), 0x00, 0x00, 0x00, 0x00},
	{0xcc, 0x300 | ((int)'I'), 0x00, 0x00, 0x00, 0x00},
	{0xcd, 0x300 | ((int)'O'), 0x00, 0x00, 0x00, 0x00},
	{0xce, 0x300 | ((int)'P'), 0x00, 0x00, 0x00, 0x00},
	{-1, -1}
};
#endif
#ifdef USE_TWO_STROKE_AUTOKEY_DAKUON
static const int kana_dakuon_keyboard_table[][6] = {
	// GA,GI,GU,GE,GO -> Q,W,E,R,T
	{0xb6, 0x1000 | 0x12, 0x200 | ((int)'Q'), 0x2000 | 0x12, 0x00, 0x00},
	{0xb7, 0x1000 | 0x12, 0x200 | ((int)'W'), 0x2000 | 0x12, 0x00, 0x00},
	{0xb8, 0x1000 | 0x12, 0x200 | ((int)'E'), 0x2000 | 0x12, 0x00, 0x00},
	{0xb9, 0x1000 | 0x12, 0x200 | ((int)'R'), 0x2000 | 0x12, 0x00, 0x00},
	{0xba, 0x1000 | 0x12, 0x200 | ((int)'T'), 0x2000 | 0x12, 0x00, 0x00},
	// ZA,ZI,ZU,ZE,ZO -> A,S,D,F,G
	{0xbb, 0x1000 | 0x12, 0x200 | ((int)'A'), 0x2000 | 0x12, 0x00, 0x00},
	{0xbc, 0x1000 | 0x12, 0x200 | ((int)'S'), 0x2000 | 0x12, 0x00, 0x00},
	{0xbd, 0x1000 | 0x12, 0x200 | ((int)'D'), 0x2000 | 0x12, 0x00, 0x00},
	{0xbe, 0x1000 | 0x12, 0x200 | ((int)'F'), 0x2000 | 0x12, 0x00, 0x00},
	{0xbf, 0x1000 | 0x12, 0x200 | ((int)'G'), 0x2000 | 0x12, 0x00, 0x00},
	// DA,DI,DU,DE,DO -> Z, X, C, V, B
	{0xc0, 0x1000 | 0x12, 0x200 | ((int)'Z'), 0x2000 | 0x12, 0x00, 0x00},
	{0xc1, 0x1000 | 0x12, 0x200 | ((int)'X'), 0x2000 | 0x12, 0x00, 0x00},
	{0xc2, 0x1000 | 0x12, 0x200 | ((int)'C'), 0x2000 | 0x12, 0x00, 0x00},
	{0xc3, 0x1000 | 0x12, 0x200 | ((int)'V'), 0x2000 | 0x12, 0x00, 0x00},
	{0xc4, 0x1000 | 0x12, 0x200 | ((int)'B'), 0x2000 | 0x12, 0x00, 0x00},
	// BA,BI,BU,BE,BO -> Y,U,I,O,P
	{0xca, 0x1000 | 0x12, 0x200 | ((int)'Y'), 0x2000 | 0x12, 0x00, 0x00},
	{0xcb, 0x1000 | 0x12, 0x200 | ((int)'U'), 0x2000 | 0x12, 0x00, 0x00},
	{0xcc, 0x1000 | 0x12, 0x200 | ((int)'I'), 0x2000 | 0x12, 0x00, 0x00},
	{0xcd, 0x1000 | 0x12, 0x200 | ((int)'O'), 0x2000 | 0x12, 0x00, 0x00},
	{0xce, 0x1000 | 0x12, 0x200 | ((int)'P'), 0x2000 | 0x12, 0x00, 0x00},
	{-1, -1}
};
#endif
	
typedef struct header_s {
	uint8_t id[3];	// 'NES'
	uint8_t ctrl_z;	// control-z
	uint8_t dummy;
	uint8_t num_8k_vrom_banks;
	uint8_t flags_1;
	uint8_t flags_2;
	uint8_t reserved[8];
	uint32_t num_16k_rom_banks()
	{
		return (dummy != 0) ? dummy : 256;
	}
	uint32_t num_8k_rom_banks()
	{
		return num_16k_rom_banks() * 2;
	}
	uint8_t mapper()
	{
		return (flags_1 >> 4) | (flags_2 & 0xf0);
	}
} header_t;

class EMU;
class DEVICE;
class EVENT;

class DATAREC;
class N2A03;
class YM2413;

namespace FAMILYBASIC {
	class MEMORY;
	class APU;
	class PPU;
}

class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	//EVENT* event;
	
	DATAREC* drec;
	N2A03* cpu;
	YM2413* opll;
	
	FAMILYBASIC::MEMORY* memory;
	FAMILYBASIC::APU* apu;
	FAMILYBASIC::PPU* ppu;
	
	int boot_mode;
	
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
	
	// user interface
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
	bool is_frame_skippable();
	
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

#endif
