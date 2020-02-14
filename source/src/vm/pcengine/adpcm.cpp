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
	adpcm_volume = 100.0;

	memset(ram, 0x00, sizeof(ram));
}

void ADPCM::reset()
{
	written_size = 0;

	reset_adpcm();
}

uint32_t ADPCM::read_signal(int ch)
{
	switch(ch) {
	case SIG_ADPCM_DATA:
		if(read_buf > 0) {
			// Don't need to modify FLAGS 20190212 K.O
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
		{
			uint8_t data = reg_0c;
			// Hack from Ootake v2.83.
			if((play_in_progress)) {
				data = data & ~0x85;
				data = data | 0x08;
			} else {
				data = data & ~0x0c;
				data = data | 0x01;
			}
			d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00000000, 0xffffffff);
			return data;
		}
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
	msm_data = 0x00;
	msm_ptr = half_addr = 0;
	msm_nibble = 0;
	msm_last_cmd = 0x00;
	msm_length = 0x00;
	
	addr_reg.w = 0x0000;
	adpcm_length = 0;
	half_addr = 0;

	reg_0b = 0x00; // OK?
	
	adpcm_stream = false;
	adpcm_repeat = false;
	dma_enabled = false;
	dma_connected = false;
	play_in_progress = false;
	adpcm_paused = false;

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
	reg_0c &= ~(ADPCM_REMAIN_READ_BUF | ADPCM_REMAIN_WRITE_BUF);
	d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00, 0xffffffff);
	d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00, 0xffffffff);
	do_stop(true);
	set_dma_status(false);
	//d_msm->reset();
	d_msm->reset_w(1);
	
	//d_msm->change_clock_w((ADPCM_CLOCK / 6) / adpcm_clock_divider);  // From mednafen 1.22.1
	adpcm_volume = 100.0;
	d_msm->set_volume((int)adpcm_volume);
	//memset(ram, 0x00, sizeof(ram));
	
	out_debug_log(_T("RESET ADPCM\n"));

}

void ADPCM::do_play()
{
	// From Ootake v2.86
	reg_0c &= ~(ADPCM_REMAIN_READ_BUF | ADPCM_REMAIN_WRITE_BUF | ADPCM_STOP_FLAG);
	reg_0c |= ADPCM_PLAY_FLAG;
	play_in_progress = true;
	adpcm_paused = false;
	d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00000000, 0xffffffff);
	d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);
}

void ADPCM::do_pause(bool pause)
{
	//if(!(play_in_progress)) return;
	if(pause) {
		reg_0c |= ADPCM_STOP_FLAG;
		reg_0c &= ~ADPCM_PLAY_FLAG;
		msm_last_cmd &= ~0x60;
		adpcm_paused = true;
		d_msm->reset_w(1);
		out_debug_log(_T("ADPCM PAUSE PLAY PTR=%04x\n"), msm_ptr);
		touch_sound();
	} else {
		adpcm_paused = false;
		touch_sound();
		reg_0c &= ~ADPCM_STOP_FLAG;
		reg_0c |= ADPCM_PLAY_FLAG;
		d_msm->reset_w(0);
		out_debug_log(_T("ADPCM UNPAUSE PLAY PTR=%04x\n"), msm_ptr);
	}
}

void ADPCM::do_stop(bool do_notify)
{
	touch_sound();
	reg_0c |= ADPCM_STOP_FLAG;
	reg_0c &= ~ADPCM_PLAY_FLAG;
	msm_last_cmd &= ~0x60;
	play_in_progress = false;
	if(do_notify) {
		set_dma_status(false);
//		d_pce->write_signal(SIG_PCE_ADPCM_DMA, 0x00000000, 0xffffffff);
	}
	out_debug_log(_T("ADPCM STOP PLAY PTR=%04x DMA CLEAR=%s\n"), msm_ptr, (do_notify) ? _T("YES") : _T("NO"));
	
}

void ADPCM::set_dma_status(bool flag)
{
	dma_enabled = flag;
	if(!(flag)) {
		dma_connected = false;
	}
	d_pce->write_signal(SIG_PCE_ADPCM_DMA, (flag) ? 0xffffffff : 0x00000000, 0xffffffff);
}

void ADPCM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	//if(ch != SIG_ADPCM_VCLK) out_debug_log(_T("WRITE_SIGNAL SIG=%d DATA=%04x MASK=%04x\n"), ch, data, mask);
	switch(ch) {
	case SIG_ADPCM_DMACTRL:
		if((data & 0x03) != 0) {
			set_dma_status(true);
			//d_pce->write_signal(SIG_PCE_ADPCM_DMA, 0xffffffff, 0xffffffff);
			//reg_0b |= 0x02;
		}
		reg_0b = data;
		break;
	case SIG_ADPCM_PAUSE:
		//do_pause(flag);
		break;
	case SIG_ADPCM_DMA_ENABLED:
		set_dma_status(flag);
		if((flag)/* && (flag != dma_enabled)*/) {
			dma_connected = true;
			reg_0c |= ADPCM_REMAIN_WRITE_BUF;
			written_size = 0;
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
		set_dma_status(true);
		dma_connected = true;
		do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
		break;
	case SIG_ADPCM_FORCE_DMA_TRANSFER:
		if(flag) {
			if(!(dma_connected)) written_size = 0;
			dma_connected = true;
			if(d_pce->read_signal(SIG_PCE_CDROM_DATA_IN) != 0) {
				do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
			}
		}
		break;
	case SIG_ADPCM_DMA_RELEASED:
		if(flag) {
			dma_connected = false;
		}
		break;
	case SIG_ADPCM_ADDR_HI: // REG $09
		if((msm_last_cmd & 0x80) == 0) {
		addr_reg.b.h = data;
			if((msm_last_cmd & 0x10) != 0) {
				update_length();
				out_debug_log(_T("SET ADDRESS REGISTER HIGH ; UPDATE LENGTH TO %04x\n"), adpcm_length);
			}
		}
		break;
	case SIG_ADPCM_ADDR_LO: // REG $08
		if((msm_last_cmd & 0x80) == 0) {
			addr_reg.b.l = data;
			if((msm_last_cmd & 0x10) != 0) {
				update_length();
				out_debug_log(_T("SET ADDRESS REGISTER LOW ; UPDATE LENGTH TO %04x\n"), adpcm_length);
			}
		}
		break;
	case SIG_ADPCM_VCLK:
		do_vclk(flag);
		break;
	case SIG_ADPCM_DATA:
		// Don't need to modify FLAGS 20190212 K.O
		if(write_buf > 0) {
			write_buf--;
			if(write_buf == 0) {
				reg_0c &= ~ADPCM_REMAIN_WRITE_BUF;
			}
		} else {
			reg_0c &= ~ADPCM_REMAIN_WRITE_BUF;
			ram[write_ptr & 0xffff] = data;
			write_ptr++;
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
	case SIG_ADPCM_CLEAR_ACK:
		reg_0b &= 0xfc; // Clear status register.
		break;
	}
}

void ADPCM::update_length()
{
	adpcm_length = (uint32_t)(addr_reg.w) & 0xffff; 
	msm_length = adpcm_length + 1;
	out_debug_log(_T("ADPCM SET LENGTH TO %04x\n"), adpcm_length);
}

void ADPCM::do_cmd(uint8_t cmd)
{
	// Register 0x0d.
	//out_debug_log(_T("ADPCM CMD=%02x\n"), cmd);
	if(((cmd & 0x80) != 0) && ((msm_last_cmd & 0x80) == 0)) {
		// Reset ADPCM
		reset_adpcm();
		//msm_init(); // SAMPLE = 0x800, SSIS=0
		out_debug_log(_T("ADPCM CMD RESET\n"));
		//msm_last_cmd = cmd;
		//return;
	}
	
	if(((cmd & 0x08) != 0) && ((msm_last_cmd & 0x08) == 0)) {
		// ADPCM set read address
		read_ptr = (uint32_t)(addr_reg.w) & 0xffff;
		read_buf = ((cmd & 0x04) == 0) ? 2 : 1;
		msm_ptr = read_ptr;
		msm_ptr = ((cmd & 0x04) == 0) ? ((msm_ptr - 1) & 0xffff) : msm_ptr;
		out_debug_log(_T("ADPCM SET READ ADDRESS ADDR=%04x BUF=%01x \n"), read_ptr, read_buf);
		//reg_0c |= ADPCM_REMAIN_READ_BUF;
		half_addr = (read_ptr + ((adpcm_length + 1) >> 1)) & 0xffff;
	}
	if(((cmd & 0x10) != 0) /*&& ((msm_last_cmd & 0x10) == 0)*/){
		// ADPCM set length
		update_length();
	}
	if(((cmd & 0x02) != 0) && ((msm_last_cmd & 0x02) == 0)) {
		// ADPCM set write address
		write_ptr = (uint32_t)(addr_reg.w) & 0xffff;
		write_buf = ((cmd & 0x01) == 0) ? 1 : 0;
		write_ptr = (write_ptr - write_buf) & 0xffff;
		if(write_buf != 0) {
			reg_0c |= ADPCM_REMAIN_WRITE_BUF;
		}
		//written_size = written_size & 0xffff; // OK?
		written_size = 0; // OK?
	}
	if((cmd & 0x10) != 0) {
		// It's ugly... (;_;)
#if 1		
		uint32_t _clk = (ADPCM_CLOCK / 6) / adpcm_clock_divider;
		if(((read_ptr & 0xffff) >= 0x4000) &&
		   ((write_ptr & 0xffff) == 0x0000) &&
		   (adpcm_length != 0x8000) &&
		   (adpcm_length != 0xffff) &&
		   (_clk < 16000)) {
			adpcm_length = adpcm_length & 0x7fff;
		}
#endif
		half_addr = (read_ptr + ((adpcm_length + 1) >> 1)) & 0xffff;
		msm_length = adpcm_length + 1;
		out_debug_log(_T("ADPCM SET LENGTH LENGTH=%04x\n"), adpcm_length);
	}
	if(((cmd & 0x02) != 0) && ((msm_last_cmd & 0x02) == 0)) {
		if(((write_ptr & 0xffff) == 0) || ((write_ptr & 0xffff) == 0x8000) || ((write_ptr & 0x1fff) == 0x1fff)) {
			if((((read_ptr + adpcm_length) & 0x1fff) == 0x1fff) ||
			   ((read_ptr == 0) && (adpcm_length == 0x8000))) {
				adpcm_stream = true;
			}
		}
		out_debug_log(_T("ADPCM SET WRITE ADDRESS ADDR=%04x BUF=%01x STREAM=%s\n"), write_ptr, write_buf, (adpcm_stream) ? _T("YES") : _T("NO"));
	}
	//bool req_play = false;
	bool req_play = play_in_progress;
	adpcm_repeat = ((cmd & 0x20) != 0) ? true : false;
		
	if((req_play) && !(adpcm_stream) && !(adpcm_repeat)) {
		req_play = false;
	}
	if(!(req_play) && (adpcm_repeat)) {
		req_play = true;
	}
	if((cmd & 0x40) != 0) {
		req_play = true;
	} else {
		if(adpcm_stream) {
			if(written_size > 0x8000) {
//			if(msm_length > 0x8000) {
//				req_play = false;
			} else {
				msm_last_cmd = cmd;
				return; // Exit from command. 20190212 K.O
			}
		}
	}
	if(req_play) {
//		msm_ptr = read_ptr;
		if(/*((cmd & 0x40) != 0) && */!(play_in_progress)) {
			// ADPCM play
			half_addr = (read_ptr + ((adpcm_length + 1) >> 1)) & 0xffff;
			write_ptr &= 0xffff;
			//msm_ptr = msm_ptr & 0xffff;
			msm_ptr = read_ptr;
			msm_nibble = 0;
			play_in_progress = true;
			msm_length  = adpcm_length + 1; // OK?
			do_play();
			d_msm->reset_w(0);
			written_size = 0; // OK?
			out_debug_log(_T("ADPCM START PLAY(%s) START=%04x LENGTH=%04x HALF=%04x STREAM=%s\n"), (dma_enabled) ? _T("DMA") : _T("PIO"), msm_ptr, msm_length, half_addr, (adpcm_stream) ? _T("YES") : _T("NO"));
			msm_last_cmd = cmd;
		} else {
			// 20181213 K.O: Import from Ootake v2.83.Thanks to developers of Ootake.
			if((adpcm_repeat)
			   && ((adpcm_length & 0xffff) >= 0x8000)
			   && ((adpcm_length & 0xffff) <= 0x80ff)) {
				half_addr = (read_ptr + 0x85) & 0xffff;
			} else {
				half_addr = (read_ptr + ((adpcm_length + 1) >> 1)) & 0xffff;
			}
			out_debug_log(_T("ADPCM UPDATE HALF ADDRESS HALF=%04x\n"), half_addr);
		}
	} else  {
		
		if(play_in_progress) {
			if((msm_last_cmd & 0x40) != 0) {
				adpcm_stream = false;
				adpcm_repeat = false;
				msm_last_cmd = cmd;
				d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00000000, 0xffffffff);
				d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);
				do_stop(true); // true?
				d_msm->reset_w(1);
				return;
			}
		}
		
		msm_last_cmd = cmd;
//		adpcm_stream = false;
//		adpcm_repeat = false;
		d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00000000, 0xffffffff);
		d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);

		out_debug_log(_T("ADPCM STATUS UPDATE / STOP\n"));
	}
}


void ADPCM::msm_init()
{
	//d_msm->reset();
	//d_msm->reset_w(1);
	//d_msm->reset_w(0);
}

void ADPCM::do_vclk(bool flag)
{
	bool need_wait =false;
	if((flag)) {
		// 20190216 K.O: Must wait when dma enabled and PCM data will empty, when DMA transferring.
		if((play_in_progress) && !(adpcm_paused)) {
			if(((dma_enabled ) && (dma_connected)) &&
			   (/*(written_size < 0x200) && */((msm_ptr & 0x8000) == (write_ptr & 0x8000))
				/*&& ((write_ptr & 0xffff) <= (msm_ptr & 0xffff))*/)) { // OK?
				// ToDo: exception
				d_msm->pause_w(1);
				//d_msm->reset_w(1);
				return;
			}
		}
		{
			if((play_in_progress) && !(adpcm_paused)) {
				d_msm->pause_w(0);
				//d_msm->reset_w(0);
				//if(dma_enabled) {
				msm_data = (msm_nibble != 0) ? (ram[msm_ptr & 0xffff] & 0x0f) : ((ram[msm_ptr & 0xffff] & 0xf0) >> 4);
				d_msm->data_w(msm_data);
				msm_nibble ^= 1;
				if((msm_nibble == 0)) {
					// Increment pointers.
					// 20181213 K.O: Re-order sequence from Ootake v2.83.Thanks to developers of Ootake.
					//if(need_wait) goto __skip0;
					if((msm_length == 0) && ((msm_last_cmd & 0x10) == 0)) {
						if((adpcm_repeat) && ((adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff))) {
							need_wait = true;
							msm_length++;
						} else
						{
							d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0x00000000, 0xffffffff);
							d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0xffffffff, 0xffffffff);
							if(((msm_last_cmd & 0x40) != 0)){
								do_stop(false); // true?
								d_msm->reset_w(1);
							}
							adpcm_stream = false;
							adpcm_repeat = false;
						}
					}
								
			   
				__skip0:
					if(!(need_wait)) {
						if((written_size > 0) /*&& !(adpcm_stopped)*/) written_size--;
						
						msm_ptr++;
						read_ptr = msm_ptr & 0xffff;
						if(msm_length > 0) msm_length--;
						if((adpcm_repeat) && (adpcm_length >= 0x8000) && (adpcm_length <= 0x80ff)) {
							if((msm_ptr & 0xffff) == (half_addr & 0xffff)) {
								half_addr = half_addr + 0x85;
								d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
								d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);
							}						
						} else if(adpcm_length < 0x7fff) {
							if((msm_ptr & 0xffff) == (half_addr & 0xffff)) {
								half_addr = half_addr + ((adpcm_length - 1024) & 0xffff);
								d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
								d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);
							}						
						} else if(((msm_ptr & 0xffff) == 0x8000) || ((msm_ptr & 0xffff) == 0x0000)) {
							d_pce->write_signal(SIG_PCE_ADPCM_HALF, 0xffffffff, 0xffffffff);
							d_pce->write_signal(SIG_PCE_ADPCM_FULL, 0x00000000, 0xffffffff);
						}
					}

					// 20190216 K.O: DO NOT DMA WITHIN VCLK.MUST DO ONLY BY DRQ.
#if 0
					if(written_size <= 0x10) {
						if((dma_connected) && (dma_enabled)) {
							if(d_pce->read_signal(SIG_PCE_CDROM_DATA_IN) != 0) {
								do_dma(d_pce->read_signal(SIG_PCE_CDROM_RAW_DATA));
							}
						}
					}
#endif
//			__skip1:
				} else { // nibble = 1
				}
			} else {
			}
		}
	}
}

bool ADPCM::do_dma(uint8_t data)
{
	ram[write_ptr & 0xffff] = data;
	write_ptr = write_ptr + 1;
	if(written_size < 0) written_size = 0;
	written_size = written_size + 1;
	if(written_size >= 0x10000) written_size = 0x10000;
	msm_length++;
	//msm_length &= 0xffff;
	//if(msm_length > 0x10000) msm_length = 0x10000;
	set_ack(0);

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
	
	state_fio->StateValue(msm_data);
	state_fio->StateValue(msm_ptr);
	state_fio->StateValue(msm_nibble);
	state_fio->StateValue(msm_length);
	state_fio->StateValue(half_addr);
	state_fio->StateValue(adpcm_length);
	
	state_fio->StateValue(written_size);
	state_fio->StateValue(dma_enabled);
	state_fio->StateValue(dma_connected);
	state_fio->StateValue(play_in_progress);
	state_fio->StateValue(adpcm_paused);
	state_fio->StateValue(adpcm_stream);
	state_fio->StateValue(adpcm_repeat);
	state_fio->StateValue(adpcm_volume);
	state_fio->StateValue(event_fader);
	state_fio->StateValue(event_ack);

	return true;
}

}

