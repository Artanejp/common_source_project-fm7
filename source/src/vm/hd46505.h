/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ HD46505 ]
*/

#ifndef _HD46505_H_
#define _HD46505_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class HD46505 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_disp;
	outputs_t outputs_vblank;
	outputs_t outputs_vsync;
	outputs_t outputs_hsync;
	
	uint8 regs[18];
	int ch;
	bool timing_changed;
	
	int cpu_clocks;
#ifdef HD46505_HORIZ_FREQ
	int horiz_freq, next_horiz_freq;
#endif
	double frames_per_sec;
	
	int hz_total, hz_disp;
	int hs_start, hs_end;
	
	int vt_total, vt_disp;
	int vs_start, vs_end;
	
	int disp_end_clock;
	int hs_start_clock, hs_end_clock;
	
	bool display, vblank, vsync, hsync;
	
	void set_display(bool val);
	void set_vblank(bool val);
	void set_vsync(bool val);
	void set_hsync(bool val);
	
public:
	HD46505(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_disp);
		init_output_signals(&outputs_vblank);
		init_output_signals(&outputs_vsync);
		init_output_signals(&outputs_hsync);
	}
	~HD46505() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_pre_frame();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_disp(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_disp, device, id, mask);
	}
	void set_context_vblank(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_vblank, device, id, mask);
	}
	void set_context_vsync(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_vsync, device, id, mask);
	}
	void set_context_hsync(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_hsync, device, id, mask);
	}
#ifdef HD46505_HORIZ_FREQ
	void set_horiz_freq(int freq)
	{
		next_horiz_freq = freq;
	}
#endif
	uint8* get_regs()
	{
		return regs;
	}
};

#endif

