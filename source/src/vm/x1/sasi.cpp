/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2018.05.15-

	[ SASI I/F ]
*/

#include "sasi.h"
#ifdef _X1TURBO_FEATURE
#include "../z80dma.h"
#endif

#define STATUS_INT	0x00	// unknown
#define STATUS_REQ	0x01
#define STATUS_BSY	0x02
#define STATUS_IXO	0x04
#define STATUS_CXD	0x08
#define STATUS_MSG	0x10

void SASI::reset()
{
	irq_status = drq_status = false;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xfd0:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_dma_io8(addr, data);
		break;
	case 0xfd1:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_signal(SIG_SCSI_SEL, 0, 1);
		break;
	case 0xfd2:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_signal(SIG_SCSI_RST, 0, 1);
		d_host->write_signal(SIG_SCSI_RST, 1, 1);
		d_host->write_signal(SIG_SCSI_RST, 0, 1);
		break;
	case 0xfd3:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_dma_io8(addr, data);
		d_host->write_signal(SIG_SCSI_SEL, 1, 1);
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	uint32_t val = 0;
	
	switch(addr) {
	case 0xfd0:
		val = d_host->read_dma_io8(addr);
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI] in  %04X %02X\n"), addr, val);
		#endif
		return val;
	case 0xfd1:
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
		return val;
	}
	return 0xff;
}

void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0xfd0, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	return read_io8(0xfd0);
}

void SASI::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SASI_IRQ:
#ifdef _X1TURBO_FEATURE
		d_dma->write_signal(SIG_Z80DMA_READY, data, mask);
#endif
		irq_status = ((data & mask) != 0);
		break;
	case SIG_SASI_DRQ:
#ifdef _X1TURBO_FEATURE
		d_dma->write_signal(SIG_Z80DMA_READY, data, mask);
#endif
		drq_status = ((data & mask) != 0);
		break;
	}
}

#define STATE_VERSION	1

#include "../statesub.h"

void SASI::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
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
//	irq_status = state_fio->FgetBool();
//	drq_status = state_fio->FgetBool();
	return true;
}

