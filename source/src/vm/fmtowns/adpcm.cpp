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
	event_adc_clock = -1;
	event_adpcm_clock = -1;
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
	dac_intr_mask = 0x0000; // Disable
	dac_intr = 0x0000;      // OFF
	latest_dac_intr = false;

	opx_intr = false;
	adpcm_mute = false;
	opn2_mute = false;
	
	write_signals(&outputs_intr, 0x00000000);
	write_signals(&outputs_led_control, 0x00000000);
	write_signals(&outputs_allmute, 0xffffffff); // OK?
	
	initialize_adc_clock(-1);
	if(event_adpcm_clock >= 0) {
		cancel_event(this, event_adpcm_clock);
	}
	// Tick is (8.0e6 / 384.0)[Hz] .. Is this true?
	register_event(this, EVENT_ADPCM_CLOCK, (384.0 * 2.0) / 16.0, true, &event_adpcm_clock);
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
	uint8_t val = 0x00;
	switch(addr) {
	case 0x04e7: // ADC data register
		if(!(adc_fifo->empty())) {
			val = (uint8_t)(adc_fifo->read() & 0xff);
		} else {
			val = 0x00;
		}
		break;
	case 0x04e8: // ADC flags
		val = (!(adc_fifo->empty())) ? 0x01 : 0x00;
		break;
	case 0x04e9: // Int13 reason
		val = 0x00 | ((dac_intr != 0) ? 0x08 : 0x00) | ((opx_intr) ? 0x01 : 0x00);
		break;
	case 0x04ea: // PCM Interrupt mask
		val = dac_intr_mask;
		break;
	case 0x04eb: // PCM Interrupt status
		{
			val = dac_intr;
			dac_intr = 0x00;
			if(latest_dac_intr) {
//				opx_intr = false;
				write_signals(&outputs_intr, 0); // Clear Interrupt
				latest_dac_intr = false;
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
	switch(addr) {
	case 0x04d5:
		opn2_mute = ((data & 0x02) == 0) ? true : false;
		adpcm_mute =  ((data & 0x01) == 0) ? true : false;
		d_opn2->write_signal(SIG_YM2612_MUTE, (opn2_mute) ? 0xffffffff : 0x00000000, 0xffffffff);
		d_rf5c68->write_signal(SIG_RF5C68_MUTE, (adpcm_mute) ? 0xffffffff : 0x00000000, 0xffffffff);
		break;
	case 0x04e8:
		adc_fifo->clear();
		break;
	case 0x04ea:
		dac_intr_mask = data;
		break;
	case 0x04ec:
		write_signals(&outputs_led_control, ((data & 0x80) == 0) ? 0xffffffff : 0x00000000);
		write_signals(&outputs_allmute, ((data & 0x40) == 0) ? 0xffffffff : 0x00000000);
		break;
	default:
		if(addr >= 0x04f0) {
			d_rf5c68->write_io8(addr & 0x0f, data);
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
//		out_debug_log(_T("SIG_ADPCM_WRITE_INTERRUPT val=%08X mask=%08X"), data ,mask);
		uint32_t n_ch = data & 0x07;
		bool n_onoff = (((data & mask) & 0x00000008) != 0) ? true : false;
		bool n_allset =(((data & mask) & 0x80000000) != 0) ? true : false;
		bool _d = false;
		if(!(n_allset)) {
			_d = ((dac_intr_mask & (1 << n_ch)) != 0) ? true : false;
			if(n_onoff) {
				dac_intr = dac_intr | (1 << n_ch);
			} else {
				dac_intr = dac_intr & ~(1 << n_ch);
			}
		} else {
			// ALLSET
			uint16_t intr_backup = dac_intr;
			dac_intr = (n_onoff) ? 0xffff : 0x0000;
			_d = true;
//			_d = (dac_intr != intr_backup) ? true : false;
		}	
		if((n_onoff) && (_d)) { // ON
			write_signals(&outputs_intr, 0xffffffff);
			latest_dac_intr = true;
		} else if(!(n_onoff) || !(_d)) {
			if(!(opx_intr) && (latest_dac_intr)) {
				write_signals(&outputs_intr, 0x00000000);
			}				
			latest_dac_intr = false;
		}
	} else if(ch == SIG_ADPCM_OPX_INTR) { // SET/RESET INT13
//		out_debug_log(_T("SIG_ADPCM_OPX_INTR val=%08X mask=%08X"), data ,mask);
		opx_intr = ((data & mask) != 0);
		if(opx_intr) {
			write_signals(&outputs_intr, 0xffffffff);
		} else {
			if(!(latest_dac_intr)) {
				write_signals(&outputs_intr, 0x00000000);
			}
		}			
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
	
	state_fio->StateValue(opx_intr);
	state_fio->StateValue(dac_intr);
	state_fio->StateValue(dac_intr_mask);
	state_fio->StateValue(latest_dac_intr);

	state_fio->StateValue(event_adc_clock);
	state_fio->StateValue(event_adpcm_clock);
	return true;
}
	
}	
