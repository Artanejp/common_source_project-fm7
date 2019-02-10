/*
	NEC-HE PC Engine Emulator 'ePCEngine'
	SHARP X1twin Emulator 'eX1twin'

	Origin : Ootake (joypad/cdrom)
	       : xpce (psg)
	       : MESS (vdc/vce/vpc/cdrom)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.02.09-  Split from pce.cpp

	[ PC-Engine around ADPCM]
*/

#include "./adpcm.h"
#include "./pce.h"


#include "../msm5205.h"
#include "../scsi_host.h"
#include "../scsi_cdrom.h"


namespace PCEDEV {

#define ADPCM_REMAIN_READ_BUF	0x80	
#define ADPCM_REMAIN_WRITE_BUF	0x04	
#define ADPCM_PLAY_FLAG			0x08
#define ADPCM_STOP_FLAG			0x01

#define EVENT_CLEAR_ACK 1
#define EVENT_SET_ACK   2
#define EVENT_FADE_IN   3
#define EVENT_FADE_OUT  4
	
void ADPCM::initialize()
{
	adpcm_clock_divider = 1; // OK?
	event_fader = -1;
	event_ack = -1;
	reg_0c = 0;
	msm_last_cmd = 0;
	reg_0b = 0;
	adpcm_volume = 0.0;

	memset(ram, 0x00, sizeof(ram));
}

void ADPCM::reset()
{
	touch_sound();

	read_ptr = write_ptr = 0;
	read_buf = write_buf = 0;
	written_size = 0;
	dma_enabled = false;
	play_in_progress = false;
	adpcm_paused = false;
	
	msm_ptr = 0;
	msm_nibble = 0;
	adpcm_length = 0;
	addr_reg.w = 0;
	half_addr = 0;
	msm_last_cmd = 0;
	reg_0b = 0;
	adpcm_stream = false;
	adpcm_stopped = true;	
	if(event_fader != -1) {
		cancel_event(this, event_fader);
	}
	event_fader = -1;
	if(event_ack != -1) {
		cancel_event(this, event_ack);
	}
	event_ack = -1;
	
	reg_0c |= ADPCM_STOP_FLAG;
	reg_0c &= ~ADPCM_PLAY_FLAG;
	
	//d_msm->change_clock_w((ADPCM_CLOCK / 6) / adpcm_clock_divider);  // From mednafen 1.22.1
	//adpcm_volume = 0.0;
	d_msm->set_volume((int)adpcm_volume);
	//memset(ram, 0x00, sizeof(ram));
}

uint32_t ADPCM::read_signal(int ch)
{
	switch(ch) {
	case SIG_ADPCM_DATA:
		if(read_buf > 0) {
			reg_0c |= ADPCM_REMAIN_READ_BUF;
			read_buf--;
			if(read_buf == 0) {
				reg_0c &= ~ADPCM_REMAIN_READ_BUF;
			}
			return 0x00;
		} else {
			reg_0c &= ~ADPCM_REMAIN_READ_BUF;
			uint8_t _d = ram[read_ptr & 0xffff];
			read_ptr = (read_ptr + 1) & 0xffff;
			return _d;
		}
		break;
	case SIG_ADPCM_DMACTRL:
		return reg_0b;
		break;
	case SIG_ADPCM_PLAY_IN_PROGRESS:
		return ((play_in_progress) ? 0xffffffff : 0);
		break;
	case SIG_ADPCM_STATUS_REG:
		return reg_0c;
		break;
	case SIG_ADPCM_CMD_REG:
		return msm_last_cmd;
		break;
	}
	return 0x00;
}

void ADPCM::reset_adpcm()
{
	touch_sound();
	// reset ADPCM hardware
	read_ptr = write_ptr = 0;
	read_buf = write_buf = 0;
	msm_ptr = half_addr = 0;
	msm_nibble = 0;
	msm_last_cmd = 0x00;
	addr_reg.w = 0x0000;
	do_stop(false);
	//d_msm->reset();
	d_msm->reset_w(1);
	out_debug_log(_T("RESET ADPCM\n"));
	d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
	d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
	
	// stop ADPCM dma
	//set_dma_status(false);
	adpcm_repeat = false;
	//adpcm_stream = false;
}

void ADPCM::do_play()
{
	reg_0c &= ~ADPCM_STOP_FLAG;
	reg_0c |= ADPCM_PLAY_FLAG;
	play_in_progress = true;
	adpcm_paused = false;
	adpcm_stopped = false;
}

void ADPCM::do_pause(bool pause)
{
	if(!(play_in_progress)) return;
	if(pause) {
		if(!(adpcm_paused)) {
			reg_0c |= ADPCM_STOP_FLAG;
			reg_0c &= ~ADPCM_PLAY_FLAG;
			msm_last_cmd &= ~0x60;
			adpcm_paused = true;
			d_msm->reset_w(1);
			out_debug_log(_T("ADPCM PAUSE PLAY PTR=%04x\n"), msm_ptr);
			touch_sound();
		}
	} else {
		if((adpcm_paused)) {
			adpcm_paused = false;
			touch_sound();
			reg_0c &= ~ADPCM_STOP_FLAG;
			reg_0c |= ADPCM_PLAY_FLAG;
			d_msm->reset_w(0);
			out_debug_log(_T("ADPCM UNPAUSE PLAY PTR=%04x\n"), msm_ptr);
		}
	}
}

void ADPCM::do_stop(bool do_irq)
{
	reg_0c |= ADPCM_STOP_FLAG;
	reg_0c &= ~ADPCM_PLAY_FLAG;
	if(do_irq) {
		d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
	}
	msm_last_cmd &= ~0x60;
	play_in_progress = false;
	//set_dma_status(false);
	out_debug_log(_T("ADPCM STOP PLAY PTR=%04x IRQ=%s\n"), msm_ptr, (do_irq) ? _T("YES") : _T("NO"));
	
}

void ADPCM::set_dma_status(bool flag)
{
	dma_enabled = flag;
	d_pce->write_signal(SIG_PCE_ADPCM_DMA, (flag) ? 0xffffffff : 0x00000000, 0xffffffff);
}

void ADPCM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	//if(ch != SIG_ADPCM_VCLK) out_debug_log(_T("WRITE_SIGNAL SIG=%d DATA=%04x MASK=%04x\n"), ch, data, mask);
	switch(ch) {
	case SIG_ADPCM_DMACTRL:
		//if((reg_0b & 0x01) != (data & 0x01)) {
		//	set_dma_status((reg_0b & 0x01) != 0);
			//reg_0b |= 0x02;
		//}
		reg_0b = data;
		break;
	case SIG_ADPCM_PAUSE:
		do_pause(flag);
		break;
	case SIG_ADPCM_DMA_ENABLED:
		set_dma_status(flag);
		if((flag)/* && (flag != dma_enabled)*/) {
			reg_0c |= ADPCM_REMAIN_WRITE_BUF;
			if(d_pce->read_signal(SIG_PCE_CDROM_DATA_IN) != 0) {
				do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
				out_debug_log(_T("Start DMA port $0B/ALREADY READ DATA ADPCM_WRITE_PTR=%04x ADPCM_READ_PTR=%04x MSM_START_ADDR=%04x\n"),write_ptr, read_ptr, msm_ptr);
			} else {
				out_debug_log(_T("Start DMA port $0B/WAIT FOR DATA\n"));
			}
		}
		break;
	case SIG_ADPCM_RESET:
		if(flag) {
			reset_adpcm();
		}
		break;
	case SIG_ADPCM_COMMAND: // REG $0D
		do_cmd(data);
		break;
	case SIG_ADPCM_WRITE_DMA_DATA:
		do_dma(data);
		break;
	case SIG_ADPCM_DO_DMA_TRANSFER:
		if((play_in_progress) && ((write_ptr & 0xffff) >= (msm_ptr & 0xffff))) {
			// now streaming, wait dma not to overwrite buffer before it is played
			//reg_0b = 0x00;// From Ootake v2.38.
			//set_dma_status(false);
		} else {
			do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
		}
		break;
	case SIG_ADPCM_ADDR_HI: // REG $09
		if((msm_last_cmd & 0x80) != 0) {
			break;
		}
		addr_reg.b.h = data;
		if((msm_last_cmd & 0x10) != 0) {
			adpcm_length = (uint32_t)(addr_reg.w);
			out_debug_log(_T("SET ADDRESS REGISTER HIGH ; UPDATE LENGTH TO %04x\n"), adpcm_length);
		}
		break;
	case SIG_ADPCM_ADDR_LO: // REG $08
		if((msm_last_cmd & 0x80) != 0) {
			break;
		}
		addr_reg.b.l = data;
		if((msm_last_cmd & 0x10) != 0) {
			adpcm_length = (uint32_t)(addr_reg.w);
			out_debug_log(_T("SET ADDRESS REGISTER LOW ; UPDATE LENGTH TO %04x\n"), adpcm_length);
		}
		break;
	case SIG_ADPCM_VCLK:
		do_vclk(flag);
		break;
	case SIG_ADPCM_DATA:
		if(write_buf > 0) {
			reg_0c |= ADPCM_REMAIN_WRITE_BUF;
			write_buf--;
			if(write_buf == 0) {
				reg_0c &= ~ADPCM_REMAIN_WRITE_BUF;
			}
		} else {
			reg_0c &= ~ADPCM_REMAIN_WRITE_BUF;
			ram[write_ptr & 0xffff] = data;
			write_ptr = (write_ptr + 1) & 0xffff;
			//written_size++;
		}
		break;
	case SIG_ADPCM_FADE_IN:
		fade_in(data);
		break;
	case SIG_ADPCM_FADE_OUT:
		fade_out(data);
		break;
	case SIG_ADPCM_SET_DIVIDER:
		adpcm_clock_divider = 0x10 - (data & 0x0f);
		d_msm->change_clock_w((ADPCM_CLOCK / 6) / adpcm_clock_divider);
		break;
	}
}
	
void ADPCM::do_cmd(uint8_t cmd)
{
	// Register 0x0d.
	//out_debug_log(_T("ADPCM CMD=%02x\n"), cmd);
	if(((cmd & 0x80) != 0) /*&& ((msm_last_cmd & 0x80) == 0)*/) {
		// Reset ADPCM
		reset_adpcm();
		msm_init(); // SAMPLE = 0x800, SSI=0
		out_debug_log(_T("ADPCM CMD RESET\n"));
		msm_last_cmd = cmd;
		return;
	}
	bool req_play = false;
	bool req_play_0 =   ((cmd & 0x40) != 0) ? true : false;
	adpcm_repeat = ((cmd & 0x20) != 0) ? true : false;
	uint32_t _clk = (ADPCM_CLOCK / 6) / adpcm_clock_divider;
		
	if((play_in_progress) && !(adpcm_repeat)) {
		req_play = false;
	}
	if(!(play_in_progress) && (adpcm_repeat)) {
		d_msm->change_clock_w((ADPCM_CLOCK / 6) / adpcm_clock_divider);  // From mednafen 1.22.1
		d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
		req_play = true;
		msm_nibble = 0;
		msm_init(); // SAMPLE = 0x800, SSI=0
	}
	//if(req_play_0) {
	//	req_play = true;
	//} else {
	//	if(/*(play_in_progress) && */(written_size > 0x8000)) {
	//		req_play = true;
	//	}
	//}
	if(((cmd & 0x10) != 0) /*&& ((msm_last_cmd & 0x10) == 0)*/){
		// ADPCM set length
		d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
		//adpcm_stopped = false;
		adpcm_length = (uint32_t)(addr_reg.w);
		out_debug_log(_T("ADPCM SET LENGTH TO %04x\n"), adpcm_length);
	}
	
	if(((cmd & 0x08) != 0) /*&& ((msm_last_cmd & 0x08) == 0)*/) {
		// ADPCM set read address
		read_ptr = (uint32_t)(addr_reg.w);
		read_buf = ((cmd & 0x04) == 0) ? 2 : 1;
		read_ptr = ((cmd & 0x04) == 0) ? ((read_ptr - 1) & 0xffff) : read_ptr;
		msm_ptr = read_ptr;
		out_debug_log(_T("ADPCM SET READ ADDRESS ADDR=%04x BUF=%01x \n"), read_ptr, read_buf);
	}
	if(((cmd & 0x02) != 0) /*&& ((msm_last_cmd & 0x02) == 0)*/) {
		// ADPCM set write address
		write_ptr = (uint32_t)(addr_reg.w);
		write_buf = ((cmd & 0x01) == 0) ? 1 : 0;
		//written_size = 0;
		write_ptr = (write_ptr - write_buf) & 0xffff;
		// It's ugly... (;_;)
		if(((read_ptr & 0xffff) >= 0x4000) &&
		 ((write_ptr & 0xffff) == 0x0000) &&
		 (adpcm_length != 0x8000) &&
		 (adpcm_length != 0xffff) &&
		 (_clk < 16000)) {
			adpcm_length = adpcm_length & 0x7fff;
		}
		out_debug_log(_T("ADPCM SET WRITE ADDRESS ADDR=%04x BUF=%01x \n"), write_ptr, write_buf);
	}
	if((req_play) && !(play_in_progress)) {
		msm_ptr = read_ptr & 0xffff;
		half_addr = (msm_ptr + (adpcm_length / 2)) & 0xffff;
		if(((adpcm_length & 0xffff) >= 0x8000) && ((adpcm_length & 0xffff) <= 0x80ff)) {
			half_addr = (read_ptr + 0x85) & 0xffff;
		}
		msm_nibble = 0;
		do_play();
		d_msm->reset_w(0);
		out_debug_log(_T("ADPCM START PLAY(%s) START=%04x LENGTH=%04x HALF=%04x\n"), (dma_enabled) ? _T("DMA") : _T("PIO"), msm_ptr, adpcm_length, half_addr);
	} else if((req_play) /*&& (cmd != msm_last_cmd)*/){
	if(((adpcm_length & 0xffff) >= 0x8000) && ((adpcm_length & 0xffff) <= 0x80ff)) {
			half_addr = (read_ptr + 0x85) & 0xffff;
		} else {
			half_addr = (read_ptr + (adpcm_length >> 1)) & 0xffff;
		}
		out_debug_log(_T("ADPCM UPDATE HALF ADDRESS HALF=%04x LENGTH=%04x\n"), half_addr, adpcm_length);
	} else /*if(cmd != msm_last_cmd)*/ { // !req_play
		//msm_nibble = 0;
		if(play_in_progress) {
			//adpcm_stopped = true;
			//play_in_progress = false;
			//d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
			//d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
			//do_stop(false);
		}
		//adpcm_stopped = true;
		out_debug_log(_T("ADPCM STATUS UPDATE PLAY=%s\n"), (play_in_progress) ? _T("YES") : _T("NO"));			
	}
	// used by Buster Bros to cancel an in-flight sample
	// if repeat flag (bit5) is high, ADPCM should be fully played (from Ootake)
	//if(!(req_play) && !(adpcm_repeat)) {
	//	d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
	//	d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
	//	do_stop(false);
	//	d_msm->reset_w(1);
	//}
	msm_last_cmd = cmd;
	
}

void ADPCM::msm_init()
{
	//d_msm->reset();
	//d_msm->reset_w(1);
	//d_msm->reset_w(0);
}

void ADPCM::do_vclk(bool flag)
{
	if((flag)) {
		uint8_t msm_data;
		{
			if(!(adpcm_stopped)) {
				msm_data = (msm_nibble != 0) ? (ram[msm_ptr & 0xffff] & 0x0f) : ((ram[msm_ptr & 0xffff] & 0xf0) >> 4);
				if(play_in_progress) {
					d_msm->data_w(msm_data);
				}
				msm_nibble ^= 1;
			}
			if(msm_nibble == 0) {
				if((written_size > 0) && !(adpcm_stopped)) written_size--;
				// Increment pointers.
				if((adpcm_length == 0) && ((msm_last_cmd & 0x10) == 0)) {
					if(!(adpcm_stopped)) {
						d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
						if((msm_last_cmd & 0x40) != 0) {
							do_stop(false);
							d_msm->reset_w(1); 
						}
						out_debug_log(_T("PLAY REACHED TO THE END ADDR=%08x SIZE=%04x LENGTH=%04x\n"), msm_ptr, written_size, adpcm_length);
						adpcm_stopped = true;
					} else {
						d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x0, 0xffffffff);
					}
				} else 	if((msm_ptr & 0xffff) == (half_addr & 0xffff)) {
					if(!(adpcm_stopped)) {
						d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
						out_debug_log(_T("PLAY PASSED TO THE HALF ADDR=%08x SIZE=%04x LENGTH=%04x\n"), msm_ptr, written_size, adpcm_length);
					}
				}
#if 0
				else if((!((dma_enabled) && (adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff)) &&
						   !(adpcm_length < 0x7fff)) &&
						  (((msm_ptr & 0xffff) == 0x8000) || ((msm_ptr & 0xffff) == 0x0000))) {
					// 20181213 K.O: Porting from Ootake v2.83.Thanks to developers of Ootake.
					//set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
					d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);

					out_debug_log(_T("SPECIAL HALF ADDRESS MSM_ADDR=%08x\n"), msm_ptr);
				}
#endif
				if(!(adpcm_stopped)) {
					msm_ptr = (msm_ptr + 1) & 0xffff;
					//read_ptr = msm_ptr;
				}
				if((msm_last_cmd & 0x10) == 0) {
					if(adpcm_length > 0) {
						adpcm_length--;
					} else {
						if(!(adpcm_stopped)) {
							d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x0, 0xffffffff);
							d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
							if((msm_last_cmd & 0x40) != 0) {
								do_stop(false);
							}
							out_debug_log(_T("PLAY REACHED TO THE END2 ADDR=%08x SIZE=%04x LENGTH=%04x\n"), msm_ptr, written_size, adpcm_length);
							adpcm_stopped = true;
						}
					}
				}
#if 0
				if((adpcm_repeat) && (dma_enabled) &&
				   (adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff) /* && (IsHuVideo()) */) {
					written_size++;
					msm_ptr = (msm_ptr - 1) & 0xffff;
					goto __skip1;
				}

				if(adpcm_length > 0) adpcm_length--;
				if(written_size > 0) written_size--;
				if((dma_enabled) && (written_size < 0xffff)) {
					if(d_pce->read_signal(SIG_PCE_CDROM_DATA_IN) != 0) {
						do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
					}
				}
				if(adpcm_length == 0) {
					d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x0, 0xffffffff);
					d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
					adpcm_stopped = true;
					out_debug_log(_T("PLAY PASSED TO THE FULL ADDR=%08x SIZE=%04x\n"), msm_ptr, written_size);
					do_stop(false);
				} else if((msm_ptr & 0xffff) == half_addr) {
					// 20181213 K.O: Porting from Ootake v2.83.Thanks to developers of Ootake.
					if((dma_enabled) && (adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff)) {
						half_addr = (half_addr + 0x85) & 0xffff;
					} else if(adpcm_length < 0x7fff) {
						half_addr = (half_addr + (uint16_t)(adpcm_length - 1024)) & 0xffff;
					}
					d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
					d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
					out_debug_log(_T("PLAY PASSED TO THE HALF ADDR=%08x SIZE=%04x\n"), msm_ptr, written_size);
				} /*else if((msm_ptr & 0xffff) == ((half_addr + 1) & 0xffff)) {
					d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
					d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
				} */else if((!((dma_enabled) && (adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff)) &&
							 !(adpcm_length < 0x7fff))) {
					if(((msm_ptr & 0xffff) == 0x8000) || ((msm_ptr & 0xffff) == 0x0000)) {
						// 20181213 K.O: Porting from Ootake v2.83.Thanks to developers of Ootake.
						//set_cdrom_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
						d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
						d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
						adpcm_stopped = false;
						out_debug_log(_T("SPECIAL HALF ADDRESS MSM_ADDR=%08x\n"), msm_ptr);
					} else if(((msm_ptr & 0xffff) == 0x8001) || ((msm_ptr & 0xffff) == 0x0001)) {
						//d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x0, 0xffffffff);
					}
				}
				msm_ptr++;
#endif
#if 1
			__skip1:
				if((dma_enabled) && (written_size <= 0)) {
					if(d_pce->read_signal(SIG_PCE_CDROM_DATA_IN) != 0) {
						do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
					}
				} else {
							
				}
#endif
			} else { // nibble = 1
			}
		}
	}
}
	
bool ADPCM::do_dma(uint8_t data)
{
	ram[write_ptr & 0xffff] = data;
	write_ptr = (write_ptr + 1) & 0xffff;
	written_size++;
	//set_ack(0);

	reg_0c &= ~ADPCM_REMAIN_WRITE_BUF;
	return true;
}


void ADPCM::set_ack(int clocks)
{
	if(event_ack != -1) cancel_event(this, event_ack);
	event_ack = -1;
	if(clocks <= 0) {
		d_pce->write_signal(SIG_PCE_CDROM_SET_ACK, 0xff, 0xff);
	} else {
		double us = (((double)clocks) * 1.0e6) / ((double)CPU_CLOCKS);
		register_event(this, EVENT_SET_ACK, us, false, &event_ack);
	}
}

void ADPCM::clear_ack(int clocks)
{
	if(event_ack != -1) cancel_event(this, event_ack);
	event_ack = -1;
	if(clocks <= 0) {
		d_pce->write_signal(SIG_PCE_CDROM_CLEAR_ACK, 0xff, 0xff);
	} else {
		double us = (((double)clocks) * 1.0e6) / ((double)CPU_CLOCKS);
		register_event(this, EVENT_CLEAR_ACK, us, false, &event_ack);
	}
}

void ADPCM::fade_in(int usec)
{
	if(event_fader != -1) {
		cancel_event(this, event_fader);
	}
	register_event(this, EVENT_FADE_IN, (double)usec, true, &event_fader);
	adpcm_volume = 0.0;
	d_msm->set_volume((int)adpcm_volume);
}

void ADPCM::fade_out(int usec)
{
	if(event_fader != -1) {
		cancel_event(this, event_fader);
	}
	register_event(this, EVENT_FADE_OUT, (double)usec, true, &event_fader);
	adpcm_volume = 100.0;
	d_msm->set_volume((int)adpcm_volume);
}
	
void ADPCM::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_CLEAR_ACK:
		event_ack = -1;
		d_pce->write_signal(SIG_PCE_CDROM_CLEAR_ACK, 0xff, 0xff);
		break;
	case EVENT_SET_ACK:
		event_ack = -1;
		d_pce->write_signal(SIG_PCE_CDROM_SET_ACK, 0xff, 0xff);
		break;
	case EVENT_FADE_IN:		
		if((adpcm_volume += 0.1) >= 100.0) {
			cancel_event(this, event_fader);
			event_fader = -1;
			adpcm_volume = 100.0;
		}
		d_msm->set_volume((int)adpcm_volume);
		break;
	case EVENT_FADE_OUT:		
		if((adpcm_volume -= 0.1) <= 0.0) {
			cancel_event(this, event_fader);
			event_fader = -1;
			adpcm_volume = 0.0;
		}
		d_msm->set_volume((int)adpcm_volume);
		break;
	}
}

void ADPCM::mix(int32_t* buffer, int cnt)
{
	d_msm->mix(buffer, cnt);
}

#define STATE_VERSION	7

bool ADPCM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(addr_reg);
	state_fio->StateValue(reg_0b);
	state_fio->StateValue(reg_0c);
	state_fio->StateValue(msm_last_cmd);
	
	state_fio->StateBuffer(ram, sizeof(ram), 1);
	
	state_fio->StateValue(read_ptr);
	state_fio->StateValue(read_buf);
	state_fio->StateValue(write_ptr);
	state_fio->StateValue(write_buf);
	
	state_fio->StateValue(msm_ptr);
	state_fio->StateValue(msm_nibble);
	state_fio->StateValue(half_addr);
	state_fio->StateValue(adpcm_length);
	
	state_fio->StateValue(written_size);
	state_fio->StateValue(dma_enabled);
	state_fio->StateValue(play_in_progress);
	state_fio->StateValue(adpcm_paused);
	state_fio->StateValue(adpcm_stream);
	state_fio->StateValue(adpcm_repeat);
	state_fio->StateValue(adpcm_stopped);	
	state_fio->StateValue(adpcm_volume);
	state_fio->StateValue(event_fader);
	state_fio->StateValue(event_ack);

	return true;
}

}

