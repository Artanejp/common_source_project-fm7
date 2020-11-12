/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADPCM RF6C68 with RAM]
*/
#pragma once

/*
  DAC: DELTA-SIGMA ADPCM
    I/O: 0-8
    MEMORY: 64KB (banked per 4kb).
*/
#include "../device.h"
#include "../../common.h"

#define SIG_RF5C68_DAC_PERIOD     0x01
#define SIG_RF5C68_CLEAR_INTR     0x02
#define SIG_RF5C68_MUTE           0x03
#define SIG_RF5C68_REG_ON         0x04
#define SIG_RF5C68_REG_BANK       0x05
#define SIG_RF5C68_REG_CH         0x06
#define SIG_RF5C68_SET_ALL_INTR   0x07
#define SIG_RF5C68_REG_ADDR_ST    0x20
#define SIG_RF5C68_REG_ADDR       0x30
#define SIG_RF5C68_REG_ENV        0x38
#define SIG_RF5C68_REG_LPAN       0x40
#define SIG_RF5C68_REG_RPAN       0x48
#define SIG_RF5C68_REG_LS         0x50
#define SIG_RF5C68_REG_FD         0x58
#define SIG_RF5C68_FORCE_LOAD     0x60

class DEBUGGER;
class RF5C68 : public DEVICE {
protected:
	DEBUGGER *d_debugger;
	outputs_t interrupt_boundary;

	// DAC
	uint8_t wave_memory[0x10000];
	// DAC REGISTERS
	bool dac_on;
	uint16_t dac_bank;
	uint8_t dac_ch;
	bool is_mute;

	double dac_rate;
	int mix_factor;
	int mix_count;
	int sample_words;
	int sample_pointer;
	int read_pointer;
	
	__DECL_ALIGNED(16) bool dac_onoff[8];
	__DECL_ALIGNED(16) pair32_t dac_addr_st[8];
	__DECL_ALIGNED(16) uint32_t dac_addr[8];
	__DECL_ALIGNED(16) uint32_t dac_env[8];
	__DECL_ALIGNED(16) uint32_t dac_pan[16];
	__DECL_ALIGNED(16) pair32_t dac_ls[8];
	__DECL_ALIGNED(16) pair32_t dac_fd[8];
	

	// TMP Values
	bool dac_force_load[8];
	__DECL_ALIGNED(16) int32_t  dac_tmpval[16];

	int volume_l, volume_r;
	int32_t* sample_buffer;

	int sample_length;
	
	int mix_rate;
	double sample_tick_us;

	// ToDo: Work correct LPF.

	void __FASTCALL get_sample(int32_t *v, int words);
	void __FASTCALL lpf_threetap(int32_t *v, int &lval, int &rval);
	
public:
	RF5C68(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		sample_buffer = NULL;
		sample_length = 0;
		mix_rate = 1; // For Error
		sample_tick_us = 0.0;
		is_mute = true;
		initialize_output_signals(&interrupt_boundary);
		d_debugger = NULL;

		dac_rate = 8.0e6 / 384;
		set_device_name(_T("ADPCM RF5C68"));
	}
	~RF5C68() {}

	void initialize();
	void release();
	void reset();

	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	
	uint32_t  __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);

	void __FASTCALL mix(int32_t* buffer, int cnt);
	void initialize_sound(int sample_rate, int samples);

	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);

	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr);

	void set_dac_rate(double freq)
	{
		dac_rate = freq;
		sample_words = 0;
		sample_pointer = 0;
		mix_factor = (int)(dac_rate * 4096.0 / (double)mix_rate);
		mix_count = 0;
		if((sample_buffer != NULL) && (sample_length > 0)) {
			memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
		}
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	bool is_debugger_available()
	{
		return ((d_debugger != NULL) ? true : false);
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	virtual uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	
	void set_context_interrupt_boundary(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&interrupt_boundary, device, id, mask);
	}

};
