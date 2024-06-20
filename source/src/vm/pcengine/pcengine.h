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
#define _SCSI_DEBUG_LOG
#define _CDROM_DEBUG_LOG

//#define SCSI_HOST_AUTO_ACK
#define SCSI_DEV_IMMEDIATE_SELECT

// device informations for win32
#define SOUND_RATE_DEFAULT	5	// 44100Hz
#define SUPPORT_TV_RENDER
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

namespace PCEDEV {
	class PCE;
	class ADPCM;
}
class VM : public VM_TEMPLATE
{
protected:
	//EMU* emu;
	//csp_state_utils* state_entry;
	
	// devices
	EVENT* pceevent;
	
	HUC6280* pcecpu;
	MSM5205* adpcm;
	SCSI_HOST* scsi_host;
	SCSI_CDROM* scsi_cdrom;
	PCEDEV::PCE* pce;
	PCEDEV::ADPCM* pce_adpcm;
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
	bool run() override;
	double get_frame_rate() override;
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index) override;
#endif
	
	// draw screen
	void draw_screen() override;
	
	// sound generation
	void initialize_sound(int rate, int samples) override;
	uint16_t* create_sound(int* extra_frames) override;
	int get_sound_buffer_ptr() override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r) override;
#endif
	
	// user interface
	void open_cart(int drv, const _TCHAR* file_path) override;
	void close_cart(int drv) override;
	bool is_cart_inserted(int drv) override;
	void open_compact_disc(int drv, const _TCHAR* file_path) override;
	void close_compact_disc(int drv) override;
	bool is_compact_disc_inserted(int drv) override;
	uint32_t is_compact_disc_accessed() override;
	bool is_frame_skippable() override
	{
		return false;
	}
	
	double get_current_usec() override;
	uint64_t get_current_clock_uint64() override;
	
	void update_config() override;
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id) override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
