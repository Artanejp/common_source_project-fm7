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
#include <atomic>
#include <mutex>

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
class DLL_PREFIX RF5C68 : public DEVICE {
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
	double lpf_cutoff;
	int mix_factor;
	int mix_count;

	int lastsample_l;
	int lastsample_r;

	bool _RF5C68_DRIVEN_BY_EXTERNAL_CLOCK;

	std::recursive_mutex m_locker;
	std::atomic<int> sample_words;
	std::atomic<int> sample_pointer;
	std::atomic<int> read_pointer;

	__DECL_ALIGNED(16) bool dac_onoff[8];
	__DECL_ALIGNED(16) pair32_t dac_addr_st[8];
	__DECL_ALIGNED(16) uint32_t dac_addr[8];
	__DECL_ALIGNED(16) uint32_t dac_env[16];
	__DECL_ALIGNED(16) uint32_t dac_pan[16];
	__DECL_ALIGNED(16) pair32_t dac_ls[8];
	__DECL_ALIGNED(16) pair32_t dac_fd[8];


	// TMP Values
	bool dac_force_load[8];
	__DECL_ALIGNED(16) int32_t  dac_tmpval[16];

	std::atomic<int> volume_l, volume_r;
	int32_t* sample_buffer;

	std::atomic<int> sample_length;
	std::atomic<int> mix_rate;

	int event_dac;
	int event_lpf;

	// ToDo: Work correct LPF.

	virtual void do_dac_period();

	virtual void __FASTCALL get_sample(int32_t *v, int words);
	void __FASTCALL lpf_threetap(int32_t *v, int &lval, int &rval);

public:
	RF5C68(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		sample_buffer = NULL;
		sample_length = 0;
		mix_rate = 48000; // For Error
		initialize_output_signals(&interrupt_boundary);
		d_debugger = NULL;

		dac_rate = 8.0e6 / 384; // About 19.2KHz
		lpf_cutoff = 4.0e3;
		_RF5C68_DRIVEN_BY_EXTERNAL_CLOCK = false;
		set_device_name(_T("ADPCM RF5C68"));
	}
	~RF5C68() {}

	virtual void initialize() override;
	virtual void release() override;

	virtual void reset() override;

	virtual void __FASTCALL event_callback(int id, int err) override;

	virtual uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait) override;

	virtual uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;
	virtual uint32_t  __FASTCALL read_debug_data8(uint32_t addr) override;
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;

	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;

	virtual uint32_t __FASTCALL read_signal(int ch) override;
	virtual void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask) override;

	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	void *get_debugger() override
	{
		return d_debugger;
	}
	bool is_debugger_available() override
	{
		return ((d_debugger != NULL) ? true : false);
	}
	virtual uint32_t get_debug_data_addr_mask() override
	{
		return 0xffff;
	}
	virtual void __FASTCALL mix(int32_t* buffer, int cnt) override;
	virtual void set_volume(int ch, int decibel_l, int decibel_r) override;
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	/*
	  unique functions
	*/
	virtual void initialize_sound(int sample_rate, int samples);

	virtual void set_lpf_cutoff(double freq)
	{
		lpf_cutoff = freq;
		//	calc_lpf_cutoff(freq);
	}
	virtual void set_dac_rate(double freq)
	{
		dac_rate = freq;
		sample_words = 0;
		sample_pointer = 0;
		//mix_factor = set_mix_factor(dac_rate, mix_rate);
		mix_count = 0;
		//calc_lpf_cutoff(lpf_cutoff);
		if((sample_buffer != NULL) && (sample_length > 0)) {
			memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
		}
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void set_context_interrupt_boundary(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&interrupt_boundary, device, id, mask);
	}

};
