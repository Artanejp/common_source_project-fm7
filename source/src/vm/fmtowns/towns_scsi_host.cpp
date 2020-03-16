
#include "../../fifo.h"
#include "./towns_scsi_host.h"

namespace FMTOWNS
{
void TOWNS_SCSI_HOST::initialize()
{
	SCSI_HOST::initialize();
}

void TOWNS_SCSI_HOST::release()
{
}

void TOWNS_SCSI_HOST::reset()
{
	SCSI_HOST::reset();
	selected = false;
	
	write_signals(&outputs_sel, 0);
	write_signals(&outputs_req, 0);
	write_signals(&outputs_atn, 0);
	write_signals(&outputs_io, 0);
	write_signals(&outputs_cd, 0);
	write_signals(&outputs_drq, 0);
	write_signals(&outputs_bsy, 0);
	write_signals(&outputs_msg, 0);
	write_signals(&outputs_rst, 0);
//	write_signals(&outputs_dat, 0);

	write_signals(&outputs_irq, 0);
}

void TOWNS_SCSI_HOST::write_dma_io16(uint32_t addr, uint32_t data)
{
	SCSI_HOST::write_dma_io8(addr, data);
}

void TOWNS_SCSI_HOST::write_dma_io8(uint32_t addr, uint32_t data)
{
	SCSI_HOST::write_dma_io8(addr, data);
}

uint32_t TOWNS_SCSI_HOST::read_dma_io16(uint32_t addr)
{
	uint8_t val = SCSI_HOST::read_dma_io8(addr);
//	out_debug_log(_T("DMA READ8 DATA: %02X"), val);
	return val;
}

uint32_t TOWNS_SCSI_HOST::read_dma_io8(uint32_t addr)
{
//	out_debug_log(_T("READ DMA8"));
	return SCSI_HOST::read_dma_io8(addr);
}

uint32_t TOWNS_SCSI_HOST::read_signal(int ch)
{
	return SCSI_HOST::read_signal(ch);
}
	
void TOWNS_SCSI_HOST::write_signal(int id, uint32_t data, uint32_t mask)
{
#if 0
	switch(id) {
	case SIG_SCSI_REQ:
		{
			uint32_t prev_status = req_status;
			prev_status &= mask;
			req_status &= ~mask;
			req_status |= (data & mask);
			if((prev_status == 0) && ((data & mask) != 0)) {
				// L -> H
//				if(bsy_status) {
					if(!cd_status && !msg_status) {
						// data phase
						set_drq(true);
//						set_irq(false);
						access = true;
					} else if(cd_status) {
						// command/status/message phase
						set_irq(true);
					}
//				}
			} else if((prev_status != 0) && ((data & mask) == 0)) {
				// H -> L
				if(!cd_status && !msg_status) {
					set_drq(false); // Data phase
				} else if(cd_status) {
					// command/status/message phase
					set_irq(false);
//					set_drq(false);
				} else {
					//	set_drq(false); // Data phase
				}
				#ifdef SCSI_HOST_AUTO_ACK
					this->write_signal(SIG_SCSI_ACK, 0, 1);
				#endif
			}
			if(prev_status != req_status) {
				write_signals(&outputs_req, (req_status != 0) ? 0xffffffff : 0);
			}
		}
		return;
		break;
#if 0
	case SIG_SCSI_SEL:
		{
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_HOST] SEL = %d\n"), (data & mask) ? 1 : 0);
		#endif
			bool prev_selected = selected;
//			if(prev_selected = !(data & mask)) {
				selected = ((data & mask) != 0);
				write_signals(&outputs_sel, (selected) ? 0xffffffff : 0);
				if(selected) {
					data_reg = 0x08;
				}
//			}
		}
		return;
		break;
#endif
	}
#endif
	SCSI_HOST::write_signal(id, data, mask);	
}

void TOWNS_SCSI_HOST::event_callback(int event_id, int err)
{
	SCSI_HOST::event_callback(event_id, err);
}

#define STATE_VERSION	1

bool TOWNS_SCSI_HOST::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(SCSI_HOST::process_state(state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(selected);
	return true;
}
}
