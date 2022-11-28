/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ memory ]
*/

#ifndef _MICOM_MAHJONG_MEMORY_H_
#define _MICOM_MAHJONG_MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#include "./keyboard.h"

#define SIG_MEMORY_KEYDATA	0

class MEMORY : public DEVICE
{
private:
	DEVICE *d_keyboard;
	DEVICE *d_pcm;
	
	uint8_t prg[0x8000];	// prg rom (16k)
	uint8_t ram[0x0400];	// ram(1k)
	uint8_t vram[0x400];	// vram(1k)
	uint8_t cg[0x800];		// cg(2k)
	
	uint8_t key_strobe;
	uint8_t key_data;
	uint8_t speaker;
	
	// renderer
	uint8_t screen[200][320];
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void reset();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_keyboard(DEVICE* device)
	{
		d_keyboard = device;
	}

	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}

	void draw_line(int v);
	void draw_screen();

};
#endif

