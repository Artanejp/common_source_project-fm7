/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ YM2612 ]
*/

#ifndef _YM2612_H_
#define _YM2612_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"
#include "fmgen/opna.h"

#ifdef SUPPORT_WIN32_DLL
#define SUPPORT_MAME_FM_DLL
//#include "fmdll/fmdll.h"
#endif
#define SIG_YM2612_MUTE		2

class DEBUGGER;

class YM2612 : public DEVICE
{
protected:
	DEBUGGER *d_debugger;
	FM::OPN2* opn2;
#ifdef SUPPORT_MAME_FM_DLL
//	CFMDLL* fmdll;
	LPVOID* dllchip;
#endif
	struct {
		bool written;
		uint8_t data;
	} port_log[0x200];
	int base_decibel_fm, base_decibel_psg;
	int decibel_vol;
	
	uint8_t ch;
	uint8_t fnum2;
	uint8_t ch1, data1;
	uint8_t fnum21;

	int32_t right_volume;
	int32_t left_volume;
	int32_t v_right_volume;
	int32_t v_left_volume;
	struct {
		uint8_t wreg;
		uint8_t rreg;
		bool first;
		// output signals
		outputs_t outputs;
	} port[2];
	uint8_t mode;
	
	int chip_clock;
	bool irq_prev, mute;
	
	uint32_t clock_prev;
	uint32_t clock_accum;
	uint32_t clock_const;
	int timer_event_id;
	
	uint32_t clock_busy;
	bool busy;
	bool addr_A1;
	
	virtual void update_count();
	virtual void update_event();
	virtual void update_interrupt();
	outputs_t outputs_irq;
public:
	YM2612(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		base_decibel_fm = base_decibel_psg = 0;
		decibel_vol = 0 + 5;
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		d_debugger = NULL;
		initialize_output_signals(&outputs_irq);
 		set_device_name(_T("YM2612 OPN2"));
	}
	~YM2612() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual void event_vline(int v, int clock);
	virtual void event_callback(int event_id, int error);
	virtual void mix(int32_t* buffer, int cnt);
	virtual void set_volume(int _ch, int decibel_l, int decibel_r);
	virtual void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	// for debugging
	virtual void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	virtual uint64_t get_debug_data_addr_space()
	{
		return 0x200;
	}
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < 0x200) {
			write_via_debugger_data8(addr, data);
		}
	}
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr)
	{
		if(addr < 0x200) {
			return read_via_debugger_data8(addr);
		}
		return 0;
	}
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	
	virtual bool process_state(FILEIO* state_fio, bool loading);
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	virtual void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	virtual void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg);
	virtual void set_reg(uint32_t addr, uint32_t data); // for patch
};

#endif

