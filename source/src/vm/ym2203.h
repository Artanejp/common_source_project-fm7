/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / YM2203 / YM2608 ]
*/

#ifndef _YM2203_H_
#define _YM2203_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"
#include "fmgen/opna.h"

#ifdef SUPPORT_WIN32_DLL
#define SUPPORT_MAME_FM_DLL
//#include "fmdll/fmdll.h"
#endif
//#if defined(HAS_AY_3_8913)
// both port a and port b are not supported
//#elif defined(HAS_AY_3_8912)
// port b is not supported
//#define SUPPORT_YM2203_PORT_A
//#else
//#define SUPPORT_YM2203_PORT_A
//#define SUPPORT_YM2203_PORT_B
//#endif

//#ifdef SUPPORT_YM2203_PORT_A
#define SIG_YM2203_PORT_A	0
//#endif
//#ifdef SUPPORT_YM2203_PORT_B
#define SIG_YM2203_PORT_B	1
//#endif
#define SIG_YM2203_MUTE		2

class DEBUGGER;

class  DLL_PREFIX YM2203 : public DEVICE
{
private:
	DEBUGGER *d_debugger;
	FM::OPNA* opna;
	FM::OPN* opn;
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

	bool _HAS_YM_SERIES;
	bool _HAS_AY_3_8910;
	bool _HAS_AY_3_8912;
	bool _HAS_AY_3_8913;
	bool _SUPPORT_YM2203_PORT_A;
	bool _SUPPORT_YM2203_PORT_B;
	uint32_t _amask;
	void update_count();
	void update_event();
//#ifdef HAS_YM_SERIES
	// output signals
	outputs_t outputs_irq;
	void update_interrupt();
//#endif
public:
	YM2203(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		base_decibel_fm = base_decibel_psg = 0;
		decibel_vol = 0 + 5;
		_amask = 1;
		_HAS_AY_3_8910 = _HAS_AY_3_8912 = _HAS_AY_3_8913 = false;
		_HAS_YM_SERIES = false;
		_SUPPORT_YM2203_PORT_A = _SUPPORT_YM2203_PORT_B = false;
		is_ym2608 = false;
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		d_debugger = NULL;
		initialize_output_signals(&outputs_irq);
 		this_device_name[0] = _T('\0');
	}
	~YM2203() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int id);
	void event_vline(int v, int clock);
	void __FASTCALL event_callback(int event_id, int error);
	void __FASTCALL mix(int32_t* buffer, int cnt);
	void set_volume(int _ch, int decibel_l, int decibel_r);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	// for debugging
	void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
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
		return is_ym2608 ? 0x200 : 0x100;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < (uint32_t)(is_ym2608 ? 0x200 : 0x100)) {
			write_via_debugger_data8(addr, data);
		}
	}
	uint32_t __FASTCALL read_debug_data8(uint32_t addr)
	{
		if(addr < (uint32_t)(is_ym2608 ? 0x200 : 0x100)) {
			return read_via_debugger_data8(addr);
		}
		return 0;
	}
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	
	bool process_state(FILEIO* state_fio, bool loading);
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg);
	void set_reg(uint32_t addr, uint32_t data); // for patch
	bool is_ym2608;
};

#endif

