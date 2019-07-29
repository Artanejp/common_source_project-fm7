/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.07.29-

	[ dipswitch ]
*/

#ifndef _PC9801_DIPSW_H_
#define _PC9801_DIPSW_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class I8255;
namespace PC9801 {
class DIPSWITCH : public DEVICE
{
protected:
	uint8_t sw1;
	uint8_t sw2;
	uint8_t sw3;
	I8255* pio_prn;
	I8255* pio_sys;
	I8255* pio_mouse;

	void update_dipswitch();
	void update_ports();
public:
	DIPSWITCH(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		sw1 = 0;
		sw2 = 0;
		sw3 = 0;
		pio_sys = NULL;
		pio_prn = NULL;
		pio_mouse = NULL;
		set_device_name(_T("DIP SWITCHES"));
	}
	~DIPSWITCH() { }
	void initialize();
	void reset();
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_pio_sys(I8255* dev)
	{
		pio_sys = dev;
	}
	void set_context_pio_prn(I8255* dev)
	{
		pio_prn = dev;
	}
	void set_context_pio_mouse(I8255* dev)
	{
		pio_mouse = dev;
	}
};


}

#endif /* _PC9801_DIPSW_H_ */
