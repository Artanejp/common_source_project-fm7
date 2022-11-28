/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / AY-3-8912 / AY-3-8913 ]
*/

#ifndef _AY_3_891X_H_
#define _AY_3_891X_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "fmgen/opna.h"

#if defined(HAS_AY_3_8913)
// AY-3-8913: both port a and port b are not supported
#elif defined(HAS_AY_3_8912)
// AY-3-8912: port b is not supported
#define SUPPORT_AY_3_891X_PORT_A
#else
// AY-3-8910: both port a and port b are supported
#define SUPPORT_AY_3_891X_PORT_A
#define SUPPORT_AY_3_891X_PORT_B
#endif
#if defined(SUPPORT_AY_3_891X_PORT_A) || defined(SUPPORT_AY_3_891X_PORT_B)
#define SUPPORT_AY_3_891X_PORT
#endif

#ifdef SUPPORT_AY_3_891X_PORT_A
#define SIG_AY_3_891X_PORT_A	0
#endif
#ifdef SUPPORT_AY_3_891X_PORT_B
#define SIG_AY_3_891X_PORT_B	1
#endif
#define SIG_AY_3_891X_MUTE	2

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class AY_3_891X : public DEVICE
{
private:
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	FM::OPN* opn;
	int base_decibel_fm, base_decibel_psg;
	
	uint8_t ch;
	
#ifdef SUPPORT_AY_3_891X_PORT
	struct {
		uint8_t wreg;
		uint8_t rreg;
		bool first;
		// output signals
		outputs_t outputs;
	} port[2];
	uint8_t mode;
#endif
	
	int chip_clock;
	bool irq_prev, mute;
	
	uint32_t clock_prev;
	uint32_t clock_accum;
	uint32_t clock_const;
	int timer_event_id;
	
	uint32_t clock_busy;
	bool busy;
	
	void update_count();
	void update_event();
	
public:
	AY_3_891X(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef SUPPORT_AY_3_891X_PORT
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
#endif
		base_decibel_fm = base_decibel_psg = 0;
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
#if defined(HAS_AY_3_8910)
		set_device_name(_T("AY-3-8910 PSG"));
#elif defined(HAS_AY_3_8912)
		set_device_name(_T("AY-3-8912 PSG"));
#elif defined(HAS_AY_3_8913)
		set_device_name(_T("AY-3-8913 PSG"));
#else
		set_device_name(_T("AY-3-891X PSG"));
#endif
	}
	~AY_3_891X() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int error);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	// for debugging
	void write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data8(uint32_t addr);
#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint64_t get_debug_data_addr_space()
	{
		return 16;
	}
	void write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < 16) {
			write_via_debugger_data8(addr, data);
		}
	}
	uint32_t read_debug_data8(uint32_t addr)
	{
		if(addr < 16) {
			return read_via_debugger_data8(addr);
		}
		return 0;
	}
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#ifdef SUPPORT_AY_3_891X_PORT_A
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
#endif
#ifdef SUPPORT_AY_3_891X_PORT_B
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg);
	void set_reg(uint32_t addr, uint32_t data); // for patch
};

#endif

