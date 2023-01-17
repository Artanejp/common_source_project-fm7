/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_PA	0
#define SIG_DISPLAY_PB	1

namespace CRC80 {
class DISPLAY : public DEVICE
{
private:
	DEVICE *d_pio;
	
	uint8_t led[16];
	uint8_t pa, pb;
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
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

