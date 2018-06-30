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
#include "../i8259.h"
#include "../scsi_host.h"
#include "../scsi_hdd.h"

#define OCR_CHEN	0x80
#define OCR_NRDSW	0x40
#define OCR_SEL		0x20
#define OCR_RST		0x08
#define OCR_DMAE	0x02
#define OCR_INTE	0x01

#define ISR_REQ		0x80
#define ISR_ACK		0x40
#define ISR_BSY		0x20
#define ISR_MSG		0x10
#define ISR_CXD		0x08
#define ISR_IXO		0x04
#define ISR_INT		0x01

void SASI::reset()
{
	ocr = 0;
	irq_status = drq_status = false;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0080:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
//		if(ocr & OCR_CHEN) {
			d_host->write_dma_io8(addr, data);
//		}
		break;
		
	case 0x0082:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_signal(SIG_SCSI_RST, data, OCR_RST);
		d_host->write_signal(SIG_SCSI_SEL, data, OCR_SEL);
		ocr = data;
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	
	switch(addr) {
	case 0x0080:
//		if(ocr & OCR_CHEN) {
			value = d_host->read_dma_io8(addr);
//		}
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] in  %04X %02X\n"), addr, value);
		#endif
		return value;
		
	case 0x0082:
		if(ocr & OCR_NRDSW) {
			value = (d_host->read_signal(SIG_SCSI_REQ) ? ISR_REQ : 0) |
//			        (d_host->read_signal(SIG_SCSI_ACK) ? ISR_ACK : 0) |
			        (d_host->read_signal(SIG_SCSI_BSY) ? ISR_BSY : 0) |
			        (d_host->read_signal(SIG_SCSI_MSG) ? ISR_MSG : 0) |
			        (d_host->read_signal(SIG_SCSI_CD ) ? ISR_CXD : 0) |
			        (d_host->read_signal(SIG_SCSI_IO ) ? ISR_IXO : 0) |
			        (irq_status                        ? ISR_INT : 0);
//			irq_status = false;
			#ifdef _SCSI_DEBUG_LOG
				this->out_debug_log(_T("[SASI] in  %04X %02X (REQ=%d,BSY=%d,MSG=%d,CxD=%d,IxO=%d,DH=%02X,DL=%02X)\n"), addr, value,
					(value & ISR_REQ) ? 1 : 0,
					(value & ISR_BSY) ? 1 : 0,
					(value & ISR_MSG) ? 1 : 0,
					(value & ISR_CXD) ? 1 : 0,
					(value & ISR_IXO) ? 1 : 0,
					vm->get_cpu(0)->read_debug_reg(_T("DH")), vm->get_cpu(0)->read_debug_reg(_T("DL")));
			#endif
		} else {
			value = 0;
			for(int i = 0; i < 2; i++) {
				HARDDISK *unit = d_hdd->get_disk_handler(i);
				uint32_t dt = 7, ct = 0;
				
				if(unit != NULL && unit->mounted()) {
					double size = unit->cylinders * unit->surfaces * unit->sectors * unit->sector_size;
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

/*
void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0x0080, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	return read_io8(0x0080);
}
*/

void SASI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SASI_IRQ:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] IRQ=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(ocr & OCR_INTE) {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR1, data, mask);
		}
		irq_status = ((data & mask) != 0);
		break;
		
	case SIG_SASI_DRQ:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] DRQ=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(ocr & OCR_DMAE) {
			#ifdef _PC98XA
				d_dma->write_signal(SIG_I8237_CH3, data, mask);
			#else
				d_dma->write_signal(SIG_I8237_CH0, data, mask);
			#endif
		} else {
			if(data & mask) {
				#ifdef _SCSI_DEBUG_LOG
					this->out_debug_log(_T("[SASI] DMAE=0, change IRQ\n"));
				#endif
				write_signal(SIG_SASI_IRQ, data, mask);
			}
		}
		drq_status = ((data & mask) != 0);
		break;
		
	case SIG_SASI_TC:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] TC=%d\n"), (data & mask) ? 1 : 0);
		#endif
		if(data & mask) {
			ocr &= ~OCR_DMAE;
		}
		break;
	}
}

#define STATE_VERSION	2

#include "../statesub.h"

void SASI::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT8(ocr);
	DECL_STATE_ENTRY_BOOL(irq_status);
	DECL_STATE_ENTRY_BOOL(drq_status);
	
	leave_decl_state();
}

void SASI::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(ocr);
//	state_fio->FputBool(irq_status);
//	state_fio->FputBool(drq_status);
}

bool SASI::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	ocr = state_fio->FgetUint8();
//	irq_status = state_fio->FgetBool();
//	drq_status = state_fio->FgetBool();
	return true;
}

