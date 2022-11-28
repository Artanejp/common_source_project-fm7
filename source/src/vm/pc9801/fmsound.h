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

class FMSOUND : public DEVICE
{
private:
	DEVICE* d_opn;
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
	
public:
	FMSOUND(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#if defined(SUPPORT_PC98_OPNA)
		set_device_name(_T("PC-9801-86 (FM Sound)"));
#else
		set_device_name(_T("PC-9801-26 (FM Sound)"));
#endif
	}
	~FMSOUND() {}
	
	// common functions
#if defined(SUPPORT_PC98_OPNA)
	void reset();
#if defined(SUPPORT_PC98_86PCM)
	void initialize();
	void release();
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
#endif
#endif
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#if defined(SUPPORT_PC98_OPNA)
	bool process_state(FILEIO* state_fio, bool loading);
#endif
	
	// unique function
	void set_context_opn(DEVICE* device)
	{
		d_opn = device;
	}
#if defined(SUPPORT_PC98_OPNA) && defined(SUPPORT_PC98_86PCM)
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void initialize_sound(int rate, double frequency, int volume)
	{
		pcm_volume = volume;
	}
#endif
};

#endif

