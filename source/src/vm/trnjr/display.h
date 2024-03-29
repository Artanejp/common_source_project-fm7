/*
	EPS TRN Junior Emulator 'eTRNJunior'

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ display / keyboard ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm_template.h"
#include "../../emu_template.h"
#include "../device.h"

#define SIG_DISPLAY_PORT_A	0
#define SIG_DISPLAY_PORT_C	1

namespace TRNJR {
class DISPLAY : public DEVICE
{
private:
	DEVICE *d_pio;
	
	int seg[8][8];
	uint8_t pa, pc;
	
	void update_keyboard();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display/Keyboard"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_vline(int v, int clock) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void draw_screen();
};

}
#endif

