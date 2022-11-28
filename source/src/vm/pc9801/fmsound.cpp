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

#include "fmsound.h"
#include "../i8259.h"
#include "../../fifo.h"

#if defined(SUPPORT_PC98_OPNA)
#if defined(SUPPORT_PC98_86PCM)
#define BOARD_ID	4
#else
#define BOARD_ID	0
#endif

void FMSOUND::reset()
{
	opna_mask = 0;
#if defined(SUPPORT_PC98_86PCM)
	pcm_vol_ctrl = pcm_fifo_ctrl = 0;
	pcm_dac_ctrl = 0x32;
	pcm_fifo_size = 0x80;
	pcm_mute_ctrl = 0x01;
	pcm_fifo_written = pcm_overflow = pcm_irq_raised = false;
	pcm_fifo->clear();
	if(pcm_register_id != -1) {
		cancel_event(this, pcm_register_id);
		pcm_register_id = -1;
	}
	pcm_sample_l = pcm_sample_r = 0;
#endif
}

#if defined(SUPPORT_PC98_86PCM)
#define EVENT_SAMPLE	0
//#define _PCM_DEBUG_LOG

static const uint32_t sample_rate_x8[] = {
	352800, 264600, 176400, 132300, 88200,  66150,  44010,  33075
};
static const int bytes_per_sample[] = {
	0, 2, 2, 4, 0, 1, 1, 2
};

void FMSOUND::initialize()
{
	pcm_clocks = 0;
	pcm_prev_clock = 0;
	pcm_fifo = new FIFO(0x8000);
	pcm_register_id = -1;
}

void FMSOUND::release()
{
	pcm_fifo->release();
	delete pcm_fifo;
}

void FMSOUND::event_callback(int event_id, int err)
{
	if(pcm_fifo->count() >= bytes_per_sample[(pcm_dac_ctrl >> 4) & 7]) {
		pcm_sample_l = pcm_sample_r = 0;
		
		if(pcm_fifo_ctrl & 0x40) {
			// record
//			pcm_overflow = pcm_fifo->full();
		} else {
			// play
			if(pcm_dac_ctrl & 0x20) pcm_sample_l = pcm_volume * get_sample() / 32768;
			if(pcm_dac_ctrl & 0x10) pcm_sample_r = pcm_volume * get_sample() / 32768;
		}
//		if(pcm_fifo_written && pcm_fifo->count() <= pcm_fifo_size) {
		if(pcm_fifo->count() == pcm_fifo_size) {
			pcm_fifo_written = false;
			pcm_irq_raised = true;
			
			if(pcm_fifo_ctrl & 0x20) {
				#ifdef SUPPORT_PC98_86PCM_IRQ
					d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR4, 1, 1);
				#endif
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Raise IRQ in Sample Event\n"));
				#endif
			}
		}
	}
}

void FMSOUND::mix(int32_t* buffer, int cnt)
{
	if((pcm_fifo_ctrl & 0x80) && !(pcm_fifo_ctrl & 0x40) && !(pcm_mute_ctrl & 1)) {
		for(int i = 0; i < cnt; i++) {
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("Mix Sample = %d,%d\n"), pcm_sample_l, pcm_sample_r);
			#endif
			*buffer++ += apply_volume(pcm_sample_l, pcm_volume_l); // L
			*buffer++ += apply_volume(pcm_sample_r, pcm_volume_r); // R
		}
	}
}

void FMSOUND::set_volume(int ch, int decibel_l, int decibel_r)
{
	pcm_volume_l = decibel_to_volume(decibel_l);
	pcm_volume_r = decibel_to_volume(decibel_r);
}

int FMSOUND::get_sample()
{
	if(!(pcm_dac_ctrl & 0x40)) {
		uint16_t sample;
		sample  = pcm_fifo->read() << 8;
		sample |= pcm_fifo->read();
		return (int)(int16_t)sample;
	} else {
		uint8_t sample = pcm_fifo->read();
		return (int)(int8_t)sample * 256;
	}
}
#endif
#endif

void FMSOUND::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0188:
		d_opn->write_io8(0, data);
		break;
	case 0x018a:
		d_opn->write_io8(1, data);
		break;
#if defined(SUPPORT_PC98_OPNA)
	case 0x018c:
		if(opna_mask & 1) {
			d_opn->write_io8(2, data);
		}
		break;
	case 0x018e:
		if(opna_mask & 1) {
			d_opn->write_io8(3, data);
		}
		break;
	case 0xa460:
		opna_mask = data;
		break;
#if defined(SUPPORT_PC98_86PCM)
	case 0xa466:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("OUT\tA466, %02X\tVOLUME\n"), data);
		#endif
		pcm_vol_ctrl = data;
		break;
	case 0xa468:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("OUT\tA468, %02X\tFIFO\n"), data);
		#endif
		if((pcm_fifo_ctrl & 0x87) != (data & 0x87)) {
			if(pcm_register_id != -1) {
				cancel_event(this, pcm_register_id);
				pcm_register_id = -1;
			}
			if(data & 0x80) {
				register_event(this, EVENT_SAMPLE, 8000000.0 / sample_rate_x8[data & 7], true, &pcm_register_id);
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Start Event\n"));
				#endif
			} else {
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Cancel Event\n"));
				#endif
				pcm_sample_l = pcm_sample_r = 0;
			}
		}
		if(/*(pcm_fifo_ctrl & 0x10) &&*/ !(data & 0x10)) {
			if(pcm_irq_raised) {
				pcm_irq_raised = false;
				#ifdef SUPPORT_PC98_86PCM_IRQ
					d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR4, 0, 0);
				#endif
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Clear IRQ in A468\n"));
				#endif
			}
		}
		if((pcm_fifo_ctrl & 0x08) != (data & 0x08)) {
			pcm_fifo->clear();
			pcm_fifo_written = false;
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("Clear Buffer\n"));
			#endif
		}
		if(!(pcm_fifo_ctrl & 0x20) && (data & 0x20)) {
			if(pcm_irq_raised) {
				#ifdef SUPPORT_PC98_86PCM_IRQ
					d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR4, 1, 1);
				#endif
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Raise IRQ in A468\n"));
				#endif
			}
		} else if((pcm_fifo_ctrl & 0x20) && !(data & 0x20)) {
			if(pcm_irq_raised) {
				#ifdef SUPPORT_PC98_86PCM_IRQ
					d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR4, 0, 0);
				#endif
				#ifdef _PCM_DEBUG_LOG
					this->out_debug_log(_T("Drop IRQ in A468\n"));
				#endif
			}
		}
		pcm_fifo_ctrl = data;
		break;
	case 0xa46a:
		if(pcm_fifo_ctrl & 0x20) {
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("OUT\tA46A, %02X\tIRQ\n"), data);
			#endif
			if(data != 0xff) {
				pcm_fifo_size = (data + 1) * 128;
			} else {
				pcm_fifo_size = 0x7ffc;
			}
		} else {
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("OUT\tA46A, %02X\tDAC\n"), data);
			#endif
			pcm_dac_ctrl = data;
		}
		break;
	case 0xa46c:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("OUT\tA46C, %02X\tBUFFER COUNT=%d\n"), data, pcm_fifo->count() + 1);
		#endif
		pcm_fifo->write(data);
		pcm_fifo_written = true;
		break;
	case 0xa66e:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("OUT\tA66E, %02X\tMUTE\n"), data);
		#endif
		pcm_mute_ctrl = data;
		break;
#endif
#endif
	}
}

uint32_t FMSOUND::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0188:
		return d_opn->read_io8(0);
	case 0x018a:
		return d_opn->read_io8(1);
#if defined(SUPPORT_PC98_OPNA)
	case 0x018c:
		if(opna_mask & 1) {
			return d_opn->read_io8(2);
		}
		break;
	case 0x018e:
		if(opna_mask & 1) {
			return d_opn->read_io8(3);
		}
		break;
	case 0xa460:
		return (BOARD_ID << 4) | (opna_mask & 3);
#if defined(SUPPORT_PC98_86PCM)
	case 0xa462:
	case 0xa464:
		return 0; // dummy
	case 0xa466:
		{
			pcm_clocks += get_passed_clock(pcm_prev_clock);
			pcm_prev_clock = get_current_clock();
			pcm_clocks %= get_event_clocks();
			uint32_t passed_samples_x8 = muldiv_u32(sample_rate_x8[pcm_fifo_ctrl & 7], (uint32_t)pcm_clocks, get_event_clocks());
			uint32_t val = (pcm_fifo->full() ? 0x80 : 0) | (pcm_fifo->empty() ? 0x40 : 0) | (pcm_overflow ? 0x20 : 0) | ((passed_samples_x8 & 7) < 4 ? 0 : 0x01);
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("IN\tA466 = %02X\tSTATUS\n"), val);
			#endif
			return val;
		}
	case 0xa468:
		{
			uint32_t val = pcm_fifo_ctrl & ~0x10;
/*
			if(!pcm_irq_raised) {
				if(pcm_fifo_ctrl & 0x20) {
					if(pcm_fifo_written && pcm_fifo->count() <= pcm_fifo_size) {
						pcm_fifo_written = false;
						pcm_irq_raised = true;
					}
				}
			}
*/
			if(pcm_irq_raised) val |= 0x10;
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("IN\tA468 = %02X\tFIFO\n"), val);
			#endif
			return val;
		}
	case 0xa46a:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("IN\tA46A = %02X\tDAC\n"), pcm_dac_ctrl);
		#endif
		return pcm_dac_ctrl;
	case 0xa46c:
		{
			uint32_t val = 0; //pcm_fifo->read();
			#ifdef _PCM_DEBUG_LOG
				this->out_debug_log(_T("IN\tA46C = %02X\tBUFFER COUNT = %d\n"), val, pcm_fifo->count());
			#endif
			return val;
		}
	case 0xa66e:
		#ifdef _PCM_DEBUG_LOG
			this->out_debug_log(_T("IN\tA66E = %02X\tMUTE\n"), pcm_mute_ctrl);
		#endif
		return pcm_mute_ctrl;
#endif
#endif
	}
	return 0xff;
}

#if defined(SUPPORT_PC98_OPNA)
#define STATE_VERSION	2

bool FMSOUND::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(opna_mask);
#if defined(SUPPORT_PC98_86PCM)
	state_fio->StateValue(pcm_clocks);
	state_fio->StateValue(pcm_prev_clock);
	state_fio->StateValue(pcm_vol_ctrl);
	state_fio->StateValue(pcm_fifo_ctrl);
	state_fio->StateValue(pcm_dac_ctrl);
	state_fio->StateValue(pcm_fifo_size);
	state_fio->StateValue(pcm_mute_ctrl);
	state_fio->StateValue(pcm_fifo_written);
	state_fio->StateValue(pcm_overflow);
	state_fio->StateValue(pcm_irq_raised);
	if(!pcm_fifo->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(pcm_register_id);
	state_fio->StateValue(pcm_sample_l);
	state_fio->StateValue(pcm_sample_r);
#endif
	return true;
}
#endif

