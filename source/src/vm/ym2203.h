/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / YM2203 / YM2608 ]
*/

#ifndef _YM2203_H_
#define _YM2203_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "fmgen/opna.h"

#if !(defined(HAS_AY_3_8910) || defined(HAS_AY_3_8912) || defined(HAS_AY_3_8913))
#define HAS_YM_SERIES
#ifdef SUPPORT_WIN32_DLL
#define SUPPORT_MAME_FM_DLL
#include "fmdll/fmdll.h"
#endif
#endif

#if defined(HAS_AY_3_8913)
// both port a and port b are not supported
#elif defined(HAS_AY_3_8912)
// port b is not supported
#define SUPPORT_YM2203_PORT_A
#else
#define SUPPORT_YM2203_PORT_A
#define SUPPORT_YM2203_PORT_B
#endif
#if defined(SUPPORT_YM2203_PORT_A) || defined(SUPPORT_YM2203_PORT_B)
#define SUPPORT_YM2203_PORT
#endif

#ifdef SUPPORT_YM2203_PORT_A
#define SIG_YM2203_PORT_A	0
#endif
#ifdef SUPPORT_YM2203_PORT_B
#define SIG_YM2203_PORT_B	1
#endif
#define SIG_YM2203_MUTE		2

class YM2203 : public DEVICE
{
private:
#ifdef HAS_YM2608
	FM::OPNA* opna;
#endif
	FM::OPN* opn;
#ifdef SUPPORT_MAME_FM_DLL
	CFMDLL* fmdll;
	LPVOID* dllchip;
	struct {
		bool written;
		uint8_t data;
	} port_log[0x200];
#endif
	int base_decibel_fm, base_decibel_psg;
	int decibel_vol;
	
	uint8_t ch;
	uint8_t fnum2;
#ifdef HAS_YM2608
	uint8_t ch1, data1;
	uint8_t fnum21;
#endif

	int32_t right_volume;
	int32_t left_volume;
	int32_t v_right_volume;
	int32_t v_left_volume;
#ifdef SUPPORT_YM2203_PORT
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
#ifdef HAS_YM_SERIES
	// output signals
	outputs_t outputs_irq;
	void update_interrupt();
#endif
public:
	YM2203(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef SUPPORT_YM2203_PORT
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
#endif
#ifdef HAS_YM_SERIES
		initialize_output_signals(&outputs_irq);
#endif
#ifdef HAS_YM2608
		is_ym2608 = true;
#endif
		base_decibel_fm = base_decibel_psg = 0;
		decibel_vol = 0 + 5;
#ifdef HAS_YM2608
		set_device_name(_T("YM2608 OPNA"));
#else		
		set_device_name(_T("YM2203 OPN"));
#endif		
	}
	~YM2203() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int error);
	void mix(int32* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	// unique functions
#ifdef HAS_YM_SERIES
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
#endif
#ifdef SUPPORT_YM2203_PORT_A
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
#endif
#ifdef SUPPORT_YM2203_PORT_B
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
#endif
	void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg);
	void set_reg(uint32_t addr, uint32_t data); // for patch
#ifdef HAS_YM2608
	bool is_ym2608;
#endif
};

#endif

