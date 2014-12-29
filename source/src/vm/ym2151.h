/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.03.08-

	[ YM2151 ]
*/

#ifndef _YM2151_H_
#define _YM2151_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "fmgen/opm.h"

#define SIG_YM2151_MUTE		0

class YM2151 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	
	FM::OPM* opm;
	
	int chip_clock;
	uint8 ch;
	bool irq_prev, mute;
	
	uint32 clock_prev;
	uint32 clock_accum;
	uint32 clock_const;
	
	uint32 clock_busy;
	bool busy;
	
	void update_count();
	void update_interrupt();
	
public:
	YM2151(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_irq);
	}
	~YM2151() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_vline(int v, int clock);
	void mix(int32* buffer, int cnt);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void init(int rate, int clock, int samples, int vol);
	void SetReg(uint addr, uint data); // for patch
};

#endif

