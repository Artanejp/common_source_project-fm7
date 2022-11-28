/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2016.03.03-

	[ scsi ]
*/

#include "scsi.h"
#include "../i8259.h"
#include "../scsi_host.h"
#include "../upd71071.h"

// control register
#define CTRL_WEN	0x80
#define CTRL_IMSK	0x40
#define CTRL_ATN	0x10
#define CTRL_SEL	0x04
#define CTRL_DMAE	0x02
#define CTRL_RST	0x01

#define STATUS_REQ	0x80
#define STATUS_IO	0x40
#define STATUS_MSG	0x20
#define STATUS_CD	0x10
#define STATUS_BSY	0x08
#define STATUS_INT	0x02
#define STATUS_PERR	0x01

void SCSI::reset()
{
	ctrl_reg = CTRL_IMSK;
	irq_status = false;
}

void SCSI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xc30:
		// data register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		if(ctrl_reg & CTRL_WEN) {
			d_host->write_dma_io8(addr, data);
		}
		break;
		
	case 0xc32:
		// control register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		ctrl_reg = data;
		if(ctrl_reg  & CTRL_WEN) {
			d_host->write_signal(SIG_SCSI_RST, data, CTRL_RST);
			d_host->write_signal(SIG_SCSI_SEL, data, CTRL_SEL);
			d_host->write_signal(SIG_SCSI_ATN, data, CTRL_ATN);
		}
		break;
	}
}

uint32_t SCSI::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	
	switch(addr & 0xffff) {
	case 0xc30:
		// data register
		if(ctrl_reg & CTRL_WEN) {
			value = d_host->read_dma_io8(addr);
		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
		
	case 0xc32:
		// status register
		value = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
		        (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IO  : 0) |
		        (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
		        (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CD  : 0) |
		        (d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
		        (irq_status                        ? STATUS_INT : 0);
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
	}
	return 0xff;
}

void SCSI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SCSI_IRQ:
		if(ctrl_reg & CTRL_IMSK) {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR0, data, mask);
		}
		irq_status = ((data & mask) != 0);
		break;
		
	case SIG_SCSI_DRQ:
		if(ctrl_reg & CTRL_DMAE) {
			d_dma->write_signal(SIG_UPD71071_CH1, data, mask);
		}
		break;
	}
}

#define STATE_VERSION	1

bool SCSI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ctrl_reg);
	state_fio->StateValue(irq_status);
	return true;
}

