/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.04.14-

	[ MC6844 ]
*/

#include "mc6844.h"

#define STAT_BUSY	0x40
#define STAT_DEND	0x80
#define IRQ_FLAG	0x80

void MC6844::reset()
{
	for(int i = 0; i < 4; i++) {
		dma[i].address_reg.w.l = 0;
		dma[i].byte_count_reg.w.l = 0;
		dma[i].channel_ctrl_reg = 0;
	}
	priority_ctrl_reg = 0;
	interrupt_ctrl_reg = 0;
	data_chain_reg = 0;
}

void MC6844::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x1f) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0c:
		dma[(addr >> 2) & 3].address_reg.b.h = data;
		break;
	case 0x01:
	case 0x05:
	case 0x09:
	case 0x0d:
		dma[(addr >> 2) & 3].address_reg.b.l = data;
		break;
	case 0x02:
	case 0x06:
	case 0x0a:
	case 0x0e:
		dma[(addr >> 2) & 3].byte_count_reg.b.h = data;
		break;
	case 0x03:
	case 0x07:
	case 0x0b:
	case 0x0f:
		dma[(addr >> 2) & 3].byte_count_reg.b.l = data;
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		dma[addr & 3].channel_ctrl_reg &= (STAT_BUSY | STAT_DEND);
		dma[addr & 3].channel_ctrl_reg |= data & ~(STAT_BUSY | STAT_DEND);
		break;
	case 0x14:
		priority_ctrl_reg = data;
		break;
	case 0x15:
		interrupt_ctrl_reg &= IRQ_FLAG;
		interrupt_ctrl_reg |= data & ~IRQ_FLAG;
		break;
	case 0x16:
		data_chain_reg = data;
		break;
	}
}

uint32_t MC6844::read_io8(uint32_t addr)
{
	switch(addr & 0x1f) {
	case 0x00:
	case 0x04:
	case 0x08:
	case 0x0c:
		return dma[(addr >> 2) & 3].address_reg.b.h;
	case 0x01:
	case 0x05:
	case 0x09:
	case 0x0d:
		return dma[(addr >> 2) & 3].address_reg.b.l;
	case 0x02:
	case 0x06:
	case 0x0a:
	case 0x0e:
		return dma[(addr >> 2) & 3].byte_count_reg.b.h;
	case 0x03:
	case 0x07:
	case 0x0b:
	case 0x0f:
		return dma[(addr >> 2) & 3].byte_count_reg.b.l;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		if(dma[addr & 3].channel_ctrl_reg & STAT_DEND) {
			dma[addr & 3].channel_ctrl_reg &= ~STAT_DEND;
			update_irq();
			return dma[addr & 3].channel_ctrl_reg | STAT_DEND;
		}
		return dma[addr & 3].channel_ctrl_reg;
	case 0x14:
		return priority_ctrl_reg;
	case 0x15:
		return interrupt_ctrl_reg;
	case 0x16:
		return data_chain_reg;
	}
	return 0xff;
}

void MC6844::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MC6844_TX_RQ_0:
	case SIG_MC6844_TX_RQ_1:
	case SIG_MC6844_TX_RQ_2:
	case SIG_MC6844_TX_RQ_3:
		if(data & mask) {
			transfer(id - SIG_MC6844_TX_RQ_0);
		}
		break;
	}
}

void MC6844::transfer(int ch)
{
	if(priority_ctrl_reg & (1 << ch)) {
		if(dma[ch].byte_count_reg.w.l != 0) {
			if(dma[ch].channel_ctrl_reg & 0x01) {
				uint8_t data;
				data = d_memory->read_dma_data8(dma[ch].address_reg.w.l);
				dma[ch].device->write_dma_io8(0, data);
			} else {
				uint8_t data;
				data = dma[ch].device->read_dma_io8(0);
				d_memory->write_dma_data8(dma[ch].address_reg.w.l, data);
			}
			if(dma[ch].channel_ctrl_reg & 0x08) {
				dma[ch].address_reg.d--;
			} else {
				dma[ch].address_reg.d++;
			}
			if(--dma[ch].byte_count_reg.w.l == 0) {
				if(data_chain_reg & 0x01) {
					dma[(data_chain_reg >> 1) & 3].address_reg.w.l = dma[3].address_reg.w.l;
					dma[(data_chain_reg >> 1) & 3].byte_count_reg.w.l = dma[3].byte_count_reg.w.l;
				}
				dma[ch].channel_ctrl_reg |=  STAT_DEND;
				update_irq();
				dma[ch].channel_ctrl_reg &= ~STAT_BUSY;
			} else {
				dma[ch].channel_ctrl_reg |=  STAT_BUSY;
			}
		}
	}
}

void MC6844::update_irq()
{
	bool cur_flag = ((interrupt_ctrl_reg & IRQ_FLAG) != 0);
	bool new_flag = false;
	
	for(int i = 0; i < 4; i++) {
		if((dma[i].channel_ctrl_reg & STAT_DEND) && (interrupt_ctrl_reg & (1 << i))) {
			new_flag = true;
			break;
		}
	}
	if(cur_flag != new_flag) {
		if(new_flag) {
			interrupt_ctrl_reg |=  IRQ_FLAG;
		} else {
			interrupt_ctrl_reg &= ~IRQ_FLAG;
		}
		write_signals(&outputs_irq, new_flag ? 0xffffffff : 0);
	}
}

#define STATE_VERSION	1

void MC6844::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 4; i++) {
		state_fio->FputUint32(dma[i].address_reg.d);
		state_fio->FputUint32(dma[i].byte_count_reg.d);
		state_fio->FputUint8(dma[i].channel_ctrl_reg);
	}
	state_fio->FputUint8(priority_ctrl_reg);
	state_fio->FputUint8(interrupt_ctrl_reg);
	state_fio->FputUint8(data_chain_reg);
}

bool MC6844::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		dma[i].address_reg.d = state_fio->FgetUint32();
		dma[i].byte_count_reg.d = state_fio->FgetUint32();
		dma[i].channel_ctrl_reg = state_fio->FgetUint8();
	}
	priority_ctrl_reg = state_fio->FgetUint8();
	interrupt_ctrl_reg = state_fio->FgetUint8();
	data_chain_reg = state_fio->FgetUint8();
	return true;
}

