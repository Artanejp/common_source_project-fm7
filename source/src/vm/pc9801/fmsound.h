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

	Author : Takeda.Toshiya
	Date   : 2012.02.03-

	[ PC-9801-26/86 ]
*/

#ifndef _FMSOUND_H_
#define _FMSOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#ifdef SUPPORT_PC98_OPNA
class FIFO;
#endif

namespace PC9801 {

class FMSOUND : public DEVICE
{
private:
	DEVICE* d_opn;

	outputs_t outputs_int_pcm;
#if defined(SUPPORT_PC98_OPNA)
	uint8_t opna_mask;
#if defined(SUPPORT_PC98_86PCM)
	DEVICE *d_pic;
	uint64_t pcm_clocks;
	uint32_t pcm_prev_clock;
	uint8_t pcm_vol_ctrl, pcm_fifo_ctrl, pcm_dac_ctrl, pcm_mute_ctrl;
	int pcm_fifo_size;
	bool pcm_fifo_written, pcm_overflow, pcm_irq_raised;
	FIFO *pcm_fifo;
	int pcm_register_id;
	int pcm_sample_l, pcm_sample_r;
	int pcm_volume, pcm_volume_l, pcm_volume_r;
	
	int get_sample();
#endif
#endif
	int sample_rate;
	int sample_samples;
	int volume_r;
	int volume_l;
	
	void check_fifo_position();
public:
	FMSOUND(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_int_pcm);
		d_opn = NULL;
#if defined(SUPPORT_PC98_OPNA)
	#if defined(SUPPORT_PC98_86PCM)
		d_pic = NULL;
		pcm_fifo = NULL;
	#endif
		set_device_name(_T("PC-9801-86 (FM Sound)"));
#else
		set_device_name(_T("PC-9801-26 (FM Sound)"));
#endif
	}
	~FMSOUND() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL mix(int32_t* buffer, int cnt);
	void __FASTCALL event_callback(int id, int err);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void initialize_sound(int rate, int samples);
#if defined(SUPPORT_PC98_OPNA)
	bool process_state(FILEIO* state_fio, bool loading);
#endif
	// unique function
	void set_context_opn(DEVICE* device)
	{
		d_opn = device;
	}
	void set_context_pcm_int(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_int_pcm, device, id, mask); 
	}
#if defined(SUPPORT_PC98_OPNA) && defined(SUPPORT_PC98_86PCM)
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void initialize_sound(int rate, int frequency, int volume)
	{
		pcm_volume = volume;
		initialize_sound(rate, frequency);
	}
#endif
};

}
#endif

