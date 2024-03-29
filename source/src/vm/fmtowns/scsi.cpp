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
#include "./dmac.h"

#undef _SCSI_DEBUG_LOG
// control register
#define CTRL_WEN	0x80
#define CTRL_IMSK	0x40
#define CTRL_RMSK	0x20
#define CTRL_ATN	0x10
#define CTRL_WB		0x08
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

namespace FMTOWNS {

void SCSI::reset()
{
	ctrl_reg = CTRL_IMSK;
	irq_status = false;
	irq_status_bak = false;
	exirq_status = false;
	ex_int_enable = false;
	dma_enabled = true;
//	dma_enabled = false;
}

void SCSI::write_io8(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("Write I/O %04X %02X"), addr, data);
	switch(addr & 0xffff) {
	case 0x0c30:
		// data register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
//			d_host->write_signal(SIG_SCSI_REQ, 0, 0);
		if(ctrl_reg & CTRL_WEN) {
//		if((ctrl_reg & CTRL_WEN) && !(ctrl_reg & CTRL_DMAE)) {
			d_host->write_dma_io8(addr, data);
		}
		break;

	case 0x0c32:
		// control register
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] out %04X %02X\n"), addr, data);
		#endif
		ctrl_reg = data;
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			ex_int_enable = ((data & 0x20) != 0) ? true : false;
			// Set host to 16bit bus width. BIT3 ,= '1'.
			//if(d_dma != NULL) {
			//	d_dma->write_signal(SIG_UPD71071_UBE_CH1, 0xffffffff, 0xffffffff);
			//}
		}
		if(ctrl_reg  & CTRL_WEN) {
			d_host->write_signal(SIG_SCSI_RST, data, CTRL_RST);
			d_host->write_signal(SIG_SCSI_ATN, data, CTRL_ATN);
			d_host->write_signal(SIG_SCSI_SEL, data, CTRL_SEL);
			if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
#if defined(USE_QUEUED_SCSI_TRANSFER)
				d_host->write_signal(SIG_SCSI_HOST_DMAE, data, CTRL_DMAE);
#endif
				//d_host->write_signal(SIG_SCSI_DATA_PHASE_INTR, data, CTRL_RMSK); // DISABLE/ENABLE INTERRUPT when DATA PHASE.
				//d_host->write_signal(SIG_SCSI_16BIT_BUS, data, CTRL_WB);  // WORD/BYTE

			}
		}
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			dma_enabled = ((data & CTRL_DMAE) != 0) ? true : false;
		}
		break;
	}
}


uint32_t SCSI::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	switch(addr & 0xffff) {
	case 0x0034:
//		if(machine_id >= 0x0600) { // After UG
//			value = 0x7f; // Ready to transfer 16bit width DMA, excepts CX/UX.
//		} else {
			value = 0xff;
//		}
		break;
	case 0x0c30:
		// data register
//		d_host->write_signal(SIG_SCSI_REQ, 0, 0);
//		if((ctrl_reg & CTRL_WEN) && !(ctrl_reg & CTRL_DMAE)) {
		if(ctrl_reg & CTRL_WEN) {
			value = d_host->read_dma_io8(addr);
		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
//		return value;
		break;
	case 0x0c32:
		// status register
		value = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
		        (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IO  : 0) |
		        (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
			    (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CD  : 0) |
				(d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
		        (irq_status                        ? STATUS_INT : 0);
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			value = value | 0x00;
		} else {
			value = value | 0x04; // Disable EX-Int.
		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI] in  %04X %02X\n"), addr, value);
		#endif
//		irq_status = false;
//		return value;
			break;
	case 0xc34:
		// From MAME 0.216
		// Linux uses this port to detect the ability to do word transfers.  We'll tell it that it doesn't for now.
		value = 0x80;
		break;
	}
//	out_debug_log(_T("[SCSI] READ I/O %04X %02X\n"), addr, value);
	return value;
}
#if 0
void SCSI::write_io16(uint32_t addr, uint32_t data)
{
	write_io8(addr & 0xfffe, data);
}
uint32_t SCSI::read_io16(uint32_t addr)
{
	return read_io8(addr & 0xfffe);
}
#endif
void SCSI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SCSI_16BIT_BUS:
		//transfer_16bit = ((data & mask) != 0) ? true : false;
		break;
	case SIG_SCSI_IRQ:
		irq_status_bak = irq_status;
		if((ctrl_reg & CTRL_IMSK)) {
//			if(irq_status_bak != irq_status) {
				d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR0, data, mask);
				//out_debug_log(_T("[SCSI] IRQ  %04X %02X\n"), data, mask);
//			}
		}
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			if(!(exirq_status)) {
				irq_status = ((data & mask) != 0);
			} else {
				irq_status = true;
			}
		} else {
			irq_status = ((data & mask) != 0);
		}
		break;

	case SIG_SCSI_DRQ:
		__LIKELY_IF(((ctrl_reg & CTRL_DMAE) != 0) /*&& (dma_enabled)*/) {
			__LIKELY_IF(d_dma != NULL) {
				d_dma->write_signal(SIG_UPD71071_CH1, data, mask);
			}
		}
/*		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			if(ex_int_enable) {
				d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR0, data, mask);
				exirq_status = ((data & mask) != 0);
				if(exirq_status) {
					irq_status = true;
				} else if(!(irq_status_bak)) {
					irq_status = false;
				}
			}
		}*/
		break;
	case SIG_SCSI_EOT:
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			dma_enabled = ((data & mask) == 0) ? true : false;
		}
		break;
	default:
		break;
	}
}

#define STATE_VERSION	4

bool SCSI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);

	state_fio->StateValue(ctrl_reg);
	state_fio->StateValue(irq_status);
	state_fio->StateValue(irq_status_bak);
	state_fio->StateValue(dma_enabled);

	if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
		state_fio->StateValue(ex_int_enable);
		state_fio->StateValue(exirq_status);
	}
	return true;
}
}
