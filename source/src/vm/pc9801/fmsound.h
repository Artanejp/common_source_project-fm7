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

	[ PC-9801-26 ]
*/

#ifndef _FMSOUND_H_
#define _FMSOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#if defined(_PC98DOPLUS) || defined(_PC98GS) || defined(SUPPORT_PC98_OPNA)
#define _PC98_SUPPORT_EXTRA_SOUND
#define _PC98_HAVE_86PCM
#endif

#ifdef _PC98_HAVE_86PCM
class FIFO;
#endif
namespace PC9801 {

class FMSOUND : public DEVICE
{
private:
	DEVICE* d_opn;

	outputs_t outputs_int_pcm;
	uint8_t mask;
#ifdef _PC98_HAVE_86PCM
	FIFO* pcm_fifo;

	// ToDo: Implement volumes
	uint8_t pcm_volume_reg;
	uint8_t pcm_ctrl_reg;
	uint8_t pcm_da_reg;
	pair32_t pcm_data;
	
	bool fifo_enabled;
	bool fifo_direction;
	bool fifo_int_flag;
	bool fifo_int_status;
	bool fifo_reset_req;

	uint32_t pcm_freq;
	bool lrclock;
	bool pcm_is_16bit;
	bool pcm_l_enabled;
	bool pcm_r_enabled;
	int  pcm_da_intleft;

	int mix_mod;

	int play_bufsize;
	int play_w_remain;
	int play_rptr;
	int play_wptr;
	int32_t play_pool[65536]; // Internal buffer
	
	int event_pcm;
#endif

	int sample_rate;
	int sample_samples;
	int volume_r;
	int volume_l;
	
	void check_fifo_position();
public:
	FMSOUND(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_int_pcm);
#ifdef SUPPORT_PC98_OPNA
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
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	void mix(int32_t* buffer, int cnt);
	void event_callback(int id, int err);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void initialize_sound(int rate, int samples);
	
	// unique function
	void set_context_opn(DEVICE* device)
	{
		d_opn = device;
	}
	void set_context_pcm_int(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_int_pcm, device, id, mask); 
	}
};

}
#endif

