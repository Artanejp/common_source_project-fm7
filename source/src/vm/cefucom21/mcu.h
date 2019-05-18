/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ MCU02 (main control unit ???) ]
*/

#ifndef _MCU_H_
#define _MCU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MCU_SYSPORT	0

class MCU : public DEVICE
{
private:
	DEVICE *d_drec, *d_psg, *d_vdp;
	
	uint8_t key_status[16];
	uint8_t system_port;
	
public:
	MCU(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MCU02"));
	}
	~MCU() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void set_context_vdp(DEVICE* device)
	{
		d_vdp = device;
	}
};

#endif

