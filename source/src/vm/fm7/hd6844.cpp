
#include "../memory.h"
#include "../vm.h"
#include "../../emu.h"

#include "hd6844.h"


HD6844::HD6844(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_emu = parent_emu;
	p_vm = parent_vm;
	init_output_signals(&interrupt_line);
	init_output_signals(&halt_line);
}

HD6844::~HD6844()
{
}

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
	}
	priority_reg = 0x00;
	interrupt_reg = 0x00;
	datachain_reg = 0x00;
	num_reg = 0x00;
	cycle_steal = false;
}

void HD6844::initialize()
{
	addr_offset = 0;
	int ch;
	for(ch = 0; ch < 4; ch++) {
		event_dmac[ch] = -1;
	}
   
}

void HD6844::write_data8(uint32 addr, uint32 data)
{
	uint8 ch = addr & 0x03;
	pair tmpd;
	uint32 channel = (addr >> 2) & 3;

	tmpd.d = 0;
	if(addr < 0x10) {
		switch(addr & 3) {
			case 0:
				tmpd.w.l = addr_reg[channel];
				tmpd.w.h = 0;
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


uint32 HD6844::read_data8(uint32 addr)
{
	uint8 ch = addr & 0x03;
	pair tmpd;
	uint32 channel = (addr >> 2) & 3; 
	uint32 retval = 0xff;
	
	tmpd.d = 0;
	if(addr < 0x10) {
		switch(addr & 3) {
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
		retval = ((datachain_reg >> 4) | 0x80) & interrupt_reg;
		interrupt_reg &= 0x7f;
		datachain_reg &= 0x0f;
		write_signals(&interrupt_line, 0x00);
	} else if(addr == 0x16) {
		retval = datachain_reg & 0x0f;
	}
	return retval;
}

uint32 HD6844::read_signal(int id)
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
  
void HD6844::write_signal(int id, uint32 data, uint32 mask)
{
	bool val_b = ((data & mask) != 0);
	uint32 ch = (data & mask) & 0x03;
	
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
			if((words_reg[ch] == 0) || (words_reg[ch] == 0xffff)) return;
			channel_control[ch] = channel_control[ch] & 0x8f;
			first_transfer[ch] = true;
			if(event_dmac[ch] < 0) register_event(this, HD6844_EVENT_START_TRANSFER + ch,
							      50.0, false, &event_dmac[ch]);
			break;
		case HD6844_DO_TRANSFER:
			if(!transfering[ch]) return;
			if(((channel_control[ch] & 0x02) != 0) && (!cycle_steal)) cycle_steal = true;
			if(!cycle_steal) {
				this->write_signals(&halt_line, 0xffffffff);
			}
			if(((words_reg[ch] & 0x0f) == 1) || (first_transfer[ch])){
				first_transfer[ch] = false;
				register_event(this, HD6844_EVENT_DO_TRANSFER + ch,
							   (double)(0x10 / 2), false, NULL); // HD68B44
			} else {
				do_transfer(ch);
			}
			break;
		default:
			break;
	}
}

void HD6844::do_transfer(int ch)
{
	if(!transfering[ch]) return;
	if((priority_reg & 0x01) == 0) {
		transfering[ch] = false;
		if(!cycle_steal) this->write_signals(&halt_line, 0);
		cycle_steal = false;
		return;
	}
	if(words_reg[ch] == 0) {
		transfering[ch] = false;
		if(!cycle_steal) this->write_signals(&halt_line, 0);
		cycle_steal = false;
		channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;  
		return;
	}
	//if(((channel_control[ch] & 0x02) != 0) && (!cycle_steal)) cycle_steal = true;
	
	if((channel_control[ch] & 0x01) == 0) {
		data_reg[ch] = src[ch]->read_io8(fixed_addr[ch]) & 0xff;
		dest[ch]->write_dma_io8((uint32)addr_reg[ch] + addr_offset, data_reg[ch]);
		//p_emu->out_debug_log(_T("HD6844: FIXED -> SRC: %04x, %02x\n"), addr_reg[ch] + addr_offset, data_reg[ch]);
	} else {
		data_reg[ch] = dest[ch]->read_dma_io8((uint32)addr_reg[ch] + addr_offset) & 0xff;
		src[ch]->write_io8(fixed_addr[ch], data_reg[ch]);
		//p_emu->out_debug_log(_T("HD6844: SRC -> FIXED: %04x, %02x\n"), addr_reg[ch] + addr_offset, data_reg[ch]);
	}
	words_reg[ch]--;
	if((channel_control[ch] & 0x08) != 0) {
		addr_reg[ch]--;
	} else {
		addr_reg[ch]++;
	}
	addr_reg[ch] = addr_reg[ch] & 0xffff;
	if(!cycle_steal) this->write_signals(&halt_line, 0);
	
	if(words_reg[ch] == 0) {
		if((datachain_reg & 0x07) == 0x01) {
			addr_reg[0] = addr_reg[3];
			addr_reg[1] = addr_reg[0];
			addr_reg[2] = addr_reg[1];
			addr_reg[3] = addr_reg[2];
			
			words_reg[0] = words_reg[3];
			words_reg[1] = words_reg[0];
			words_reg[2] = words_reg[1];
			words_reg[3] = 0;
		} else {
			transfering[ch] = false;
			cycle_steal = false;
			channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;
			datachain_reg = datachain_reg | 0x10;
			if((interrupt_reg & 0x01) != 0) {
				interrupt_reg |= 0x80;
				write_signals(&interrupt_line, 0xffffffff);
			}				  
			//p_emu->out_debug_log(_T("HD6844: Complete transfer ch %d\n"), ch);
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
		//p_emu->out_debug_log(_T("HD6844: Start to transfer ch %d Words = $%04x $%04x $%04x $%04x:\n"), ch, words_reg[0], words_reg[1], words_reg[2], words_reg[3]);
	} else 	if((event_id >= HD6844_EVENT_DO_TRANSFER) && (event_id < (HD6844_EVENT_DO_TRANSFER + 4))) {
		ch = event_id - HD6844_EVENT_DO_TRANSFER;
		do_transfer(ch);
	}
}
