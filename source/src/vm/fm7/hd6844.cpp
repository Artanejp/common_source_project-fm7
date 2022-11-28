/*
 * DMAC HD6844/MC6844 [hd6844.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 18, 2015 : Initial
 *
 */

//#include "../memory.h"
//#include "../vm.h"
//#include "../../emu.h"

#include "hd6844.h"


void HD6844::reset()
{
	int ch;
	for(ch = 0; ch < 4; ch++) {
		addr_reg[ch] = 0xffff;
		words_reg[ch] = 0xffff;
		fixed_addr[ch] = 0x0000;
		data_reg[ch] = 0x00;
		first_transfer[ch] = false;
		
		channel_control[ch] = 0;
		transfering[ch] = false;
		if(event_dmac[ch] >= 0) cancel_event(this, event_dmac[ch]);
		event_dmac[ch] = -1;
		cycle_steal[ch] = false;
		halt_flag[ch] = false;
	}
	for(int i = 0; i < 2; i++) write_signals(&(busreq_line[i]), 0);
	write_signals(&interrupt_line, 0);
	priority_reg = 0x00;
	interrupt_reg = 0x00;
	datachain_reg = 0x00;
	num_reg = 0x00;
}

void HD6844::initialize()
{
	DEVICE::initialize();
	addr_offset = 0;
	int ch;
	for(ch = 0; ch < 4; ch++) {
		event_dmac[ch] = -1;
	}
	__USE_CHAINING = true;
	__USE_MULTIPLE_CHAINING = true;
#ifdef _MSC_VER
	#if defined(_FM77AV40)
		__FM77AV40 = true;
	#else
		__FM77AV40 = false;
	#endif
	#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		__FM77AV40EX = true;
	#else
		__FM77AV40EX = false;
	#endif
#else
	__FM77AV40 = osd->check_feature(_T("_FM77AV40"));	
	__FM77AV40EX = osd->check_feature(_T("_FM77AV40EX"));	
	__FM77AV40EX |= osd->check_feature(_T("_FM77AV40SX"));
#endif

	if(__FM77AV40 || __FM77AV40EX) __USE_MULTIPLE_CHAINING = false;
	if(__FM77AV40EX) __USE_CHAINING = false;
}

void HD6844::write_data8(uint32_t addr, uint32_t data)
{
	uint8_t ch = addr & 0x03;
	pair32_t tmpd;
	uint32_t channel = (addr >> 2) & 3;

	tmpd.d = 0;
	if(addr < 0x10) {
		switch(ch) {
			case 0:
				tmpd.w.l = addr_reg[channel];
				tmpd.b.h = data & 0xff;
				addr_reg[channel] = tmpd.d;
				break;
			case 1:
				tmpd.w.l = addr_reg[channel];
				tmpd.b.l = data & 0xff;
				addr_reg[channel] = tmpd.d;
				break;
			case 2:
				tmpd.w.l = words_reg[channel];		  
				tmpd.b.h = data & 0xff;
				words_reg[channel] = tmpd.w.l;
				break;
			case 3:
				tmpd.w.l = words_reg[channel];		  
				tmpd.b.l = data & 0xff;
				words_reg[channel] = tmpd.w.l;
				break;
		}
	} else if((addr >= 0x10) && (addr < 0x14)) { // $10-$13
		channel_control[addr - 0x10] = (channel_control[addr - 0x10] & 0xc0 ) | (data & 0x0f);
	} else if(addr == 0x14) {
		priority_reg = data & 0x8f;
	} else if(addr == 0x15) {
		interrupt_reg = (interrupt_reg & 0x80) | (data & 0x0f);
	} else if(addr == 0x16) {
		datachain_reg = (datachain_reg & 0xf0) | (data & 0x0f);
	}
}


uint32_t HD6844::read_data8(uint32_t addr)
{
	uint8_t ch = addr & 0x03;
	pair32_t tmpd;
	uint32_t channel = (addr >> 2) & 3; 
	uint32_t retval = 0xff;

	if(__FM77AV40EX) { // FM77AV40EX has only one channel.
		if((addr < 0x14) && (channel != 0)) return 0x00;;
	}
	tmpd.d = 0;
	if(addr < 0x10) {
		switch(ch) {
			case 0:
				tmpd.d = addr_reg[channel];
				retval = tmpd.b.h & 0x00ff;
				break;
			case 1:
				tmpd.d = addr_reg[channel];
				retval = tmpd.b.l & 0x00ff;
				break;
			case 2:
				tmpd.w.l = words_reg[channel];
				retval = tmpd.b.h & 0x00ff;
				break;
			case 3:
				tmpd.w.l = words_reg[channel];
				retval = tmpd.b.l & 0x00ff;
				break;
		}
	} else if((addr >= 0x10) && (addr < 0x14)) { // $10-$13
		retval = channel_control[addr - 0x10];
		channel_control[addr - 0x10] &= 0x7f;
	} else if(addr == 0x14) {
		retval = priority_reg;
	} else if(addr == 0x15) {
		int i;
		retval = ((datachain_reg >> 4) | 0x80) & interrupt_reg;
		interrupt_reg &= 0x7f;
		datachain_reg &= 0x0f;
		do_irq();
		//write_signals(&interrupt_line, 0x00); // Clear interrupt
	} else if(addr == 0x16) {
		retval = datachain_reg & 0x0f;
	}
	return retval;
}

uint32_t HD6844::read_signal(int id)
{
	switch(id) {
		case HD6844_SET_CONST_OFFSET:
			return addr_offset;
			break;
		case HD6844_SRC_FIXED_ADDR_CH0:
			return fixed_addr[0];
			break;
		case HD6844_SRC_FIXED_ADDR_CH1:
			return fixed_addr[1];
			break;
		case HD6844_SRC_FIXED_ADDR_CH2:
			return fixed_addr[2];
			break;
		case HD6844_SRC_FIXED_ADDR_CH3:
			return fixed_addr[3];
			break;
		case HD6844_ADDR_REG_0:
			return addr_reg[0];
			break;
		case HD6844_ADDR_REG_1:
			return addr_reg[1];
			break;
		case HD6844_ADDR_REG_2:
			return addr_reg[2];
			break;
		case HD6844_ADDR_REG_3:
			return addr_reg[3];
			break;
		case HD6844_WORDS_REG_0:
			return words_reg[0];
			break;
		case HD6844_WORDS_REG_1:
			return words_reg[1];
			break;
		case HD6844_WORDS_REG_2:
			return words_reg[2];
			break;
		case HD6844_WORDS_REG_3:
			return words_reg[3];
			break;
		case HD6844_IS_TRANSFER_0:
			return transfering[0] ? 0xffffffff : 0;
			break;
		case HD6844_IS_TRANSFER_1:
			return transfering[1] ? 0xffffffff : 0;
			break;
		case HD6844_IS_TRANSFER_2:
			return transfering[2] ? 0xffffffff : 0;
			break;
		case HD6844_IS_TRANSFER_3:
			return transfering[3] ? 0xffffffff : 0;
			break;
		default:
			break;
	}
	return 0x0;
}
  
void HD6844::write_signal(int id, uint32_t data, uint32_t mask)
{
	//bool val_b = ((data & mask) != 0);
	uint32_t ch = (data & mask) & 0x03;
	
	switch(id) {
		case HD6844_SET_CONST_OFFSET:
			addr_offset = data;
			break;
		case HD6844_SRC_FIXED_ADDR_CH0:
			fixed_addr[0] = data;
			break;
		case HD6844_SRC_FIXED_ADDR_CH1:
			fixed_addr[1] = data;
			break;
		case HD6844_SRC_FIXED_ADDR_CH2:
			fixed_addr[2] = data;
			break;
		case HD6844_SRC_FIXED_ADDR_CH3:
			fixed_addr[3] = data;
			break;
		case HD6844_TRANSFER_START:
			if(transfering[ch]) return;
			if((priority_reg & 0x01) == 0) return; // 20180117
			//if((words_reg[ch] == 0) || (words_reg[ch] == 0xffff)) return;
			if(words_reg[ch] == 0) return;
			channel_control[ch] = channel_control[ch] & 0x8f;
			first_transfer[ch] = true;
			cycle_steal[ch] = false;
			if((channel_control[ch] & 0x02) == 0) cycle_steal[ch] = true;	
			if(event_dmac[ch] >= 0) cancel_event(this, event_dmac[ch]);
			event_dmac[ch] = -1;
			if(event_dmac[ch] < 0) register_event(this, HD6844_EVENT_START_TRANSFER + ch,
							      50.0, false, &event_dmac[ch]);
			//this->out_debug_log(_T("DMAC: Start Transfer CH=%d $%04x Words, CMDREG=%02x"), ch, words_reg[ch], channel_control[ch]);
			break;
		case HD6844_ACK_BUSREQ_CLIENT:
			write_signals(&(busreq_line[HD6844_BUSREQ_CLIENT]), 0xffffffff);
			break;
		case HD6844_ACK_BUSREQ_HOST:
			write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0xffffffff);
			break;
		case HD6844_DO_TRANSFER:
			if(!transfering[ch]) return;

			if(((words_reg[ch] & 0x07) == 1) || (first_transfer[ch])){
				first_transfer[ch] = false;
				if(!cycle_steal[ch]) {
					write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0xffffffff);
				} else {
					if((channel_control[ch] & 0x04) != 0) {
						write_signals(&(busreq_line[HD6844_BUSREQ_CLIENT]), 0xffffffff);
					} else {
						write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0xffffffff);
					}
				}
				halt_flag[ch] = true;
				if(event_dmac[ch] >= 0) cancel_event(this, event_dmac[ch]);
				event_dmac[ch] = -1;
				register_event(this, HD6844_EVENT_DO_TRANSFER + ch,
							   (double)(0x08 / 2), false, NULL); // HD68B44
			} else {
				halt_flag[ch] = false;
				if(!cycle_steal[ch]) {
					write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0xffffffff); // Stop (HOST)line.
				} 
				do_transfer(ch);
			}
			break;
		default:
			break;
	}
}

void HD6844::do_irq(void)
{
	bool irq_stat = ((interrupt_reg & 0x80) != 0);
	bool req_irq = false;
	for(int cch = 0;cch < 4; cch++) {
		if((interrupt_reg & (1 << cch)) != 0) {
			if((channel_control[cch] & 0x80) != 0) {
				interrupt_reg |= 0x80;
				req_irq = true;
			}
		}
	}
	if(!(req_irq) && (irq_stat)) {
		write_signals(&interrupt_line, 0);
	} else if((req_irq) && !(irq_stat)) {
		write_signals(&interrupt_line, 0xffffffff);
	}
}

void HD6844::do_transfer_end(int ch)
{
	if(words_reg[ch] == 0) {
		transfering[ch] = false;
		if(event_dmac[ch] >= 0) {
			cancel_event(this, event_dmac[ch]);
			event_dmac[ch] = -1;
		}
		write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0); // release host_bus.
		cycle_steal[ch] = false;
		channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;
		datachain_reg = datachain_reg | 0x10;
		do_irq();
		//this->out_debug_log(_T("HD6844: Complete transfer ch %d\n"), ch);
	}	
}

void HD6844::do_transfer(int ch)
{
	ch = ch & 3;
	if(!transfering[ch]) return;
	if((priority_reg & 0x01) == 0) {
#if 0
		transfering[ch] = false;
		if(event_dmac[ch] >= 0) {
			cancel_event(this, event_dmac[ch]);
			event_dmac[ch] = -1;
		}
		
		if((channel_control[ch] & 0x04) != 0) {
			write_signals(&(busreq_line[HD6844_BUSREQ_CLIENT]), 0);
		} else {
			write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0);
		}
		cycle_steal[ch] = false;
#endif
		return;
	}
	if(words_reg[ch] == 0) {
		transfering[ch] = false;
		if(event_dmac[ch] >= 0) {
			cancel_event(this, event_dmac[ch]);
			event_dmac[ch] = -1;
		}
		if((channel_control[ch] & 0x04) != 0) {
			write_signals(&(busreq_line[HD6844_BUSREQ_CLIENT]), 0);
		} else {
			write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0);
		}
		cycle_steal[ch] = false;
		channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;  
		return;
	}
	if((channel_control[ch] & 0x01) == 0) {
		data_reg[ch] = src[ch]->read_io8(fixed_addr[ch]) & 0xff;
		dest[ch]->write_dma_io8((uint32_t)addr_reg[ch] + addr_offset, data_reg[ch]);
	} else {
		data_reg[ch] = dest[ch]->read_dma_io8((uint32_t)addr_reg[ch] + addr_offset) & 0xff;
		src[ch]->write_io8(fixed_addr[ch], data_reg[ch]);
	}
	words_reg[ch]--;
	if((channel_control[ch] & 0x08) != 0) {
		addr_reg[ch]--;
	} else {
		addr_reg[ch]++;
	}
	addr_reg[ch] = addr_reg[ch] & 0xffff;
	if(cycle_steal[ch] && halt_flag[ch]) {
		if(event_dmac[ch] >= 0) cancel_event(this, event_dmac[ch]);
		event_dmac[ch] = -1;
		halt_flag[ch] = false;
		register_event(this, HD6844_EVENT_END_TRANSFER + ch,
			      (double)(0x08 / 2 * 2), false, &event_dmac[ch]); // Really?
	}
	if(words_reg[ch] == 0) {
		if(((datachain_reg & 0x01) == 1) && __USE_CHAINING) {
			uint16_t tmp;
			uint8_t chain_ch = (datachain_reg & 0x06) >> 1;
			//20180117 K.O Is use multiple chaining?
			if(((datachain_reg & 0x08) != 0) && __USE_MULTIPLE_CHAINING) {
				//this->out_debug_log(_T("DMAC: chain 1->2->3->0(1/2) \n"));
				//if(chain_ch > 2) chain_ch = 2;
# if 1
				tmp = addr_reg[chain_ch];
# endif
				addr_reg[chain_ch] = addr_reg[(chain_ch + 3) & 3];
				addr_reg[(chain_ch + 3) & 3] = addr_reg[(chain_ch + 2) & 3];
				addr_reg[(chain_ch + 2) & 3] = addr_reg[(chain_ch + 1) & 3];
					
				words_reg[chain_ch] = words_reg[(chain_ch + 3) & 3];
				words_reg[(chain_ch + 3) & 3] = words_reg[(chain_ch + 2) & 3];
				words_reg[(chain_ch + 2) & 3] = words_reg[(chain_ch + 1) & 3];
# if 1
				//addr_reg[(chain_ch + 1) & 3] = tmp;
				words_reg[(chain_ch + 1) & 3] = 0;
# endif
			} else {
				// 20180117 K.O Is reset address reg?
				//if(chain_ch > 1) chain_ch = 1;
#if 1
				tmp = addr_reg[chain_ch];
#endif
				addr_reg[chain_ch] = addr_reg[3];
				words_reg[chain_ch] = words_reg[3];
#if 1
				//addr_reg[3] = tmp;
				words_reg[3] = 0;
#endif
				//do_irq();  // OK?
			}				
		} else {
			do_transfer_end(ch);
		}
	}
}	

void HD6844::event_callback(int event_id, int err)
{
	int ch;

	if((event_id >= HD6844_EVENT_START_TRANSFER) && (event_id < (HD6844_EVENT_START_TRANSFER + 4))) {
		ch = event_id - HD6844_EVENT_START_TRANSFER;
		event_dmac[ch] = -1;
		channel_control[ch] = (channel_control[ch] & 0x0f) | 0x40;
		transfering[ch] = true;
	} else 	if((event_id >= HD6844_EVENT_DO_TRANSFER) && (event_id < (HD6844_EVENT_DO_TRANSFER + 4))) {
		ch = event_id - HD6844_EVENT_DO_TRANSFER;
		event_dmac[ch] = -1;
		do_transfer(ch);
	} else if((event_id >= HD6844_EVENT_END_TRANSFER) && (event_id < (HD6844_EVENT_END_TRANSFER + 4))) {
		ch = event_id - HD6844_EVENT_END_TRANSFER;
		event_dmac[ch] = -1;
		if(cycle_steal[ch]) {
			if((channel_control[ch] & 0x04) != 0) {
				write_signals(&(busreq_line[HD6844_BUSREQ_CLIENT]), 0);
			} else {
				write_signals(&(busreq_line[HD6844_BUSREQ_HOST]), 0);
			}
		}
	}
}

#define STATE_VERSION 4

bool HD6844::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

	state_fio->StateArray(addr_reg, sizeof(addr_reg), 1);
	state_fio->StateArray(words_reg, sizeof(words_reg), 1);
	state_fio->StateArray(channel_control, sizeof(channel_control), 1);
	
	state_fio->StateValue(priority_reg);
	state_fio->StateValue(interrupt_reg);
	state_fio->StateValue(datachain_reg);
	state_fio->StateValue(num_reg);
	state_fio->StateValue(addr_offset);
		
	state_fio->StateArray(transfering, sizeof(transfering), 1);
	state_fio->StateArray(first_transfer, sizeof(first_transfer), 1);
	state_fio->StateArray(cycle_steal, sizeof(cycle_steal), 1);
	state_fio->StateArray(halt_flag, sizeof(halt_flag), 1);
		
	state_fio->StateArray(fixed_addr, sizeof(fixed_addr), 1);
	state_fio->StateArray(data_reg, sizeof(data_reg), 1);
	state_fio->StateArray(event_dmac, sizeof(event_dmac), 1);

	return true;
}

void HD6844::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
	out_debug_log(_T("Save State: HD6844: id=%d ver=%d"), this_device_id, STATE_VERSION);
}

bool HD6844::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	out_debug_log(_T("Load State: HD6844: id=%d stat=%s"), this_device_id, (mb) ? _T("OK") : _T("NG"));
	return mb;
}
