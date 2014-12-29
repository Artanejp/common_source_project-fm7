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
#if defined(_WIN32)
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
	FM::OPNA* chip;
#else
	FM::OPN* chip;
#endif
#ifdef SUPPORT_MAME_FM_DLL
	CFMDLL* fmdll;
	LPVOID* dllchip;
	struct {
		bool written;
		uint8 data;
	} port_log[0x200];
#endif
	
	uint8 ch;
#ifdef SUPPORT_YM2203_PORT
	uint8 mode;
#endif
#ifdef HAS_YM2608
	uint8 ch1, data1;
#endif
	
#ifdef SUPPORT_YM2203_PORT
	struct {
		uint8 wreg;
		uint8 rreg;
		bool first;
		// output signals
		outputs_t outputs;
	} port[2];
#endif
	
	int chip_clock;
	bool irq_prev, mute;
	
	uint32 clock_prev;
	uint32 clock_accum;
	uint32 clock_const;
	
	uint32 clock_busy;
	bool busy;
	
	void update_count();
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
			init_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
#endif
#ifdef HAS_YM_SERIES
		init_output_signals(&outputs_irq);
#endif
	}
	~YM2203() {}
	
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
#ifdef HAS_YM_SERIES
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
#endif
#ifdef SUPPORT_YM2203_PORT_A
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
#endif
#ifdef SUPPORT_YM2203_PORT_B
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
#endif
	void init(int rate, int clock, int samples, int volf, int volp);
	void SetReg(uint addr, uint data); // for patch
};

#endif

