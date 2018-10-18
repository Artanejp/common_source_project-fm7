/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt/printer ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_IN		0
#define SIG_CMT_EOT		1
//#define SIG_PRINTER_BUSY	2

class CMT : public DEVICE
{
private:
	DEVICE* d_drec;
	
	// data recorder
	bool in, out, remote, eot;
	
	// printer
	uint8_t pout;
	bool strobe, busy;
	
	// reset/halt key
	const uint8_t* key_stat;
	
public:
	CMT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMT I/F"));
	}
	~CMT() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
};

#endif
