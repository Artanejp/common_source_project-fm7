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

class RF5C68 : public DEVICE {
protected:
	outputs_t interrupt_boundary;

	// DAC
	uint8_t wave_memory[0x10000];
	// DAC REGISTERS
	bool dac_on;
	uint8_t dac_bank;
	uint8_t dac_ch;
	bool is_mute;
	
	bool dac_onoff[8];
	pair32_t dac_addr_st[8];
	uint32_t dac_addr[8];
	uint32_t dac_env[8];
	uint32_t dac_lpan[8];
	uint32_t dac_rpan[8];
	pair32_t dac_ls[8];
	pair32_t dac_fd[8];
	

	// TMP Values
	bool dac_force_load[8];
	int32_t  dac_tmpval_l[8];
	int32_t  dac_tmpval_r[8];

	int volume_l, volume_r;
	int32_t* sample_buffer;
	int event_dac_sample;

	int sample_length;
	int sample_count;
	
	int mix_rate;
	double sample_tick_us;
public:
	RF5C68(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		sample_buffer = NULL;
		sample_length = 0;
		sample_count = 0;
		mix_rate = 0;
		event_dac_sample = -1;
		sample_tick_us = 0.0;
		is_mute = true;
		initialize_output_signals(&interrupt_boundary);
		set_device_name(_T("ADPCM RF5C68"));
	}
	~RF5C68() {}

	void initialize();
	void release();
	void reset();

	uint32_t __FASTCALL read_data8(uint32_t addr);
	void __FASTCALL write_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);

	void event_callback(int id, int err);
	
	void mix(int32_t* buffer, int cnt);
	void initialize_sound(int sample_rate, int samples);

	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	void set_context_interrupt_boundary(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&interrupt_boundary, device, id, mask);
	}

};
