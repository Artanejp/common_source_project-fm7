/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2018.04.01-

	[ sasi i/f ]
*/

#include "sasi.h"
#include "../harddisk.h"
#include "../i8237.h"
#if !defined(SUPPORT_HIRESO)
#include "../i8255.h"
#endif
#include "../i8259.h"
#include "../scsi_host.h"
#include "../scsi_hdd.h"

#define CONTROL_CHEN	0x80
#define CONTROL_NRDSW	0x40
#define CONTROL_SEL	0x20
#define CONTROL_RST	0x08
#define CONTROL_DMAE	0x02
#define CONTROL_INTE	0x01

#define STATUS_REQ	0x80
#define STATUS_ACK	0x40
#define STATUS_BSY	0x20
#define STATUS_MSG	0x10
#define STATUS_CXD	0x08
#define STATUS_IXO	0x04
#define STATUS_INT	0x01

void SASI::reset()
{
	control = 0;
	bsy_status = prev_bsy_status = true;
	cxd_status = prev_cxd_status = true;
	ixo_status = prev_ixo_status = true;
	msg_status = prev_msg_status = true;
	req_status = prev_req_status = true;
	ack_status = prev_ack_status = true;
	irq_status = drq_status = false;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0080:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
//		if(control & CONTROL_CHEN) {
			d_host->write_dma_io8(addr, data);
//		}
		break;
		
	case 0x0082:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		control = data;
		if((prev_control & CONTROL_RST) != (control & CONTROL_RST)) {
			d_host->write_signal(SIG_SCSI_RST, data, CONTROL_RST);
		}
		if((prev_control & CONTROL_SEL) != (control & CONTROL_SEL)) {
			d_host->write_signal(SIG_SCSI_SEL, data, CONTROL_SEL);
		}
		if((prev_control & (CONTROL_DMAE | CONTROL_INTE)) != (control & (CONTROL_DMAE | CONTROL_INTE))) {
			update_signal();
		}
		prev_control = control;
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	
	switch(addr) {
	case 0x0080:
//		if(control & CONTROL_CHEN) {
			value = d_host->read_dma_io8(addr);
//		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
		
	case 0x0082:
		if(control & CONTROL_NRDSW) {
			value = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
//			        (d_host->read_signal(SIG_SCSI_ACK) ? STATUS_ACK : 0) |
			        (d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
			        (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
			        (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CXD : 0) |
			        (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IXO : 0) |
			        (irq_status                        ? STATUS_INT : 0);
//			irq_status = false;
			#ifdef _SCSI_DEBUG_LOG
				this->out_debug_log(_T("[SASI] in  %04X %02X (REQ=%d,BSY=%d,MSG=%d,CxD=%d,IxO=%d,DH=%02X,DL=%02X)\n"), addr, value,
					(value & STATUS_REQ) ? 1 : 0,
					(value & STATUS_BSY) ? 1 : 0,
					(value & STATUS_MSG) ? 1 : 0,
					(value & STATUS_CXD) ? 1 : 0,
					(value & STATUS_IXO) ? 1 : 0,
					vm->get_cpu(0)->read_debug_reg(_T("DH")), vm->get_cpu(0)->read_debug_reg(_T("DL")));
			#endif
		} else {
			value = 0;
			for(int i = 0; i < 2; i++) {
				HARDDISK *unit = d_hdd->get_disk_handler(i);
				uint32_t dt = 7, ct = 0;
				
				if(unit != NULL && unit->mounted()) {
					double size = unit->sector_num * unit->sector_size;
					int size_mb = (int)(size / 1024.0 / 1024.0 + 0.5);
					
					if(size_mb <= 6) {
						dt = 0;
					} else if(size_mb <= 11) {
						dt = 1;
					} else if(size_mb <= 16) {
						dt = 2;
					} else if(size_mb <= 21) {
						if(unit->surfaces != 4) {
							dt = 3;
						} else {
							dt = 4;
						}
					} else if(size_mb <= 31) {
						dt = 5;
					} else {
						dt = 6;
					}
					if(unit->sector_size == 512) {
						ct = 1;
					}
				}
				value |= dt << (i == 0 ? 3 : 0);
				value |= ct << (i == 0 ? 7 : 6);
			}
			#ifdef _SCSI_DEBUG_LOG
				this->out_debug_log(_T("[SASI] in  %04X %02X (NRDSW=0)\n"), addr, value);
			#endif
		}
		return value;
	}
	return 0xff;
}

void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	#ifdef _SCSI_DEBUG_LOG
		this->out_debug_log(_T("[SASI] DMA out %02X\n"), data);
	#endif
	d_host->write_dma_io8(addr, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	uint32_t val = d_host->read_dma_io8(addr);
	#ifdef _SCSI_DEBUG_LOG
		this->out_debug_log(_T("[SASI] DMA in  %02X\n"), val);
	#endif
	return val;
}

void SASI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SASI_BSY:
		bsy_status = ((data & mask) == 0);
		update_signal();
		prev_bsy_status = bsy_status;
		break;
	case SIG_SASI_CXD:
		cxd_status = ((data & mask) == 0);
		update_signal();
		prev_cxd_status = cxd_status;
		break;
	case SIG_SASI_IXO:
		ixo_status = ((data & mask) == 0);
		update_signal();
		prev_ixo_status = ixo_status;
		break;
	case SIG_SASI_MSG:
		msg_status = ((data & mask) == 0);
		update_signal();
		prev_msg_status = msg_status;
		break;
	case SIG_SASI_REQ:
		req_status = ((data & mask) == 0);
		update_signal();
		prev_req_status = req_status;
		break;
	case SIG_SASI_ACK:
		ack_status = ((data & mask) == 0);
		update_signal();
		prev_ack_status = ack_status;
		break;
	case SIG_SASI_TC:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] TC=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(data & mask) {
			control &= ~CONTROL_DMAE;
			update_signal();
			prev_control = control;
		}
		break;
	}
}

void SASI::update_signal()
{
	// http://retropc.net/ohishi/museum/mz1e30.htm
	bool prev_ic20_o11 = (!prev_req_status) && prev_cxd_status;
	bool prev_ic10_o8 = !(!prev_req_status && !prev_ixo_status && prev_msg_status && !prev_cxd_status);
	bool prev_ic18_o11 = !(!(prev_control & CONTROL_DMAE) && prev_ic20_o11);
	bool prev_ic18_o8 = !(prev_ic10_o8 && prev_ic18_o11);
	
	bool ic20_o11 = (!req_status) && cxd_status;
	bool ic10_o8 = !(!req_status && !ixo_status && msg_status && !cxd_status);
	bool ic18_o11 = !(!(control & CONTROL_DMAE) && ic20_o11);
	bool ic18_o8 = !(ic10_o8 && ic18_o11);
	
	bool prev_irq_status = irq_status;
	bool prev_drq_status = drq_status;
	
	if(!prev_ic18_o8 && ic18_o8) {
		irq_status = ((control & CONTROL_INTE) != 0);
	}
	if(!prev_ic20_o11 && ic20_o11) {
		drq_status = ((control & CONTROL_DMAE) != 0);
	}
	if((prev_control & CONTROL_INTE) && !(control & CONTROL_INTE)) {
		irq_status = false;
	}
	if((prev_control & CONTROL_DMAE) && !(control & CONTROL_DMAE)) {
		drq_status = false;
	}
	if(prev_ack_status && !ack_status) {
		drq_status = false;
	}
#ifdef _SCSI_DEBUG_LOG
	if(prev_irq_status != irq_status) {
		this->out_debug_log(_T("[SASI] irq_status=%d\n"), irq_status ? 1 : 0);
	}
	if(prev_drq_status != drq_status) {
		this->out_debug_log(_T("[SASI] drq_status=%d\n"), drq_status ? 1 : 0);
	}
#endif
#if !defined(SUPPORT_HIRESO)
	d_pio->write_signal(SIG_I8255_PORT_B, (irq_status ? 0x10 : 0), 0x10);
#endif
	d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR1, (irq_status ? 1 : 0), 1);
#ifdef _PC98XA
	d_dma->write_signal(SIG_I8237_CH3, (drq_status ? 1 : 0), 1);
#else
	d_dma->write_signal(SIG_I8237_CH0, (drq_status ? 1 : 0), 1);
#endif
}

#define STATE_VERSION	3

bool SASI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(control);
	state_fio->StateValue(prev_control);
	state_fio->StateValue(bsy_status);
	state_fio->StateValue(prev_bsy_status);
	state_fio->StateValue(cxd_status);
	state_fio->StateValue(prev_cxd_status);
	state_fio->StateValue(ixo_status);
	state_fio->StateValue(prev_ixo_status);
	state_fio->StateValue(msg_status);
	state_fio->StateValue(prev_msg_status);
	state_fio->StateValue(req_status);
	state_fio->StateValue(prev_req_status);
	state_fio->StateValue(ack_status);
	state_fio->StateValue(prev_ack_status);
	state_fio->StateValue(irq_status);
	state_fio->StateValue(drq_status);
	return true;
}

