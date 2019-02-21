/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[I/O around ADPCM]
*/

#include "./adpcm.h"
#include "rf5c68.h"
#include "ad7820kr.h"

namespace FMTOWNS {
void ADPCM::initialize()
{
	for(int i = 0; i < 8; i++) {
		dac_int_mask[i] = true; // true = enable intrrupt.
		dac_intr[i] = false;
	}
	intr_opx = false;
}

void ADPCM::reset()
{
	// Is clear FIFO?
}
uint32_t ADPCM::read_io8(uint32_t addr)
{
	/*
	  0x04e7 - 0x04e8 : ADC
	  0x04f0 - 0x04f8 : DAC
	*/
	uint8_t val = 0xff;
	switch(addr & 0x1f) {
	case 0x07: // ADC data register
		if(!(adc_fifo->empty())) {
			val = (uint8_t)(adc_fifo->read() & 0xff);
		} else {
			val = 0x00;
		}
		break;
	case 0x08: // ADC flags
		val = (!(adc_fifo->empty())) ? 0x01 : 0x00;
		break;
	case 0x09: // Int13 reason
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
	case 0x0a: // PCM Interrupt mask
		val = 0x00;
		for(int i = 0; i < 8; i++) {
			val = val | ((dac_int_mask[i]) ? (0x01 << i) : 0);
		}
		break;
	case 0x0b: // PCM Interrupt status
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
		if((addr & 0x1f) >= 0x10) val = d_rf5c68->read_io8(addr & 0x0f); // AROUND DAC
		break;
	}
	return val;
}

void ADPCM::write_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0x1f;
	if(naddr == 0x08) {
		adc_fifo->clear();
	} else if(naddr == 0x0a) {
		uint32_t mask = 0x01;
		for(int i = 0; i < 8; i++) {
			if((data & mask) != 0) {
				dac_int_mask[i] = true;
			} else {
				dac_int_mask[i] = false;
			}
			mask <<= 1;
		}
	} else if(naddr >= 0x10) {
		d_rf5c68->write_io8(naddr & 0x0f, data);
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
			n_onoff = n_onoff & dac_int_mask[n_ch];
			if(n_onoff != dac_intr[n_ch]) { // SET/RESET INT13				
				write_signals(&output_intr, (n_onoff) ? 0xffffffff : 0x00000000);
			}
			dac_intr[n_ch] = n_onoff;
		} else {
			// ALLSET
			bool n_backup;
			bool _s = false;
			for(int i = 0; i < 8; i++) { // SET/RESET INT13				
				n_backup = dac_intr[i];
				dac_intr[i] = n_onoff & dac_int_mask[i];
				if(n_backup != dac_intr[i]) _s = true;
			}
			if(_s) {
				write_signals(&output_intr, (n_onoff) ? 0xffffffff : 0x00000000);
			}
		}
	} else if(ch == SIG_ADPCM_OPX_INTR) { // SET/RESET INT13
		intr_opx = ((data & mask) != 0); 
		write_signals(&output_intr, (intr_opx) ? 0xffffffff : 0x00000000);
	} else if(ch == SIG_ADPCM_PUSH_FIFO) { // Push data to FIFO from ADC.
		if(adc_fifo->full()) {
			adc_fifo->read(); // Dummy read
		}
		adc_fifo->write((int)(data & 0xff));
	}
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
	state_fio->StateValue(intr_opx);
	state_fio->StateArray(dac_intr,     sizeof(dac_intr), 1);
	state_fio->StateArray(dac_int_mask, sizeof(dac_int_mask), 1);
	return true;
}
	
}	