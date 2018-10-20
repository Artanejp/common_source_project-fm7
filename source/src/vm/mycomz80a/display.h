/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.18-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_ADDR_L	0
#define SIG_DISPLAY_ADDR_H	1
#define SIG_DISPLAY_MODE	2

class HD46505;

class DISPLAY : public DEVICE
{
private:
	HD46505* d_crtc;
	uint8_t* regs;
	bool chr, wide;
	uint16_t cursor, cblink;
	
	uint8_t screen[200][640];
	uint8_t font[0x800], cg[0x800];
	uint8_t vram[0x800];
	uint16_t vram_addr;
	
	void draw_40column();
	void draw_80column();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_crtc(HD46505* device)
	{
		d_crtc = device;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

