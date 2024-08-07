/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ and gate ]
*/

#ifndef _AND_H_
#define _AND_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_AND_BIT_0	0x01
#define SIG_AND_BIT_1	0x02
#define SIG_AND_BIT_2	0x04
#define SIG_AND_BIT_3	0x08
#define SIG_AND_BIT_4	0x10
#define SIG_AND_BIT_5	0x20
#define SIG_AND_BIT_6	0x40
#define SIG_AND_BIT_7	0x80

class VM_TEMPLATE;
class EMU_TEMPLATE;
class DLL_PREFIX AND : public DEVICE
{
private:
	outputs_t outputs;
	uint32_t bits_mask, bits_in;
	bool prev, first;
	
public:
	AND(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
		bits_mask = bits_in = 0;
		prev = first = true;
		set_device_name(_T("AND GATE"));
	}
	~AND() {}
	
	// common functions
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_out(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs, device, id, mask);
	}
	void set_mask(uint32_t mask)
	{
		bits_mask |= mask;
	}
};

#endif

