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

	[ PC-9801-26 ]
*/

#include "fmsound.h"
#ifdef _PC98_HAVE_86PCM
#include "fifo.h"
#endif

// From http://www.webtech.co.jp/company/doc/undocumented_mem/io_sound.txt .
// bit 7-4:
// 0000 : PC-98DO+
// 0001 : PC-98GS
// 0010 : PC-9801-73 (I/O = 0x018x)
// 0011 : PC-9801-73/76 (I/O 0x028x)
// 0100 : PC-9801-86(I/O = 0x018x),PC-9821/Ap/As/Ae/Af/Ap2/As2/An/Ap3/As3/Ce/Cs2/Ce2
// 0101 : PC-9801-86(I/O = 0x028x)
// 0110 : PC-9821Nf/Np
// 0111 : PC-9821Xt/Xa/Xf/Xn/Xp/Xs/Xa10/Xa9/Xa7/Xt13/Xe10C4/Xa12/Xa2e,PC-9821-XE10-B?
// 1000 : PC-9821Cf/Cx/Cb/Cx2/Cb2/Cx3/Cb3/Na7/Nx
// 1xx0 : Unknown
// 1111 : NO Sound or PC-9801-26.
// PC-98DO+
#if defined(_PC98DOPLUS)
#define BOARD_ID	0x00
#elif defined(_PC98GS)
#define BOARD_ID	0x10
#elif defined(SUPPORT_PC98_OPNA)
#define BOARD_ID	0x40
//#define BOARD_ID	0x50
#else
#define BOARD_ID	0xf0
#endif

#define EVENT_PCM 1

namespace PC9801 {

void FMSOUND::initialize()
{
#ifdef _PC98_HAVE_86PCM
	pcm_fifo = new FIFO(32768);
	play_bufsize = sizeof(play_pool) / sizeof(int32_t);
	play_w_remain = 0;
	play_rptr = 0;
	play_wptr = 0;
	memset(play_pool, 0x00, sizeof(int32_t) * play_bufsize);

	event_pcm = -1;
#endif
}

void FMSOUND::release()
{
#ifdef _PC98_HAVE_86PCM
	if(pcm_fifo != NULL) pcm_fifo->release();
#endif
}
	
void FMSOUND::reset()
{
	mask = (BOARD_ID & 0xf0) | 0; 
#ifdef _PC98_HAVE_86PCM
	// Will move to initialize()?
	fifo_enabled = false;
	pcm_freq = 44100;
	pcm_da_intleft = 128;
	// Q: Is clear FIFO?
	lrclock = false;
	fifo_direction = true; // PLAY
	fifo_int_status = false;
	fifo_int_flag = false;
	pcm_is_16bit = false;
	pcm_l_enabled = true;
	pcm_r_enabled = true;
	play_bufsize = sizeof(play_pool) / sizeof(int32_t);

	play_w_remain = 0;
	play_rptr = 0;
	play_wptr = 0;
	memset(play_pool, 0x00, sizeof(int32_t) * play_bufsize);

	mix_mod = 0;
	if(event_pcm >= 0) {
		cancel_event(this, event_pcm);
		event_pcm = -1;
	}
#endif
}

void FMSOUND::check_fifo_position()
{
#ifdef _PC98_HAVE_86PCM
	if(pcm_fifo != NULL) {
		if(pcm_da_intleft >= pcm_fifo->count()) {
			if(fifo_int_flag && !(fifo_int_status)) {
				//out_debug_log("PCM INTERRUPT at %d\n", pcm_da_intleft);
				fifo_int_status = true;
				write_signals(&outputs_int_pcm, 0xffffffff);
			}
		}
	}
#endif
}

void FMSOUND::mix(int32_t* buffer, int cnt)
{
	int ncount = 0;
	int32_t sample_l, sample_r;
	int32_t lastvol_l, lastvol_r;
#ifdef _PC98_HAVE_86PCM
	int lptr = play_rptr;
	if(lptr >= play_bufsize) lptr = 0;
	int rptr;

	if(((pcm_l_enabled) || (pcm_r_enabled)) && (play_w_remain > 0) && (fifo_direction) && (fifo_enabled)) { // Play
		if(pcm_freq == sample_rate) {
			int32_t* p = buffer;
			if((pcm_l_enabled) && (pcm_r_enabled)) {
				rptr = lptr + 1;
				if(rptr >= play_bufsize) rptr = 0;
				sample_l = play_pool[lptr];
				sample_r = play_pool[rptr];
			} else if(pcm_l_enabled) {
				rptr = lptr;
				sample_l = play_pool[lptr];
				sample_r = 0;
			} else if(pcm_r_enabled) {
				rptr = lptr;
				sample_r = play_pool[rptr];
				sample_l = 0;
			} 
			lastvol_l = apply_volume(sample_l, volume_l);
			lastvol_r = apply_volume(sample_r, volume_r);
			for(int i = 0; i < cnt; i++) {
				p[0] += lastvol_l;
				p[1] += lastvol_r;
				play_w_remain -= (((pcm_l_enabled) && (pcm_r_enabled)) ? 2 : 1);
				if(play_w_remain > 0) {
					if((pcm_l_enabled) && (pcm_r_enabled)) {
						lptr += 2;
						if(lptr >= play_bufsize) lptr = 0;
						rptr = lptr + 1;
						if(rptr >= play_bufsize) rptr = 0;
						sample_l = play_pool[lptr];
						sample_r = play_pool[rptr];
					} else if(pcm_l_enabled) {
						lptr++;
						if(lptr >= play_bufsize) lptr = 0;
						rptr = lptr;
						sample_l = play_pool[lptr];
						sample_r = 0;
					} else if(pcm_r_enabled) {
						lptr++;
						if(lptr >= play_bufsize) lptr = 0;
						rptr = lptr;
						sample_r = play_pool[rptr];
						sample_l = 0;
					} 
					lastvol_l = apply_volume(sample_l, volume_l);
					lastvol_r = apply_volume(sample_r, volume_r);
				} else {
					play_w_remain = 0;
				}
				p += 2;
				play_rptr = lptr;
			}
		} else if(pcm_freq < sample_rate) {
			int32_t* p = buffer;
			if((pcm_l_enabled) && (pcm_r_enabled)) {
				rptr = lptr + 1;
				if(rptr >= play_bufsize) rptr = 0;
				sample_l = play_pool[lptr];
				sample_r = play_pool[rptr];
			} else if(pcm_l_enabled) {
				rptr = lptr;
				sample_l = play_pool[lptr];
				sample_r = 0;
			} else if(pcm_r_enabled) {
				rptr = lptr;
				sample_r = play_pool[rptr];
				sample_l = 0;
			} 
			lastvol_l = apply_volume(sample_l, volume_l);
			lastvol_r = apply_volume(sample_r, volume_r);
			for(int i = 0; i < cnt; i++) {
				p[0] += lastvol_l;
				p[1] += lastvol_r;
				mix_mod = mix_mod + pcm_freq;
				if(mix_mod >= sample_rate) {
					mix_mod -= sample_rate;
					//play_w_remain -= (((pcm_l_enabled) && (pcm_r_enabled)) ? 2 : 1);
					if(play_w_remain > 0) {
						if((pcm_l_enabled) && (pcm_r_enabled)) {
							lptr += 2;
							if(lptr >= play_bufsize) lptr = 0;
							rptr = lptr + 1;
							if(rptr >= play_bufsize) rptr = 0;
							sample_l = play_pool[lptr];
							sample_r = play_pool[rptr];
							play_w_remain -= 2;
						} else if(pcm_r_enabled) {
							lptr++;
							if(lptr >= play_bufsize) lptr = 0;
							rptr = lptr;
							sample_r = play_pool[rptr];
							play_w_remain -= 1;
						} else if(pcm_l_enabled) {
							lptr++;
							if(lptr >= play_bufsize) lptr = 0;
							rptr = lptr;
							sample_l = play_pool[lptr];
							play_w_remain -= 1;
						}
						lastvol_l = apply_volume(sample_l, volume_l);
						lastvol_r = apply_volume(sample_r, volume_r);
					} else {
						play_w_remain = 0;
					}
				}
				p += 2;
				play_rptr = lptr;
			}
		} else if(pcm_freq > sample_rate) {
			int32_t* p = buffer;
			int min_skip = pcm_freq / sample_rate;
			if((pcm_l_enabled) && (pcm_r_enabled)) {
				rptr = lptr + 1;
				if(rptr >= play_bufsize) rptr = 0;
				sample_l = play_pool[lptr];
				sample_r = play_pool[rptr];
			} else if(pcm_l_enabled) {
				rptr = lptr;
				sample_l = play_pool[lptr];
				sample_r = 0;
			} else if(pcm_r_enabled) {
				rptr = lptr;
				sample_r = play_pool[rptr];
				sample_l = 0;
			} 
			lastvol_l = apply_volume(sample_l, volume_l);
			lastvol_r = apply_volume(sample_r, volume_r);
			int inc_factor;
			for(int i = 0; i < cnt; i++) {
				p[0] += lastvol_l;
				p[1] += lastvol_r;
				inc_factor = 0;
				mix_mod = mix_mod + (sample_rate * min_skip);
				if(mix_mod >= pcm_freq) {
					mix_mod -= pcm_freq;
					inc_factor = 1;
				}
				play_w_remain -= (((pcm_l_enabled) && (pcm_r_enabled)) ? ((min_skip + inc_factor) * 2) : (min_skip + inc_factor));
				if(play_w_remain > 0) {
					if((pcm_l_enabled) && (pcm_r_enabled)) {
						lptr += ((min_skip + inc_factor) * 2);
						if(lptr >= play_bufsize) lptr = lptr - play_bufsize;
						rptr = lptr + 1;
						if(rptr >= play_bufsize) rptr = 0;
						sample_l = play_pool[lptr];
						sample_r = play_pool[rptr];
					} else if(pcm_l_enabled) {
						lptr += (min_skip + inc_factor);
						if(lptr >= play_bufsize) lptr = lptr - play_bufsize;
						rptr = lptr;
						sample_l = play_pool[lptr];
						sample_r = 0;
					} else if(pcm_r_enabled) {
						lptr += (min_skip + inc_factor);
						if(lptr >= play_bufsize) lptr = lptr - play_bufsize;
						rptr = lptr;
						sample_r = play_pool[rptr];
						sample_l = 0;
					}
					lastvol_l = apply_volume(sample_l, volume_l);
					lastvol_r = apply_volume(sample_r, volume_r);
				} else {
					play_w_remain = 0;
				}
				p += 2;
				play_rptr = lptr;
			}
		}
	}
	//* ToDo: Mix PCM
#endif
}
	
void FMSOUND::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_PCM:
#ifdef _PC98_HAVE_86PCM
		lrclock = !lrclock;
		if((pcm_fifo != NULL)) {
			if(fifo_direction) { // Play
				pair16_t data;
				data.w = 0;
				if(!(pcm_fifo->empty())) {
					if(fifo_enabled) {
						if(pcm_is_16bit) {
							if(((lrclock) && (pcm_l_enabled)) || (!(lrclock) && (pcm_r_enabled))) {
								data.b.h = (uint8_t)(pcm_fifo->read());
								data.b.l = (uint8_t)(pcm_fifo->read());
								check_fifo_position();
							}
						} else {
							if(((lrclock) && (pcm_l_enabled)) || (!(lrclock) && (pcm_r_enabled))) {
								data.b.h = (uint8_t)(pcm_fifo->read());
								check_fifo_position();
								data.b.l = 0x00;
							}
						}
					}
					if(((lrclock) && (pcm_l_enabled)) || (!(lrclock) && (pcm_r_enabled))) {
						pcm_data.sd = (int32_t)(data.sw >> 1);
						play_pool[play_wptr++] = pcm_data.sd;
					}
					if(play_wptr >= play_bufsize) play_wptr = 0;
					play_w_remain++;
					if(play_w_remain >= play_bufsize) play_w_remain = play_bufsize;
				}
			} else { // ToDo: Record
				pcm_data.sd = 0;
				if(pcm_is_16bit) {
					pcm_fifo->write(pcm_data.b.h);
					check_fifo_position();
					pcm_fifo->write(pcm_data.b.l);
					check_fifo_position();
				} else {
					pcm_fifo->write(pcm_data.b.h);
					check_fifo_position();
				}
			}
		}
#endif
		break;
	}
}
	
void FMSOUND::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0188:
		if((mask & 2) == 0) {
			d_opn->write_io8(0, data);
		}
		break;
	case 0x018a:
		if((mask & 2) == 0) {
			d_opn->write_io8(1, data);
		}
		break;
#ifdef SUPPORT_PC98_OPNA
	case 0x018c:
		if((mask & 2) == 0) {
			if(mask & 1) {
				d_opn->write_io8(2, data);
			}
		}
		break;
	case 0x018e:
		if((mask & 2) == 0) {
			if(mask & 1) {
				d_opn->write_io8(3, data);
			}
		}
		break;
	case 0xa460:
		// bit 7-2: Unused
		// bit 1: Mask OPNA='1' (If set release OPNA).
		// bit 0: Using YM2608='1'
		mask = data;
		break;
#endif
#ifdef _PC98_HAVE_86PCM
	case 0xa466:
		pcm_volume_reg = data;
		// ToDo: Implement volumes
		break;
	case 0xa468:
		{
			if((pcm_ctrl_reg & 0x80) != (data & 0x80)) {
				fifo_enabled = ((data & 0x80) != 0);
				//out_debug_log("FIFO : %s\n", (fifo_enabled) ? "ENABLED" : "DISABLED");
			}
			if((pcm_ctrl_reg & 0x40) != (data & 0x40)) {
				fifo_direction = ((data & 0x40) == 0); // '1' = recording, '0' = playing.
				//out_debug_log("Update PCM FIFO TYPE : %s\n", (fifo_direction) ? "PLAY" : "REC");
			}
			if((pcm_ctrl_reg & 0x20) != (data & 0x20)) {
				fifo_int_flag = ((data & 0x20) != 0);
				//out_debug_log("Update PCM FIFO INT-MASK : %s\n", (fifo_int_flag) ? "YES" : "NO");
			}
			if(((pcm_ctrl_reg & 0x10) != 0) && ((data & 0x10) == 0)) {
				//out_debug_log("PCM CLEAR INTr\n");
				write_signals(&outputs_int_pcm, 0x00000000);
				fifo_int_status = false;
			}
			if((pcm_ctrl_reg & 0x08) != (data & 0x08)) {
				if((data & 0x08) != 0) {
					//out_debug_log("PCM RESET \n");
					play_w_remain = 0;
					play_rptr = 0;
					play_wptr = 0;
					memset(play_pool, 0x00, sizeof(int32_t) * play_bufsize);
					mix_mod = 0;
					pcm_fifo->clear();
				}
			}
			uint32_t freq;
			switch(data & 0x07) {
			case 0:
				freq = 44100;
				break;
			case 1:
				freq = 33080;
				break;
			case 2:
				freq = 22050;
				break;
			case 3:
				freq = 16540;
				break;
			case 4:
				freq = 11030;
				break;
			case 5:
				freq = 8270;
				break;
			case 6:
				freq = 5520;
				break;
			case 7:
				freq = 4130;
				break;
			}
			if(freq != pcm_freq) {
				if(event_pcm >= 0) {
					cancel_event(this, event_pcm);
					event_pcm = -1;
				}
				//out_debug_log("Update PCM FREQ=%d\n", freq);
				lrclock = false;
				pcm_freq = freq;
				mix_mod = 0;
			}
			if(event_pcm < 0) {
				register_event(this, EVENT_PCM, 1.0e6 / (double)(freq * 2), true, &event_pcm);
			}
		}
		pcm_ctrl_reg = data;
		break;
	case 0xa46a:
		if(!fifo_int_flag) {
			pcm_is_16bit = ((data & 0x40) == 0);
			pcm_l_enabled = ((data & 0x20) != 0);
			pcm_r_enabled = ((data & 0x10) != 0);
			pcm_da_reg = data;
			//out_debug_log("Update PCM TYPE: %s / %s%s\n", (pcm_is_16bit) ? "16bit" : "8bit", (pcm_l_enabled) ? "L" : " ", (pcm_r_enabled) ? "R" : " ");
		} else {
			if((data & 0xff) == 0xff) {
				pcm_da_intleft = 0x7ffc;
			} else {
				pcm_da_intleft = (int)(((uint32_t)((data & 0xff) + 1)) << 7);
			}
		}
	case 0xa46c:
		if(pcm_fifo != NULL) {
			//out_debug_log("FIFO DATA OUT %02x", data);
			//if((fifo_enabled) && (fifo_direction)) {
				pcm_fifo->write(data);
				//out_debug_log("FIFO DATA OUT %02x", data);
				//check_fifo_position();
			//}
		}
		break;
#endif		
	}
}

uint32_t FMSOUND::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0188:
		if((mask & 2) == 0) {
			return d_opn->read_io8(0);
		}
		break;
	case 0x018a:
		if((mask & 2) == 0) {
			return d_opn->read_io8(1);
		}
		break;
#ifdef SUPPORT_PC98_OPNA
	case 0x018c:
		if((mask & 2) == 0) {
			if(mask & 1) {
				return d_opn->read_io8(2);
			}
		}
		break;
	case 0x018e:
		if((mask & 2) == 0) {
			if(mask & 1) {
				return d_opn->read_io8(3);
			}
		}
		break;
	case 0xa460:
		// bit 3,2 :(Unused)
		// bit 1 : YM2608 Masking '1' = masked.
		// bit 0 : Having OPNA
		return BOARD_ID | (mask & 0x0f);
#endif
#ifdef _PC98_HAVE_86PCM
	case 0xa466:
		{
			uint8_t data = 0x00;
			if(pcm_fifo != NULL) {
				data = data | ((pcm_fifo->full()) ? 0x80 : 0x00);
				data = data | ((pcm_fifo->empty()) ? 0x40 : 0x00);
				//data = data | ((pcm_fifo->full()) ? 0x80 : 0x00);  // WIP: recording
			}
			data = data | ((lrclock) ? 0x01 : 0x00);
			return data;
		}
		break;
	case 0xa468:
		pcm_ctrl_reg = pcm_ctrl_reg & (uint8_t)(~0x10);
		pcm_ctrl_reg = pcm_ctrl_reg | ((fifo_int_status) ? 0x10 : 0x00);
		return pcm_ctrl_reg;
		break;
	case 0xa46a:
		return pcm_da_reg;
		break;
	case 0xa46c:
		//if((fifo_enabled) && !(fifo_direction)) {
			uint8_t data = pcm_fifo->read();
			//	check_fifo_position();
			return (uint32_t)data;
			//}
		break;
#endif
	}
	return 0xff;
}

void FMSOUND::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void FMSOUND::initialize_sound(int rate, int samples)
{
	sample_rate = rate;
	sample_samples = samples;
}
	
#define STATE_VERSION	2

bool FMSOUND::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#ifdef _PC98_HAVE_86PCM
	if(!(pcm_fifo->process_state(state_fio, loading))) {
		return false;
	}
#endif
	state_fio->StateValue(mask);
#ifdef _PC98_HAVE_86PCM
	state_fio->StateValue(pcm_ctrl_reg);
	state_fio->StateValue(pcm_da_reg);
	state_fio->StateValue(pcm_volume_reg);
	state_fio->StateValue(pcm_data);
	state_fio->StateValue(pcm_freq);

	state_fio->StateValue(fifo_enabled);
	state_fio->StateValue(fifo_direction);
	state_fio->StateValue(fifo_int_flag);
	state_fio->StateValue(fifo_int_status);

	state_fio->StateValue(lrclock);
	state_fio->StateValue(pcm_is_16bit);
	state_fio->StateValue(pcm_l_enabled);
	state_fio->StateValue(pcm_r_enabled);
	state_fio->StateValue(pcm_da_intleft);

	state_fio->StateValue(play_w_remain);
	state_fio->StateValue(play_rptr);
	state_fio->StateValue(play_wptr);
	state_fio->StateValue(mix_mod);
	state_fio->StateValue(event_pcm);
	
	state_fio->StateArray(play_pool, sizeof(play_pool), 1);
#endif
	return true;
}

}
