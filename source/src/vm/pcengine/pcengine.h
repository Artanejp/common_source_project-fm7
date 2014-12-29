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
#define SCREEN_HEIGHT		238

#define SUPPORT_SUPER_GFX
#define SUPPORT_BACKUP_RAM

// device informations for win32
#define USE_CART1
#define USE_STATE

#include "../../common.h"

class EMU;
class DEVICE;
class EVENT;

class HUC6280;
class PCE;

class FILEIO;

class VM
{
protected:
	EMU* emu;
	
	// devices
	EVENT* pceevent;
	
	HUC6280* pcecpu;
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
	double frame_rate();
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	// user interface
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
	bool now_skip()
	{
		return false;
	}
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
	DEVICE* dummy;
	DEVICE* first_device;
	DEVICE* last_device;
};

#endif
