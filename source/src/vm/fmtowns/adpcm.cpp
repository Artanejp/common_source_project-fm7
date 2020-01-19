/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[I/O around ADPCM]
*/

#include "./adpcm.h"
#include "ad7820kr.h"
#include "rf5c68.h"
#include "ym2612.h"
#include "../i8259.h"

namespace FMTOWNS {

#define EVENT_ADC_CLOCK   1
#define EVENT_ADPCM_CLOCK 2
	
void ADPCM::initialize()
{
	adc_fifo = new FIFO(64); // OK?
	for(int i = 0; i < 8; i++) {
		dac_intr_mask[i] = true; // true = enable intrrupt.
		dac_intr[i] = false;
	}
	intr_opx = false;
}

void ADPCM::release()
{
	adc_fifo->release();
	delete adc_fifo;
}
	
void ADPCM::reset()
{
	// Is clear FIFO?
	adc_fifo->clear();
	write_signals(&outputs_intr, 0x00000000);
	write_signals(&outputs_led_control, 0x00000000);
	write_signals(&outputs_allmute, 0xffffffff); // OK?
	
	initialize_adc_clock(-1);
	if(event_adpcm_clock >= 0) {
		cancel_event(this, event_adpcm_clock);
	}
	// Tick is (8.0e6 / 384.0)[Hz] .. Is this true?
	register_event(this, EVENT_ADPCM_CLOCK, 1.0e6 / (16.0 / (384.0 * 2.0)), true, &event_adpcm_clock);
}

void ADPCM::initialize_adc_clock(int freq)
{
	if(freq <= 0) {
		freq = (int)d_adc->read_signal(SIG_AD7820_SAMPLE_RATE);
	}
	if(event_adc_clock >= 0) {
		cancel_event(this, event_adc_clock);
	}
	d_adc->write_signal(SIG_AD7820_DATA_REG, 0x00, 0x00);
	d_adc->write_signal(SIG_AD7820_CS, 0xffffffff, 0xffffffff);
	d_adc->write_signal(SIG_AD7820_WR_CONVERSION_MODE, 0, 0xffffffff); // READ MODE..
	register_event(this, EVENT_ADC_CLOCK, 1.0e6 / (double)freq, true, &event_adc_clock);
}		

void ADPCM::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_ADC_CLOCK:
		if(!(adc_fifo->full())) {
			d_adc->write_signal(SIG_AD7820_WR_CONVERSION_MODE, 0, 0xffffffff); // READ MODE
			d_adc->write_signal(SIG_AD7820_CS, 0xffffffff, 0xffffffff);
			d_adc->read_data8(SIG_AD7820_DATA_REG); // Dummy read, start to  sample.
		}
		break;
	case EVENT_ADPCM_CLOCK:
		d_rf5c68->write_signal(SIG_RF5C68_DAC_PERIOD, 1, 1);
		break;
	}
}
	
uint32_t ADPCM::read_io8(uint32_t addr)
{
	/*
	  0x04d5 : OPN2/ADPCM MUTE
	  0x04e7 - 0x04e8 : ADC
	  0x04e9 - 0x04ec : DAC CONTROL
	  0x04f0 - 0x04f8 : DAC
	*/
	uint8_t val = 0xff;
	switch(addr & 0xff) {
	case 0xe7: // ADC data register
		if(!(adc_fifo->empty())) {
			val = (uint8_t)(adc_fifo->read() & 0xff);
		} else {
			val = 0x00;
		}
		break;
	case 0xe8: // ADC flags
		val = (!(adc_fifo->empty())) ? 0x01 : 0x00;
		break;
	case 0xe9: // Int13 reason
		{
			bool intr_pcm = false;
			for(int i = 0; i < 8; i++) {
				if(dac_intr[i]) {
					intr_pcm = true;
					break;
				}
			}
			val = 0xf6 | ((intr_pcm) ? 0x08 : 0x00) | ((intr_opx) ? 0x01 : 0x00);
		}
		break;
	case 0xea: // PCM Interrupt mask
		val = 0x00;
		for(int i = 0; i < 8; i++) {
			val = val | ((dac_intr_mask[i]) ? (0x01 << i) : 0);
		}
		break;
	case 0xeb: // PCM Interrupt status
		{
			bool _s = false;
			val = 0x00;
			for(int i = 0; i < 8; i++) {
				val = val | ((dac_intr[i]) ? (0x01 << i) : 0);
			}
			for(int i = 0; i < 8; i++) {
				if(dac_intr[i]) {
					_s = true;
				}
				dac_intr[i] = false;
			}
			if(_s) {
				d_pic->write_signal(SIG_I8259_IR5 | SIG_I8259_CHIP1, 0x00000000, 0xffffffff);
			}
		}
		break;
	default:
		if((addr & 0xff) >= 0xf0) val = d_rf5c68->read_io8(addr & 0x0f); // AROUND DAC
		break;
	}
	return val;
}

void ADPCM::write_io8(uint32_t addr, uint32_t data)
{
	/*
	  0x04d5 : OPN2/ADPCM MUTE
	  0x04e7 - 0x04e8 : ADC
	  0x04e9 - 0x04ec : DAC CONTROL
	  0x04f0 - 0x04f8 : DAC
	*/
	uint32_t naddr = addr & 0xff;
	switch(naddr) {
	case 0xd5:
		opn2_mute = ((data & 0x02) == 0) ? true : false;
		adpcm_mute =  ((data & 0x01) == 0) ? true : false;
		d_opn2->write_signal(SIG_YM2612_MUTE, (opn2_mute) ? 0xffffffff : 0x00000000, 0xffffffff);
		d_rf5c68->write_signal(SIG_RF5C68_MUTE, (adpcm_mute) ? 0xffffffff : 0x00000000, 0xffffffff);
		break;
	case 0xe8:
		adc_fifo->clear();
		break;
	case 0xea:
		{
			uint32_t mask = 0x01;
			for(int i = 0; i < 8; i++) {
				if((data & mask) != 0) {
					dac_intr_mask[i] = true;
				} else {
					dac_intr_mask[i] = false;
				}
				mask <<= 1;
			}
		}
		break;
	case 0xec:
		write_signals(&outputs_led_control, ((data & 0x80) == 0) ? 0xffffffff : 0x00000000);
		write_signals(&outputs_allmute, ((data & 0x40) == 0) ? 0xffffffff : 0x00000000);
		break;
	default:
		if(naddr >= 0xf0) {
			d_rf5c68->write_io8(naddr & 0x0f, data);
		}
		break;
	}
}

uint32_t ADPCM::read_data8(uint32_t addr)
{
	if((addr >= 0xc2200000) && (addr < 0xc2201000)) {
		return d_rf5c68->read_data8(addr & 0x0fff);
	}
	return 0xff;
}

void ADPCM::write_data8(uint32_t addr, uint32_t data)
{
	if((addr >= 0xc2200000) && (addr < 0xc2201000)) {
		d_rf5c68->write_data8(addr & 0x0fff, data);
	}
}

void ADPCM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_ADPCM_WRITE_INTERRUPT) {
		uint32_t n_ch = data & 0x07;
		bool n_onoff = (((data & mask) & 0x00000008) != 0) ? true : false;
		bool n_allset =(((data & mask) & 0x80000000) != 0) ? true : false;
		if(!(n_allset)) {
			n_onoff = n_onoff & dac_intr_mask[n_ch];
			if(n_onoff != dac_intr[n_ch]) { // SET/RESET INT13				
				write_signals(&outputs_intr, (n_onoff) ? 0xffffffff : 0x00000000);
			}
			dac_intr[n_ch] = n_onoff;
		} else {
			// ALLSET
			bool n_backup;
			bool _s = false;
			for(int i = 0; i < 8; i++) { // SET/RESET INT13				
				n_backup = dac_intr[i];
				dac_intr[i] = n_onoff & dac_intr_mask[i];
				if(n_backup != dac_intr[i]) _s = true;
			}
			if(_s) {
				write_signals(&outputs_intr, (n_onoff) ? 0xffffffff : 0x00000000);
			}
		}
	} else if(ch == SIG_ADPCM_OPX_INTR) { // SET/RESET INT13
		intr_opx = ((data & mask) != 0); 
		write_signals(&outputs_intr, (intr_opx) ? 0xffffffff : 0x00000000);
	} else if(ch == SIG_ADPCM_ADC_INTR) { // Push data to FIFO from ADC.
		if((data & mask) != 0) {
			uint32_t n_data = d_adc->read_signal(SIG_AD7820_DATA_REG);
			d_adc->write_signal(SIG_AD7820_CS, 0, 0xffffffff);
			if(!(adc_fifo->full())) {
				adc_fifo->write((int)(n_data & 0xff));
			}
		}
	}
}

uint32_t ADPCM::read_signal(int ch)
{
	return 0;
}
#define STATE_VERSION	1

bool ADPCM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(adc_fifo->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(opn2_mute);
	state_fio->StateValue(adpcm_mute);
	
	state_fio->StateValue(intr_opx);
	state_fio->StateArray(dac_intr,     sizeof(dac_intr), 1);
	state_fio->StateArray(dac_intr_mask, sizeof(dac_intr_mask), 1);

	state_fio->StateValue(event_adc_clock);
	state_fio->StateValue(event_adpcm_clock);
	return true;
}
	
}	
