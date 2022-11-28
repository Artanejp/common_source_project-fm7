/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ APU ]
*/

#include <math.h>
#include "apu.h"

//#define APU_USE_QUEUE

#define APU_BASEFREQ   1789772.5
#define APU_TO_FIXED(x)    ((x) << 16)
#define APU_FROM_FIXED(x)  ((x) >> 16)

static const uint8_t vbl_length[32] = {
	 5,	127,
	10,	  1,
	19,	  2,
	40,	  3,
	80,	  4,
	30,	  5,
	 7,	  6,
	13,	  7,
	 6,	  8,
	12,	  9,
	24,	 10,
	48,	 11,
	96,	 12,
	36,	 13,
	 8,	 14,
	16,	 15
};
static const int freq_limit[8] = {
	0x3ff, 0x555, 0x666, 0x71c, 0x787, 0x7c1, 0x7e0, 0x7f0
};
static const int noise_freq[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};
static const int dmc_clocks[16] = {
	428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 85, 72, 54
};
static const int duty_lut[4] = {
	2, 4, 8, 12
};

// rectangle wave

int32_t APU::create_rectangle(rectangle_t *chan)
{
	int32_t output = 0;
	double total, sample_weight;
	
	if(!chan->enabled || chan->vbl_length <= 0) {
		return chan->output_vol;
	}
	
	// vbl length counter
	if(!chan->holdnote) {
		chan->vbl_length -= count_rate;
	}
	
	// envelope decay at a rate of (env_delay + 1) / 240 secs
	chan->env_phase -= 4 * count_rate;
	while(chan->env_phase < 0) {
		chan->env_phase += chan->env_delay;
		if(chan->holdnote) {
			chan->env_vol = (chan->env_vol + 1) & 0x0f;
		} else if(chan->env_vol < 0x0f) {
			chan->env_vol++;
		}
	}
	if(chan->freq < 8 || (!chan->sweep_inc && chan->freq > chan->freq_limit)) {
		return chan->output_vol;
	}
	
	// frequency sweeping at a rate of (sweep_delay + 1) / 120 secs
	if(chan->sweep_on && chan->sweep_shifts) {
		chan->sweep_phase -= 2 * count_rate;
		while(chan->sweep_phase < 0) {
			chan->sweep_phase += chan->sweep_delay;
			if(chan->sweep_inc) {
				if(chan->sweep_complement) {
					chan->freq += ~(chan->freq >> chan->sweep_shifts);
				} else {
					chan->freq -= (chan->freq >> chan->sweep_shifts);
				}
			} else {
				chan->freq += (chan->freq >> chan->sweep_shifts);
			}
		}
	}
	
	if(chan->fixed_envelope) {
		output = chan->volume << 8; // fixed volume
	} else {
		output = (chan->env_vol ^ 0x0f) << 8;
	}
	sample_weight = chan->phaseacc;
	if(sample_weight > cycle_rate) {
		sample_weight = cycle_rate;
	}
	total = (chan->adder < chan->duty_flip) ? sample_weight : -sample_weight;
	
	chan->phaseacc -= cycle_rate; // number of cycles per sample
	while(chan->phaseacc < 0) {
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);
		chan->adder = (chan->adder + 1) & 0x0f;
		sample_weight = APU_TO_FIXED(chan->freq + 1);
		if(chan->phaseacc > 0) {
			sample_weight -= chan->phaseacc;
		}
		total += (chan->adder < chan->duty_flip) ? sample_weight : -sample_weight;
	}
	chan->output_vol = (int)floor(output * total / cycle_rate + 0.5);
	
	return chan->output_vol;
}

// triangle wave

int32_t APU::create_triangle(triangle_t *chan)
{
	double sample_weight, total;
	
	if(!chan->enabled || chan->vbl_length <= 0) {
		return ((chan->output_vol * 21) >> 4);
	}
	if(chan->counter_started) {
		if(chan->linear_length > 0) {
			chan->linear_length -= 4 * count_rate;
		}
		if(chan->vbl_length > 0 && !chan->holdnote) {
			chan->vbl_length -= count_rate;
		}
	} else if(!chan->holdnote && chan->write_latency) {
		if(--chan->write_latency == 0) {
			chan->counter_started = true;
		}
	}
	if(chan->linear_length <= 0 || chan->freq < APU_TO_FIXED(4)) {
		return ((chan->output_vol * 21) >> 4);
	}
	
	sample_weight = chan->phaseacc;
	if(sample_weight > cycle_rate) {
		sample_weight = cycle_rate;
	}
	total = (((chan->adder & 0x10) ? 0x1f : 0) ^ chan->adder) * sample_weight;
	
	chan->phaseacc -= cycle_rate; // number of cycles per sample
	while(chan->phaseacc < 0) {
		chan->phaseacc += chan->freq;
		chan->adder = (chan->adder + 1) & 0x1f;
		
		sample_weight = chan->freq;
		if(chan->phaseacc > 0) {
			sample_weight -= chan->phaseacc;
		}
		total += (((chan->adder & 0x10) ? 0x1f : 0) ^ chan->adder) * sample_weight;
	}
	chan->output_vol = (int)floor(total * 512 / cycle_rate + 0.5);
	
	return ((chan->output_vol * 21) >> 4);
}

// white noise channel

int32_t APU::create_noise(noise_t *chan)
{
	int32_t outvol;
	double total;
	double sample_weight;
	
	if(!chan->enabled || chan->vbl_length <= 0) {
		return ((chan->output_vol * 13) >> 4);
	}
	
	// vbl length counter
	if(!chan->holdnote) {
		chan->vbl_length -= count_rate;
	}
	
	// envelope decay at a rate of (env_delay + 1) / 240 secs
	chan->env_phase -= 4 * count_rate;
	while(chan->env_phase < 0) {
		chan->env_phase += chan->env_delay;
		if(chan->holdnote) {
			chan->env_vol = (chan->env_vol + 1) & 0x0f;
		} else if(chan->env_vol < 0x0f) {
			chan->env_vol++;
		}
	}
	
	if(chan->fixed_envelope) {
		outvol = chan->volume << 8; // fixed volume
	} else {
		outvol = (chan->env_vol ^ 0x0f) << 8;
	}
	sample_weight = chan->phaseacc;
	if(sample_weight > cycle_rate) {
		sample_weight = cycle_rate;
	}
	total = chan->noise_bit ? sample_weight : -sample_weight;
	
	chan->phaseacc -= cycle_rate; // number of cycles per sample
	while(chan->phaseacc < 0) {
		chan->phaseacc += chan->freq;
		int bit0 = chan->shift_reg & 1;
		int tap = (chan->shift_reg & chan->xor_tap) ? 1 : 0;
		int bit14 = (bit0 ^ tap);
		chan->shift_reg >>= 1;
		chan->shift_reg |= (bit14 << 14);
		chan->noise_bit = bit0 ^ 1;
		sample_weight = chan->freq;
		if(chan->phaseacc > 0) {
			sample_weight -= chan->phaseacc;
		}
		total += chan->noise_bit ? sample_weight : -sample_weight;
	}
	chan->output_vol = (int)floor(outvol * total / cycle_rate + 0.5);
	
	return ((chan->output_vol * 13) >> 4);
}

// delta modulation

inline void APU::dmc_reload(dmc_t *chan)
{
	chan->address = chan->cached_addr;
	chan->dma_length = chan->cached_dmalength;
	chan->irq_occurred = false;
}

int32_t APU::create_dmc(dmc_t *chan)
{
	double total;
	double sample_weight;
	int delta_bit;
	
	// only process when channel is alive
	if(chan->dma_length) {
		sample_weight = chan->phaseacc;
		if(sample_weight > cycle_rate) {
			sample_weight = cycle_rate;
		}
		total = (chan->regs[1] << 8) * sample_weight;
		chan->phaseacc -= cycle_rate; // number of cycles per sample
		
		while(chan->phaseacc < 0) {
			chan->phaseacc += chan->freq;
			
			if(!(chan->dma_length & 7)) {
				chan->cur_byte = d_mem->read_data8(chan->address);
				//nes6502_burn(1);
				
				if(chan->address == 0xffff) {
					chan->address = 0x8000;
				} else {
					chan->address++;
				}
			}
			if(--chan->dma_length == 0) {
				if(chan->looping) {
					dmc_reload(chan);
				} else {
					// check to see if we should generate an irq
					if(chan->irq_gen) {
						chan->irq_occurred = true;
					}
					// bodge for timestamp queue
					sample_weight = chan->freq - chan->phaseacc;
					total += (chan->regs[1] << 8) * sample_weight;
					while(chan->phaseacc < 0) {
						chan->phaseacc += chan->freq;
					}
					chan->enabled = false;
					break;
				}
			}
			delta_bit = (chan->dma_length & 7) ^ 7;
			
			if(chan->cur_byte & (1 << delta_bit)) {
				if(chan->regs[1] < 0x7d) {
					chan->regs[1] += 2;
				}
			} else {
				if(chan->regs[1] > 1) {
					chan->regs[1] -= 2;
				}
			}
			sample_weight = chan->freq;
			if(chan->phaseacc > 0) {
				sample_weight -= chan->phaseacc;
			}
			total += (chan->regs[1] << 8) * sample_weight;
		}
		chan->output_vol = (int)floor(total / cycle_rate + 0.5);
	} else {
		chan->output_vol = chan->regs[1] << 8;
	}
	
	return ((chan->output_vol * 13) >> 4);
}

void APU::enqueue(queue_t *d)
{
	queue[q_head] = *d;
	q_head = (q_head + 1) & APUQUEUE_MASK;
}

queue_t* APU::dequeue()
{
	int loc = q_tail;
	q_tail = (q_tail + 1) & APUQUEUE_MASK;
	return &queue[loc];
}

void APU::write_data_sync(uint32_t addr, uint32_t data)
{
	int chan;
	
	switch (addr) {
	case 0x4000:
	case 0x4004:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].regs[0] = data;
		rectangle[chan].volume = data & 0x0f;
		rectangle[chan].env_delay = decay_lut[data & 0x0f];
		rectangle[chan].holdnote = ((data & 0x20) != 0);
		rectangle[chan].fixed_envelope = ((data & 0x10) != 0);
		rectangle[chan].duty_flip = duty_lut[data >> 6];
		break;
	case 0x4001:
	case 0x4005:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].regs[1] = data;
		rectangle[chan].sweep_on = ((data & 0x80) != 0);
		rectangle[chan].sweep_shifts = data & 7;
		rectangle[chan].sweep_delay = decay_lut[(data >> 4) & 7];
		rectangle[chan].sweep_inc = ((data & 0x08) != 0);
		rectangle[chan].freq_limit = freq_limit[data & 7];
		break;
	case 0x4002:
	case 0x4006:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].regs[2] = data;
		rectangle[chan].freq = (rectangle[chan].freq & ~0xff) | data;
		break;
	case 0x4003:
	case 0x4007:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].regs[3] = data;
		rectangle[chan].vbl_length = vbl_lut[data >> 3];
		rectangle[chan].env_vol = 0;
		rectangle[chan].freq = ((data & 7) << 8) | (rectangle[chan].freq & 0xff);
		rectangle[chan].adder = 0;
		if(enable_reg & (1 << chan)) {
			rectangle[chan].enabled = true;
		}
		break;
	case 0x4008:
		touch_sound();
		triangle.regs[0] = data;
		triangle.holdnote = ((data & 0x80) != 0);
		if(!triangle.counter_started && triangle.vbl_length > 0) {
			triangle.linear_length = trilength_lut[data & 0x7f];
		}
		break;
	case 0x400a:
		touch_sound();
		triangle.regs[1] = data;
		triangle.freq = APU_TO_FIXED((((triangle.regs[2] & 7) << 8) + data) + 1);
		break;
	case 0x400b:
		touch_sound();
		triangle.regs[2] = data;
		triangle.write_latency = (int)(228 / APU_FROM_FIXED(cycle_rate));
		triangle.freq = APU_TO_FIXED((((data & 7) << 8) + triangle.regs[1]) + 1);
		triangle.vbl_length = vbl_lut[data >> 3];
		triangle.counter_started = false;
		triangle.linear_length = trilength_lut[triangle.regs[0] & 0x7f];
		if(enable_reg & 0x04) {
			triangle.enabled = true;
		}
		break;
	case 0x400c:
		touch_sound();
		noise.regs[0] = data;
		noise.env_delay = decay_lut[data & 0x0f];
		noise.holdnote = ((data & 0x20) != 0);
		noise.fixed_envelope = ((data & 0x10) != 0);
		noise.volume = data & 0x0f;
		break;
	case 0x400e:
		touch_sound();
		noise.regs[1] = data;
		noise.freq = APU_TO_FIXED(noise_freq[data & 0x0f]);
		noise.xor_tap = (data & 0x80) ? 0x40: 0x02;
		break;
	case 0x400f:
		touch_sound();
		noise.regs[2] = data;
		noise.vbl_length = vbl_lut[data >> 3];
		noise.env_vol = 0; /* reset envelope */
		if(enable_reg & 0x08) {
			noise.enabled = true;
		}
		break;
	case 0x4010:
		touch_sound();
		dmc.regs[0] = data;
		dmc.freq = APU_TO_FIXED(dmc_clocks[data & 0x0f]);
		dmc.looping = ((data & 0x40) != 0);
		if(data & 0x80) {
			dmc.irq_gen = true;
		} else {
			dmc.irq_gen = false;
			dmc.irq_occurred = false;
		}
		break;
	case 0x4011:	/* 7-bit DAC */
		touch_sound();
		data &= 0x7f; /* bit 7 ignored */
		dmc.regs[1] = data;
		break;
	case 0x4012:
		touch_sound();
		dmc.regs[2] = data;
		dmc.cached_addr = 0xc000 + (uint16_t) (data << 6);
		break;
	case 0x4013:
		touch_sound();
		dmc.regs[3] = data;
		dmc.cached_dmalength = ((data << 4) + 1) << 3;
		break;
	case 0x4015:
		touch_sound();
		// bodge for timestamp queue
		dmc.enabled = ((data & 0x10) != 0);
		enable_reg = data;
		for(chan = 0; chan < 2; chan++) {
			if(!(data & (1 << chan))) {
				rectangle[chan].enabled = false;
				rectangle[chan].vbl_length = 0;
			}
		}
		if(!(data & 0x04)) {
			triangle.enabled = false;
			triangle.vbl_length = 0;
			triangle.linear_length = 0;
			triangle.counter_started = false;
			triangle.write_latency = 0;
		}
		if(!(data & 0x08)) {
			noise.enabled = false;
			noise.vbl_length = 0;
		}
		if(data & 0x10) {
			if(!dmc.dma_length) {
				dmc_reload(&dmc);
			}
		} else {
			dmc.dma_length = 0;
			dmc.irq_occurred = false;
		}
		break;
	case 0x4009:
	case 0x400D:
		break;
	case 0x4017:
		touch_sound();
		count_rate = (data & 0x80) ? 4 : 5;
		break;
	default:
		break;
	}
}

void APU::write_data_cur(uint32_t addr, uint32_t data)
{
	// for sync read $4015
	int chan;
	
	switch (addr) {
	case 0x4000:
	case 0x4004:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].holdnote_cur = ((data & 0x20) != 0);
		break;
	case 0x4003:
	case 0x4007:
		touch_sound();
		chan = (addr & 4) >> 2;
		rectangle[chan].vbl_length_cur = vbl_length[data >> 3] * 5;
		if(enable_reg_cur & (1 << chan)) {
			rectangle[chan].enabled_cur = true;
		}
		break;
	case 0x4008:
		touch_sound();
		triangle.holdnote_cur = ((data & 0x80) != 0);
		break;
	case 0x400b:
		touch_sound();
		triangle.vbl_length_cur = vbl_length[data >> 3] * 5;
		if(enable_reg_cur & 0x04) {
			triangle.enabled_cur = true;
		}
		triangle.counter_started_cur = true;
		break;
	case 0x400c:
		touch_sound();
		noise.holdnote_cur = ((data & 0x20) != 0);
		break;
	case 0x400f:
		touch_sound();
		noise.vbl_length_cur = vbl_length[data >> 3] * 5;
		if(enable_reg_cur & 0x08) {
			noise.enabled_cur = true;
		}
		break;
	case 0x4010:
		touch_sound();
		dmc.freq_cur = dmc_clocks[data & 0x0f];
		dmc.phaseacc_cur = 0;
		dmc.looping_cur = ((data & 0x40) != 0);
		if(data & 0x80) {
			dmc.irq_gen_cur = true;
		} else {
			dmc.irq_gen_cur = false;
			dmc.irq_occurred_cur = false;
		}
		break;
	case 0x4013:
		touch_sound();
		dmc.cached_dmalength_cur = (data << 4) + 1;
		break;
	case 0x4015:
		touch_sound();
		enable_reg_cur = data;
		for(chan = 0; chan < 2; chan++) {
			if(!(data & (1 << chan))) {
				rectangle[chan].enabled_cur = false;
				rectangle[chan].vbl_length_cur = 0;
			}
		}
		if(!(data & 0x04)) {
			triangle.enabled_cur = false;
			triangle.vbl_length_cur = 0;
			triangle.counter_started_cur = false;
		}
		if(!(data & 0x08)) {
			noise.enabled_cur = false;
			noise.vbl_length_cur = 0;
		}
		if(data & 0x10) {
			if(!dmc.dma_length_cur) {
				dmc.dma_length_cur = dmc.cached_dmalength_cur;
			}
			dmc.enabled_cur = true;
		} else {
			dmc.dma_length_cur = 0;
			dmc.enabled_cur = false;
			dmc.irq_occurred_cur = false;
		}
		break;
	}
}

// interface

void APU::initialize()
{
	register_frame_event(this);
	register_vline_event(this);
}

void APU::reset()
{
	touch_sound();
	
	// reset queue
	elapsed_cycles = 0;
	memset(&queue, 0, APUQUEUE_SIZE * sizeof(queue_t));
	q_head = q_tail = 0;
	
	// reset apu
	for(int i = 0; i < 2; i++) {
		memset(&rectangle[i], 0, sizeof(rectangle[i]));
	}
	rectangle[0].sweep_complement = true;
	rectangle[1].sweep_complement = false;
	memset(&triangle, 0, sizeof(triangle));
	memset(&noise, 0, sizeof(noise));
	noise.shift_reg = 0x4000;
	memset(&dmc, 0, sizeof(dmc));
	
	// reset registers
	for(uint32_t addr = 0x4000; addr <= 0x4013; addr++) {
		write_data_sync(addr, 0);
		write_data_cur(addr, 0);
	}
	write_data_sync(0x4015, 0);
	write_data_cur(0x4015, 0);
	
	enable_reg = 0;
	enable_reg_cur = 0;
	count_rate = 5;
	ave = max = min = 0;
}

void APU::write_data8(uint32_t addr, uint32_t data)
{
	queue_t d;
	
	write_data_cur(addr, data);
	
	switch (addr) {
	case 0x4015:
		// bodge for timestamp queue
		dmc.enabled = ((data & 0x10) != 0);
	case 0x4000: case 0x4001: case 0x4002: case 0x4003:
	case 0x4004: case 0x4005: case 0x4006: case 0x4007:
	case 0x4008: case 0x4009: case 0x400a: case 0x400b:
	case 0x400c: case 0x400d: case 0x400e: case 0x400f:
	case 0x4010: case 0x4011: case 0x4012: case 0x4013:
	case 0x4017:
		touch_sound();
		d.timestamp = get_current_clock();
		d.addr = addr;
		d.data = data;
#ifdef APU_USE_QUEUE
		enqueue(&d);
#else
		write_data_sync(addr, data);
#endif
		break;
	}
}

uint32_t APU::read_data8(uint32_t addr)
{
	if(addr == 0x4015) {
		uint32_t data = 0;
		// return 1 in 0-5 bit pos if a channel is playing
		if(rectangle[0].enabled_cur && rectangle[0].vbl_length_cur > 0) {
			data |= 0x01;
		}
		if(rectangle[1].enabled_cur && rectangle[1].vbl_length_cur > 0) {
			data |= 0x02;
		}
		if(triangle.enabled_cur && triangle.vbl_length_cur > 0) {
			data |= 0x04;
		}
		if(noise.enabled_cur && noise.vbl_length_cur > 0) {
			data |= 0x08;
		}
		// bodge for timestamp queue
		if(dmc.enabled_cur) {
			data |= 0x10;
		}
		if(dmc.irq_occurred_cur) {
			data |= 0x80;
		}
		return data;
	}
	return (addr >> 8);
}

void APU::event_frame()
{
	if(!rectangle[0].holdnote_cur && rectangle[0].vbl_length_cur > 0) {
		rectangle[0].vbl_length_cur -= count_rate;
	}
	if(!rectangle[1].holdnote_cur && rectangle[1].vbl_length_cur > 0) {
		rectangle[1].vbl_length_cur -= count_rate;
	}
	if(triangle.counter_started_cur) {
		if(triangle.vbl_length_cur > 0 && !triangle.holdnote_cur) {
			triangle.vbl_length_cur -= count_rate;
		}
	}
	if(!noise.holdnote_cur && noise.vbl_length_cur > 0) {
		noise.vbl_length_cur -= count_rate;
	}
}

void APU::event_vline(int v, int clock)
{
	// 525 -> 262.5
	if(v & 1) {
		return;
	}
	v >>= 1;
	
	bool irq_occurred = false;
	
	dmc.phaseacc_cur -= clock;
	while(dmc.phaseacc_cur < 0) {
		dmc.phaseacc_cur += dmc.freq_cur * 8;
		if(dmc.dma_length_cur) {
			if(--dmc.dma_length_cur == 0) {
				if(dmc.looping_cur) {
					dmc.dma_length_cur = dmc.cached_dmalength_cur;
					dmc.irq_occurred_cur = false;
				} else {
					dmc.dma_length_cur = 0;
					if(dmc.irq_gen_cur) {
						dmc.irq_occurred_cur = true;
						irq_occurred = true;
					}
					dmc.enabled_cur = false;
				}
			}
		}
	}
	if(irq_occurred) {
		// pending
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
	}
}

void APU::initialize_sound(int rate, int samples)
{
	cycle_rate = (int32_t)(APU_BASEFREQ * 65536.0 / (float)rate);
	
	// lut used for enveloping and frequency sweeps
	for(int i = 0; i < 16; i++) {
		decay_lut[i] = (rate / 60) * (i + 1) * 5;
	}
	// used for note length, based on vblanks and size of audio buffer
	for(int i = 0; i < 32; i++) {
		vbl_lut[i] = vbl_length[i] * (rate / 60) * 5;
	}
	// triangle wave channel's linear length table
	for(int i = 0; i < 128; i++) {
		trilength_lut[i] = (rate / 60) * i * 5;
	}
}

void APU::mix(int32_t* buffer, int num_samples)
{
	uint32_t cpu_cycles = elapsed_cycles;
	
	while(num_samples--) {
#ifdef APU_USE_QUEUE
		// check queue
		while((q_head != q_tail) && (queue[q_tail].timestamp <= cpu_cycles)) {
			queue_t *d = dequeue();
			write_data_sync(d->addr, d->data);
		}
		cpu_cycles += APU_FROM_FIXED(cycle_rate);
#endif
		int32_t accum = 0;
		accum += create_rectangle(&rectangle[0]);
		accum += create_rectangle(&rectangle[1]);
		accum += create_triangle(&triangle);
		accum += create_noise(&noise);
		accum += create_dmc(&dmc);
		
		double delta = (max - min) / 32768.0;
		max -= delta;
		min += delta;
		if(accum > max) {
			max = accum;
		}
		if(accum < min) {
			min = accum;
		}
		ave -= ave / 1024.0;
		ave += (max + min) / 2048.0;
		accum -= (int32_t)ave;
		
		*buffer++ += apply_volume(accum, volume_l); // L
		*buffer++ += apply_volume(accum, volume_r); // R
	}
	
	// resync cycle counter
	elapsed_cycles = get_current_clock();
}

void APU::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

#define STATE_VERSION	2

void process_state_rectangle(rectangle_t* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->regs, sizeof(val->regs), 1);
	state_fio->StateValue(val->enabled);
	state_fio->StateValue(val->phaseacc);
	state_fio->StateValue(val->freq);
	state_fio->StateValue(val->output_vol);
	state_fio->StateValue(val->fixed_envelope);
	state_fio->StateValue(val->holdnote);
	state_fio->StateValue(val->volume);
	state_fio->StateValue(val->sweep_phase);
	state_fio->StateValue(val->sweep_delay);
	state_fio->StateValue(val->sweep_on);
	state_fio->StateValue(val->sweep_shifts);
	state_fio->StateValue(val->sweep_length);
	state_fio->StateValue(val->sweep_inc);
	state_fio->StateValue(val->freq_limit);
	state_fio->StateValue(val->sweep_complement);
	state_fio->StateValue(val->env_phase);
	state_fio->StateValue(val->env_delay);
	state_fio->StateValue(val->env_vol);
	state_fio->StateValue(val->vbl_length);
	state_fio->StateValue(val->adder);
	state_fio->StateValue(val->duty_flip);
	state_fio->StateValue(val->enabled_cur);
	state_fio->StateValue(val->holdnote_cur);
	state_fio->StateValue(val->vbl_length_cur);
}

void process_state_triangle(triangle_t* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->regs, sizeof(val->regs), 1);
	state_fio->StateValue(val->enabled);
	state_fio->StateValue(val->freq);
	state_fio->StateValue(val->phaseacc);
	state_fio->StateValue(val->output_vol);
	state_fio->StateValue(val->adder);
	state_fio->StateValue(val->holdnote);
	state_fio->StateValue(val->counter_started);
	state_fio->StateValue(val->write_latency);
	state_fio->StateValue(val->vbl_length);
	state_fio->StateValue(val->linear_length);
	state_fio->StateValue(val->enabled_cur);
	state_fio->StateValue(val->holdnote_cur);
	state_fio->StateValue(val->counter_started_cur);
	state_fio->StateValue(val->vbl_length_cur);
}

void process_state_noise(noise_t* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->regs, sizeof(val->regs), 1);
	state_fio->StateValue(val->enabled);
	state_fio->StateValue(val->freq);
	state_fio->StateValue(val->phaseacc);
	state_fio->StateValue(val->output_vol);
	state_fio->StateValue(val->env_phase);
	state_fio->StateValue(val->env_delay);
	state_fio->StateValue(val->env_vol);
	state_fio->StateValue(val->fixed_envelope);
	state_fio->StateValue(val->holdnote);
	state_fio->StateValue(val->volume);
	state_fio->StateValue(val->vbl_length);
	state_fio->StateValue(val->xor_tap);
	state_fio->StateValue(val->enabled_cur);
	state_fio->StateValue(val->holdnote_cur);
	state_fio->StateValue(val->vbl_length_cur);
	state_fio->StateValue(val->shift_reg);
	state_fio->StateValue(val->noise_bit);
}

void process_state_dmc(dmc_t* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->regs, sizeof(val->regs), 1);
	state_fio->StateValue(val->enabled);
	state_fio->StateValue(val->freq);
	state_fio->StateValue(val->phaseacc);
	state_fio->StateValue(val->output_vol);
	state_fio->StateValue(val->address);
	state_fio->StateValue(val->cached_addr);
	state_fio->StateValue(val->dma_length);
	state_fio->StateValue(val->cached_dmalength);
	state_fio->StateValue(val->cur_byte);
	state_fio->StateValue(val->looping);
	state_fio->StateValue(val->irq_gen);
	state_fio->StateValue(val->irq_occurred);
	state_fio->StateValue(val->freq_cur);
	state_fio->StateValue(val->phaseacc_cur);
	state_fio->StateValue(val->dma_length_cur);
	state_fio->StateValue(val->cached_dmalength_cur);
	state_fio->StateValue(val->enabled_cur);
	state_fio->StateValue(val->looping_cur);
	state_fio->StateValue(val->irq_gen_cur);
	state_fio->StateValue(val->irq_occurred_cur);
}

void process_state_queue(queue_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->timestamp);
	state_fio->StateValue(val->addr);
	state_fio->StateValue(val->data);
}

bool APU::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < array_length(rectangle); i++) {
		process_state_rectangle(&rectangle[i], state_fio);
	}
	process_state_triangle(&triangle, state_fio);
	process_state_noise(&noise, state_fio);
	process_state_dmc(&dmc, state_fio);
	state_fio->StateValue(enable_reg);
	state_fio->StateValue(enable_reg_cur);
	state_fio->StateValue(count_rate);
	for(int i = 0; i < array_length(queue); i++) {
		process_state_queue(&queue[i], state_fio);
	}
	state_fio->StateValue(q_head);
	state_fio->StateValue(q_tail);
	state_fio->StateValue(elapsed_cycles);
	state_fio->StateValue(ave);
	state_fio->StateValue(max);
	state_fio->StateValue(min);
	return true;
}

