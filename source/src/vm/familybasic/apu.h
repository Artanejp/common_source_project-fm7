/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ APU ]
*/

#ifndef _APU_H_
#define _APU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define APUQUEUE_SIZE	4096
#define APUQUEUE_MASK	(APUQUEUE_SIZE - 1)

// rectangle
typedef struct {
	uint8_t regs[4];
	bool enabled;
	int32_t phaseacc;
	int32_t freq;
	int32_t output_vol;
	bool fixed_envelope;
	bool holdnote;
	uint8_t volume;
	int32_t sweep_phase;
	int32_t sweep_delay;
	bool sweep_on;
	uint8_t sweep_shifts;
	uint8_t sweep_length;
	bool sweep_inc;
	int32_t freq_limit;
	bool sweep_complement;
	int32_t env_phase;
	int32_t env_delay;
	uint8_t env_vol;
	int vbl_length;
	uint8_t adder;
	int duty_flip;
	bool enabled_cur;
	bool holdnote_cur;
	int vbl_length_cur;
} rectangle_t;

// triangle
typedef struct {
	uint8_t regs[3];
	bool enabled;
	int32_t freq;
	int32_t phaseacc;
	int32_t output_vol;
	uint8_t adder;
	bool holdnote;
	bool counter_started;
	int write_latency;
	int vbl_length;
	int linear_length;
	bool enabled_cur;
	bool holdnote_cur;
	bool counter_started_cur;
	int vbl_length_cur;
} triangle_t;

// noise
typedef struct {
	uint8_t regs[3];
	bool enabled;
	int32_t freq;
	int32_t phaseacc;
	int32_t output_vol;
	int32_t env_phase;
	int32_t env_delay;
	uint8_t env_vol;
	bool fixed_envelope;
	bool holdnote;
	uint8_t volume;
	int vbl_length;
	uint8_t xor_tap;
	bool enabled_cur;
	bool holdnote_cur;
	int vbl_length_cur;
	
	int shift_reg;
	int noise_bit;
} noise_t;

// dmc
typedef struct {
	uint8_t regs[4];
	bool enabled;
	int32_t freq;
	int32_t phaseacc;
	int32_t output_vol;
	uint32_t address;
	uint32_t cached_addr;
	int dma_length;
	int cached_dmalength;
	uint8_t cur_byte;
	bool looping;
	bool irq_gen;
	bool irq_occurred;
	int32_t freq_cur;
	int32_t phaseacc_cur;
	int dma_length_cur;
	int cached_dmalength_cur;
	bool enabled_cur;
	bool looping_cur;
	bool irq_gen_cur;
	bool irq_occurred_cur;
} dmc_t;

// queue
typedef struct {
	uint32_t timestamp, addr;
	uint32_t data;
} queue_t;

class APU : public DEVICE
{
private:
	DEVICE *d_cpu, *d_mem;
	
	rectangle_t rectangle[2];
	triangle_t triangle;
	noise_t noise;
	dmc_t dmc;
	
	int32_t cycle_rate;
	int32_t decay_lut[16];
	int vbl_lut[32];
	int trilength_lut[128];
	
	uint32_t enable_reg;
	uint32_t enable_reg_cur;
	int count_rate;
	
	queue_t queue[APUQUEUE_SIZE];
	int q_head, q_tail;
	uint32_t elapsed_cycles;
	double ave, max, min;
	
	int32_t create_rectangle(rectangle_t *chan);
	int32_t create_triangle(triangle_t *chan);
	int32_t create_noise(noise_t *chan);
	inline void dmc_reload(dmc_t *chan);
	int32_t create_dmc(dmc_t *chan);
	void enqueue(queue_t *d);
	queue_t* dequeue();
	void write_data_sync(uint32_t addr, uint32_t data);
	void write_data_cur(uint32_t addr, uint32_t data);
	
	int volume_l, volume_r;
	
public:
	APU(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("APU"));
	}
	~APU() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void event_frame();
	void event_vline(int v, int clock);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void initialize_sound(int rate, int samples);
};

#endif

