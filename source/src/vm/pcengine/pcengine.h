/*
	NEC-HE PC Engine Emulator 'ePCEngine'

	Author : Takeda.Toshiya
	Date   : 2012.10.31-

	[ virtual machine ]
*/

#ifndef _PCENGINE_H_
#define _PCENGINE_H_

#define DEVICE_NAME		"NEC-HE PC Engine"
#define CONFIG_NAME		"pcengine"

#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME 	262
#define CPU_CLOCKS		7159090
#define SCREEN_WIDTH		352
#define SCREEN_HEIGHT		240
// pixel aspect should be 8:7
#define WINDOW_HEIGHT_ASPECT	210

#define SUPPORT_SUPER_GFX
#define SUPPORT_BACKUP_RAM
#define SUPPORT_CDROM
//#define SCSI_HOST_AUTO_ACK
#define SCSI_DEV_IMMEDIATE_SELECT

// device informations for win32
#define SOUND_RATE_DEFAULT	5	// 44100Hz
#define USE_CART		1
#define USE_COMPACT_DISC	1
#define USE_SOUND_VOLUME	3
#define USE_JOYSTICK
#define USE_JOYSTICK_TYPE	4
#define JOYSTICK_TYPE_DEFAULT	0
#define USE_JOY_BUTTON_CAPTIONS
#define USE_DEBUGGER
#define USE_STATE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("PSG"), _T("CD-DA"), _T("ADPCM")
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
	_T("Run"),
	_T("Button #3"),
	_T("Button #4"),
	_T("Button #5"),
	_T("Button #6"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class HUC6280;
class MSM5205;
class SCSI_HOST;
class SCSI_CDROM;
class PCE;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* pceevent;
	
	HUC6280* pcecpu;
	MSM5205* adpcm;
	SCSI_HOST* scsi_host;
	SCSI_CDROM* scsi_cdrom;
	PCE* pce;
	
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
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
	void open_compact_disc(int drv, const _TCHAR* file_path);
	void close_compact_disc(int drv);
	bool is_compact_disc_inserted(int drv);
	uint32_t is_compact_disc_accessed();
	bool is_frame_skippable()
	{
		return false;
	}
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
