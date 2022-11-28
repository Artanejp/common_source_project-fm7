/*
	Skelton for retropc emulator

	Origin : PockEmul
	Author : Takeda.Toshiya
	Date   : 2016.03.18-

	[ uPD16434 ]
*/

#ifndef _UPD16434_H_
#define _UPD16434_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class UPD16434 : public DEVICE
{
private:
	uint8_t pointer;
	uint8_t mode;
	uint8_t imem[0x80];
	
	void draw_char(uint8_t c, bool right);
	void cmd_mode(uint8_t cmd);
	void cmd_bset(uint8_t cmd, bool set);
	void update_pointer(uint8_t mode);
	
public:
	UPD16434(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("uPD16434 LCD Controller"));
	}
	~UPD16434() {}
	
	// common functions
	void reset();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void data(uint8_t data);
	void instruction(uint8_t cmd);
	void draw(int xoffset);
};

#endif
