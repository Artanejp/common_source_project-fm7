/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ virtual machine ]
*/

#ifndef _MICOM_MAHJONG_H_
#define _MICOM_MAHJONG_H_

#define DEVICE_NAME		"MICOM MAHJONG"
#define CONFIG_NAME		"micom_mahjong"

// device informations for virtual machine
#define FRAMES_PER_SEC		60
#define LINES_PER_FRAME		262
#define CPU_CLOCKS		(11059200/4)
#define SCREEN_WIDTH		512
#define SCREEN_HEIGHT		400

// device informations for win32
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_SOUND_VOLUME	1
#define USE_DEBUGGER
#define USE_STATE
#define USE_CPU_Z80
#define USE_SCREEN_FILTER
#define USE_SCANLINE

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
	_T("BEEP"),
};
#endif


class EMU;
class DEVICE;
class EVENT;

class AND;
class PCM1BIT;
class Z80;

namespace MICOM_MAHJONG {
	class KEYBOARD;
	class MEMORY;
}

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;

	// devices
	EVENT* event;

	PCM1BIT* pcm;
	Z80* cpu;

	MICOM_MAHJONG::MEMORY* memory;
	MICOM_MAHJONG::KEYBOARD* keyboard;

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
	void reset()  override;
	double get_frame_rate()	override
	{
		return FRAMES_PER_SEC;
	}

#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index)  override;
#endif

	// draw screen
	void draw_screen()  override;

	// sound generation
	void initialize_sound(int rate, int samples)  override;
	uint16_t* create_sound(int* extra_frames)  override;
	int get_sound_buffer_ptr()  override;
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r)  override;
#endif

	// user interface

	bool is_frame_skippable()  override;

	void update_config()  override;
	bool process_state(FILEIO* state_fio, bool loading);

	// ----------------------------------------
	// for each device
	// ----------------------------------------

	// devices
	DEVICE* get_device(int id)  override;
	//DEVICE* dummy;
	//DEVICE* first_device;
	//DEVICE* last_device;
};

#endif
