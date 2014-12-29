/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2011.12.27-

	[ mouse ]
*/

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MOUSE_PORT_C	0

class MOUSE : public DEVICE
{
private:
	DEVICE *d_pic, *d_pio;
	
	// mouse
	int* status;
	int ctrlreg, freq, cur_freq, dx, dy, lx, ly;
	int register_id;
	
	void update_mouse();
	
public:
	MOUSE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MOUSE() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void event_callback(int event_id, int err);
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

