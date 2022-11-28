/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2016.03.04-

	[ scsi ]
*/

#include "scsi.h"
#include "../i8237.h"
#include "../i8259.h"
#include "../scsi_host.h"

// control register
#define CTRL_SWEN	0x80
#define CTRL_IMSK	0x40
#define CTRL_ATN	0x10
#define CTRL_SEL	0x04
#define CTRL_DMAE	0x02
#define CTRL_RSTS	0x01

#define STATUS_REQ	0x80
#define STATUS_IO	0x40
#define STATUS_MSG	0x20
#define STATUS_CD	0x10
#define STATUS_BSY	0x08
#define STATUS_INT	0x02
#define STATUS_PERR	0x01

#define IMSK_PHASE	0x02
#define IMSK_EOP	0x01

#define STATUS_PHASE	0x02
#define STATUS_EOP	0x01

void SCSI::reset()
{
	ctrl_reg = 0;
	intm_reg = IMSK_PHASE; // DMA terminal count IRQ is disabled
	phase_status = eop_status = false;
}

void SCSI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x2f0:
		// data register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
//		if(ctrl_reg & CTRL_SWEN) {
			d_host->write_dma_io8(addr, data);
//		}
		break;
		
	case 0x2f1:
		// control register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		ctrl_reg = data;
//		if(ctrl_reg  & CTRL_SWEN) {
			d_host->write_signal(SIG_SCSI_RST, data, CTRL_RSTS);
			d_host->write_signal(SIG_SCSI_SEL, data, CTRL_SEL);
			d_host->write_signal(SIG_SCSI_ATN, data, CTRL_ATN);
//		}
		break;
		
	case 0x2f2:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		if(data & STATUS_PHASE) {
			phase_status = false;
		}
		if(data & STATUS_EOP) {
			eop_status = false;
		}
		if(!(phase_status || eop_status)) {
//			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, 0, 0);
		}
		break;
		
	case 0x2f3:
		// irq mask register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		intm_reg = data;
		break;
	}
}

uint32_t SCSI::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	
	switch(addr & 0xffff) {
	case 0x2f0:
		// data register
//		if(ctrl_reg & CTRL_SWEN) {
			value = d_host->read_dma_io8(addr);
//		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
		
	case 0x2f1:
		// status register
		value = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
		        (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IO  : 0) |
		        (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
		        (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CD  : 0) |
		        (d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
		        (phase_status || eop_status        ? STATUS_INT : 0) | 0x04;
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
		
	case 0x2f2:
		// irq status register
		value = (phase_status ? STATUS_PHASE : 0) | (eop_status ? STATUS_EOP : 0);
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
			if(intm_reg & IMSK_PHASE) {
				d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, data, mask);
			}
		}
		phase_status = ((data & mask) != 0);
		break;
		
	case SIG_SCSI_DRQ:
		if(ctrl_reg & CTRL_DMAE) {
			d_dma->write_signal(SIG_I8237_CH1, data, mask);
		}
		break;
		
	case SIG_SCSI_TC:
		if(ctrl_reg & CTRL_IMSK) {
			if(intm_reg & IMSK_EOP) {
				d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, data, mask);
			}
		}
		eop_status = ((data & mask) != 0);
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
	state_fio->StateValue(intm_reg);
	state_fio->StateValue(phase_status);
	state_fio->StateValue(eop_status);
	return true;
}

