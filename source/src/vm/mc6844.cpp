/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.04.14-

	[ MC6844 ]
*/

#include "mc6844.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define STAT_BUSY	0x40
#define STAT_DEND	0x80
#define IRQ_FLAG	0x80

void MC6844::initialize()
{
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (MC6844 DMAC)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

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
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT CH=%d ADDRESS=%04X\n"), (addr >> 2) & 3, dma[(addr >> 2) & 3].address_reg.w.l);
#endif
		break;
	case 0x01:
	case 0x05:
	case 0x09:
	case 0x0d:
		dma[(addr >> 2) & 3].address_reg.b.l = data;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT CH=%d ADDRESS=%04X\n"), (addr >> 2) & 3, dma[(addr >> 2) & 3].address_reg.w.l);
#endif
		break;
	case 0x02:
	case 0x06:
	case 0x0a:
	case 0x0e:
		dma[(addr >> 2) & 3].byte_count_reg.b.h = data;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT CH=%d BYTECOUNT=%04X\n"), (addr >> 2) & 3, dma[(addr >> 2) & 3].byte_count_reg.w.l);
#endif
		break;
	case 0x03:
	case 0x07:
	case 0x0b:
	case 0x0f:
		dma[(addr >> 2) & 3].byte_count_reg.b.l = data;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT CH=%d BYTECOUNT=%04X\n"), (addr >> 2) & 3, dma[(addr >> 2) & 3].byte_count_reg.w.l);
#endif
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		dma[addr & 3].channel_ctrl_reg &= (STAT_BUSY | STAT_DEND);
		dma[addr & 3].channel_ctrl_reg |= data & ~(STAT_BUSY | STAT_DEND);
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT CH=%d CH_CTRL=%02X\n"), addr & 3, dma[addr & 3].channel_ctrl_reg);
#endif
		break;
	case 0x14:
		priority_ctrl_reg = data;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT PRI_CTRL=%02X\n"), priority_ctrl_reg);
#endif
		break;
	case 0x15:
		interrupt_ctrl_reg &= IRQ_FLAG;
		interrupt_ctrl_reg |= data & ~IRQ_FLAG;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT INT_CTRL=%02X\n"), interrupt_ctrl_reg);
#endif
		break;
	case 0x16:
		data_chain_reg = data;
#ifdef _DMA_DEBUG_LOG
		this->out_debug_log(_T("DMA: OUT DAT_CHAIN=%02X\n"), data_chain_reg);
#endif
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

void MC6844::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	d_memory->write_dma_data8(addr, data);
}

uint32_t MC6844::read_via_debugger_data8(uint32_t addr)
{
	return d_memory->read_dma_data8(addr);
}

void MC6844::transfer(int ch)
{
	if(priority_ctrl_reg & (1 << ch)) {
//		if(dma[ch].byte_count_reg.w.l != 0) {
			if(dma[ch].channel_ctrl_reg & 0x01) {
				uint8_t data;
#ifdef USE_DEBUGGER
				if(d_debugger != NULL && d_debugger->now_device_debugging) {
					data = d_debugger->read_via_debugger_data8(dma[ch].address_reg.w.l);
				} else
#endif
				data = this->read_via_debugger_data8(dma[ch].address_reg.w.l);
				dma[ch].device->write_dma_io8(0, data);
			} else {
				uint8_t data = dma[ch].device->read_dma_io8(0);
#ifdef USE_DEBUGGER
				if(d_debugger != NULL && d_debugger->now_device_debugging) {
					d_debugger->write_via_debugger_data8(dma[ch].address_reg.w.l, data);
				} else
#endif
				this->write_via_debugger_data8(dma[ch].address_reg.w.l, data);
			}
			if(dma[ch].channel_ctrl_reg & 0x08) {
				dma[ch].address_reg.d--;
			} else {
				dma[ch].address_reg.d++;
			}
			if(--dma[ch].byte_count_reg.w.l == 0) {
				static const int dest[8] = {0, 1, 0, 1, 0, 1, 2, 3};
				if((data_chain_reg & 0x01) && ch == dest[(data_chain_reg >> 1) & 3]) {
					dma[ch].address_reg.w.l = dma[3].address_reg.w.l;
					dma[ch].byte_count_reg.w.l = dma[3].byte_count_reg.w.l;
				}
				dma[ch].channel_ctrl_reg |=  STAT_DEND;
				update_irq();
				dma[ch].channel_ctrl_reg &= ~STAT_BUSY;
			} else {
				dma[ch].channel_ctrl_reg |=  STAT_BUSY;
			}
//		}
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

#ifdef USE_DEBUGGER
bool MC6844::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
CH0 ADDR=FFFF COUNT=FFFF CTRL=FF ENABLE=1 MEM->I/O
CH1 ADDR=FFFF COUNT=FFFF CTRL=FF ENABLE=1 I/O->MEM
CH2 ADDR=FFFF COUNT=FFFF CTRL=FF ENABLE=1 MEM->I/O
CH3 ADDR=FFFF COUNT=FFFF CTRL=FF ENABLE=1 I/O->MEM
*/
	static const _TCHAR *dir[2] = {
		_T("I/O->MEM"), _T("MEM->I/O")
	};
	my_stprintf_s(buffer, buffer_len,
	_T("CH0 ADDR=%04X COUNT=%04X CTRL=%02X ENABLE=%d %s\n")
	_T("CH1 ADDR=%04X COUNT=%04X CTRL=%02X ENABLE=%d %s\n")
	_T("CH2 ADDR=%04X COUNT=%04X CTRL=%02X ENABLE=%d %s\n")
	_T("CH3 ADDR=%04X COUNT=%04X CTRL=%02X ENABLE=%d %s"),
	dma[0].address_reg.w.l, dma[0].byte_count_reg.w.l, dma[0].channel_ctrl_reg, (priority_ctrl_reg >> 0) & 1, dir[dma[0].channel_ctrl_reg & 1], 
	dma[1].address_reg.w.l, dma[1].byte_count_reg.w.l, dma[1].channel_ctrl_reg, (priority_ctrl_reg >> 1) & 1, dir[dma[1].channel_ctrl_reg & 1], 
	dma[2].address_reg.w.l, dma[2].byte_count_reg.w.l, dma[2].channel_ctrl_reg, (priority_ctrl_reg >> 2) & 1, dir[dma[2].channel_ctrl_reg & 1], 
	dma[3].address_reg.w.l, dma[3].byte_count_reg.w.l, dma[3].channel_ctrl_reg, (priority_ctrl_reg >> 3) & 1, dir[dma[3].channel_ctrl_reg & 1]);
	return true;
}
#endif

#define STATE_VERSION	1

bool MC6844::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		state_fio->StateValue(dma[i].address_reg.d);
		state_fio->StateValue(dma[i].byte_count_reg.d);
		state_fio->StateValue(dma[i].channel_ctrl_reg);
	}
	state_fio->StateValue(priority_ctrl_reg);
	state_fio->StateValue(interrupt_ctrl_reg);
	state_fio->StateValue(data_chain_reg);
	return true;
}

