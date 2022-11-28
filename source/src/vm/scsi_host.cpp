/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base initiator ]
*/

#include "scsi_host.h"

void SCSI_HOST::reset()
{
	data_reg = 0;
	bsy_status = cd_status = io_status = msg_status = req_status = ack_status = 0;
	access = false;
	
	set_irq(false);
	set_drq(false);
}

#ifdef SCSI_HOST_WIDE
void SCSI_HOST::write_dma_io16(uint32_t addr, uint32_t data)
#else
void SCSI_HOST::write_dma_io8(uint32_t addr, uint32_t data)
#endif
{
	#ifdef _SCSI_DEBUG_LOG
		this->force_out_debug_log(_T("[SCSI_HOST] Write %02X\n"), data);
	#endif
	write_signals(&outputs_dat, data);
	
	#ifdef SCSI_HOST_AUTO_ACK
		// set ack to clear req signal immediately
		if(bsy_status && !io_status) {
			this->write_signal(SIG_SCSI_ACK, 1, 1);
		}
	#endif
}

#ifdef SCSI_HOST_WIDE
uint32_t SCSI_HOST::read_dma_io16(uint32_t addr)
#else
uint32_t SCSI_HOST::read_dma_io8(uint32_t addr)
#endif
{
	uint32_t value = data_reg;
	#ifdef _SCSI_DEBUG_LOG
		this->force_out_debug_log(_T("[SCSI_HOST] Read %02X\n"), value);
	#endif
	#ifdef SCSI_HOST_AUTO_ACK
		// set ack to clear req signal immediately
		if(bsy_status && io_status) {
			this->write_signal(SIG_SCSI_ACK, 1, 1);
		}
	#endif
	return value;
}

void SCSI_HOST::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	// from initiator
	case SIG_SCSI_SEL:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_HOST] SEL = %d\n"), (data & mask) ? 1 : 0);
		#endif
		write_signals(&outputs_sel, (data & mask) ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_ATN:
		#ifdef _SCSI_DEBUG_LOG
//			this->out_debug_log(_T("[SCSI_HOST] ATN = %d\n"), (data & mask) ? 1 : 0);
		#endif
		write_signals(&outputs_atn, (data & mask) ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_ACK:
		#ifdef _SCSI_DEBUG_LOG
//			this->out_debug_log(_T("[SCSI_HOST] ACK = %d\n"), (data & mask) ? 1 : 0);
		#endif
		write_signals(&outputs_ack, (data & mask) ? 0xffffffff : 0);
		ack_status = data & mask;
		break;
		
	case SIG_SCSI_RST:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_HOST] RST = %d\n"), (data & mask) ? 1 : 0);
		#endif
		write_signals(&outputs_rst, (data & mask) ? 0xffffffff : 0);
		break;
		
	// from target
	case SIG_SCSI_DAT:
		data_reg &= ~mask;
		data_reg |= (data & mask);
		break;
		
	case SIG_SCSI_BSY:
		bsy_status &= ~mask;
		bsy_status |= (data & mask);
		write_signals(&outputs_bsy, bsy_status ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_CD:
		cd_status &= ~mask;
		cd_status |= (data & mask);
		write_signals(&outputs_cd, cd_status ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_IO:
		io_status &= ~mask;
		io_status |= (data & mask);
		write_signals(&outputs_io, io_status ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_MSG:
		msg_status &= ~mask;
		msg_status |= (data & mask);
		write_signals(&outputs_msg, msg_status ? 0xffffffff : 0);
		break;
		
	case SIG_SCSI_REQ:
		{
			uint32_t prev_status = req_status;
			req_status &= ~mask;
			req_status |= (data & mask);
			
			if(!prev_status && req_status) {
				// L -> H
//				if(bsy_status) {
					if(!cd_status && !msg_status) {
						// data phase
						set_drq(true);
						access = true;
					} else if(cd_status) {
						// command/status/message phase
						set_irq(true);
					}
//				}
			} else if(prev_status && !req_status) {
				// H -> L
				set_drq(false);
				set_irq(false);
				#ifdef SCSI_HOST_AUTO_ACK
					this->write_signal(SIG_SCSI_ACK, 0, 0);
				#endif
			}
			write_signals(&outputs_req, req_status ? 0xffffffff : 0);
		}
		break;
	}
}

uint32_t SCSI_HOST::read_signal(int id)
{
	// SCSI signals
	switch (id) {
	case SIG_SCSI_BSY:
		return bsy_status ? 0xffffffff : 0;
		
	case SIG_SCSI_CD:
		return cd_status  ? 0xffffffff : 0;
		
	case SIG_SCSI_IO:
		return io_status  ? 0xffffffff : 0;
		
	case SIG_SCSI_MSG:
		return msg_status ? 0xffffffff : 0;
		
	case SIG_SCSI_REQ:
		return req_status ? 0xffffffff : 0;
		
	case SIG_SCSI_ACK:
		return ack_status ? 0xffffffff : 0;
	}
	
	// access lamp
	uint32_t value = access ? 0xffffffff : 0;
	access = false;
	return value;
}

void SCSI_HOST::set_irq(bool value)
{
	write_signals(&outputs_irq, value ? 0xffffffff : 0);
}

void SCSI_HOST::set_drq(bool value)
{
	write_signals(&outputs_drq, value ? 0xffffffff : 0);
}

#define STATE_VERSION	2

bool SCSI_HOST::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(data_reg);
	state_fio->StateValue(bsy_status);
	state_fio->StateValue(cd_status);
	state_fio->StateValue(io_status);
	state_fio->StateValue(msg_status);
	state_fio->StateValue(req_status);
	state_fio->StateValue(ack_status);
	return true;
}

