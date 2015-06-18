

#include "hd6844.h"


HD6844::HD6844(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_emu = parent_emu;
	p_vm = parent_vm;
	init_output_signals(&interrupt_line);
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
		
		channel_control[ch] = 0;
		transfering[ch] = false;
		if(event_dmac[ch] >= 0) cancel_event(this, event_dmac[ch]);
		event_dmac[ch] = -1;
	}
	priority_reg = 0x00;
	interrupt_reg = 0x00;
	datachain_reg = 0x00;
	num_reg = 0x00;
	burst = false;
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
	if(addr < 0x10) {
		switch(addr & 3) {
			case 0:
				tmpd.d = addr_reg[channel];
				tmpd.w.h = 0;
				tmpd.b.l = data & 0xff;
				addr_reg[channel] = tmpd.d;
				break;
			case 1:
				tmpd.d = addr_reg[channel];
				tmpd.w.h = 0;
				tmpd.b.h = data & 0xff;
				addr_reg[channel] = tmpd.d;
				break;
			case 2:
				tmpd.w.l = words_reg[channel];		  
				tmpd.b.l = data & 0xff;
				words_reg[channel] = tmpd.w.l;
				break;
			case 3:
				tmpd.w.l = words_reg[channel];		  
				tmpd.b.h = data & 0xff;
				words_reg[channel] = tmpd.w.l;
				break;
		}
	} else if((addr >= 0x10) && (addr < 0x14)) { // $10-$13
		channel_control[addr - 0x10] = data & 0xff;
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
				retval = tmpd.b.l & 0x00ff;
				break;
			case 1:
				tmpd.d = addr_reg[channel];
				retval = tmpd.b.h & 0x00ff;
				break;
			case 2:
				tmpd.w.l = words_reg[channel];
				retval = tmpd.b.l & 0x00ff;
				break;
			case 3:
				tmpd.w.l = words_reg[channel];
				retval = tmpd.b.h & 0x00ff;
				break;
		}
	} else if((addr >= 0x10) && (addr < 0x14)) { // $10-$13
		retval = channel_control[addr - 0x10] & 0x7f;
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
			if(words_reg[ch] == 0) return;
			if(event_dmac[ch] < 0) register_event(this, HD6844_EVENT_START_TRANSFER + ch,
							      50.0, false, &event_dmac[ch]);
			break;
		case HD6844_DO_TRANSFER:
			if(!transfering[ch]) return;
			if((priority_reg & 0x01) == 0) {
				transfering[ch] = false;
				burst = false;
				return;
			}
			if(words_reg[ch] == 0) {
				transfering[ch] = false;
				burst = false;
				channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;  
				return;
			}
			if(((channel_control[ch] & 0x02) != 0) && (!burst)) burst = true;
			
			if((channel_control[ch] & 0x01) != 0) {
				data_reg[ch] = src[ch]->read_data8(fixed_addr[ch]) & 0xff;
				dest[ch]->write_data8((uint32)addr_reg[ch] + addr_offset, data_reg[ch]);
			} else {
				data_reg[ch] = dest[ch]->read_data8((uint32)addr_reg[ch] + addr_offset) & 0xff;
				src[ch]->write_data8(fixed_addr[ch], data_reg[ch]);
			}			  
			words_reg[ch]--;
			if((channel_control[ch] & 0x08) != 0) {
				addr_reg[ch]--;
			} else {
				addr_reg[ch]++;
			}
			addr_reg[ch] = addr_reg[ch] & 0xffff;
			
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
					burst = false;
					channel_control[ch] = (channel_control[ch] & 0x0f) | 0x80;
					datachain_reg = datachain_reg | 0x10;
					if((interrupt_reg & 0x01) != 0) {
						interrupt_reg |= 0x80;
						write_signals(&interrupt_line, 0xffffffff);
					}				  
				}
			}
			break;
		default:
			break;
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
	}
}
