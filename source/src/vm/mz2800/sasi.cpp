/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2018.05.25 -

	[ SASI I/F ]
*/

#include "sasi.h"
#include "../i8259.h"
#include "../upd71071.h"
#include "../scsi_host.h"

#define STATUS_INT	0x01
#define STATUS_IXO	0x04
#define STATUS_CXD	0x08
#define STATUS_MSG	0x10
#define STATUS_BSY	0x20
#define STATUS_ACK	0x40
#define STATUS_REQ	0x80

void SASI::reset()
{
	control = 0x00;
	irq_status = drq_status = false;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xa4:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
//		if(!d_host->read_signal(SIG_SCSI_IO)) {
			d_host->write_dma_io8(addr, data);
//		}
		break;
	case 0xa5:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		if((control & 0x20) != (data & 0x20)) {
			d_host->write_signal(SIG_SCSI_SEL, data, 0x20);
		}
		if((control & 0x08) != (data & 0x08)) {
			d_host->write_signal(SIG_SCSI_RST, data, 0x08);
		}
		control = data;
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	uint32_t val = 0;
	
	switch(addr & 0xff) {
	case 0xa4:
//		if(d_host->read_signal(SIG_SCSI_IO)) {
			val = d_host->read_dma_io8(addr);
//		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] in  %04X %02X\n"), addr, val);
		#endif
		return val;
	case 0xa5:
		val = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
//		      (d_host->read_signal(SIG_SCSI_ACK) ? STATUS_ACK : 0) |
		      (d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
		      (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
		      (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CXD : 0) |
		      (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IXO : 0) |
		      (irq_status                        ? STATUS_INT : 0);
		irq_status = false;
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] in  %04X %02X (REQ=%d,BSY=%d,MSG=%d,CxD=%d,IxO=%d)\n"), addr, val,
				(val & STATUS_REQ) ? 1 : 0,
				(val & STATUS_BSY) ? 1 : 0,
				(val & STATUS_MSG) ? 1 : 0,
				(val & STATUS_CXD) ? 1 : 0,
				(val & STATUS_IXO) ? 1 : 0);
		#endif
		return val | 0x02;
	}
	return 0xff;
}

void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0xa4, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	return read_io8(0xa4);
}

void SASI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SASI_IRQ:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] IRQ=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(control & 0x01) {
			d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR4, data, mask);
		}
		irq_status = ((data & mask) != 0);
		break;
	case SIG_SASI_DRQ:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] DRQ=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(control & 0x02) {
			d_dma->write_signal(SIG_UPD71071_CH0, data, mask);
		}
		drq_status = ((data & mask) != 0);
		break;
	}
}

#define STATE_VERSION	2

bool SASI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(control);
	state_fio->StateBool(irq_status);
	state_fio->StateBool(drq_status);
	return true;
}

bool SASI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(control);
	state_fio->StateBool(irq_status);
	state_fio->StateBool(drq_status);
	return true;
}
