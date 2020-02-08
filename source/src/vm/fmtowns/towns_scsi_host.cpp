
#include "../../fifo.h"
#include "./towns_scsi_host.h"

namespace FMTOWNS
{
#define EVENT_WRITE_QUEUE 1
#define EVENT_READ_QUEUE  2
	
void TOWNS_SCSI_HOST::initialize()
{
	SCSI_HOST::initialize();
	read_queue = new FIFO(1024);
	write_queue = new FIFO(1024);
	event_write_queue = -1;
	event_read_queue = -1;
}

void TOWNS_SCSI_HOST::release()
{
	if(read_queue != NULL) {
		read_queue->release();
		delete read_queue;
		read_queue = NULL;
	}
	if(write_queue != NULL) {
		write_queue->release();
		delete write_queue;
		write_queue = NULL;
	}
}

void TOWNS_SCSI_HOST::reset()
{
	SCSI_HOST::reset();
	selected = false;
	
	read_queue->clear();
	write_queue->clear();
	if(event_write_queue > -1) {
		cancel_event(this, event_write_queue);
	}
	event_write_queue = -1;
	if(event_read_queue > -1) {
		cancel_event(this, event_read_queue);
	}
	event_read_queue = -1;

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
#if 0
	pair32_t d;
	d.d = data;
	if(!write_queue->full()) {
		write_queue->write(d.b.h);
	}
	if(!write_queue->full()) {
		write_queue->write(d.b.l);
	}
	if(event_write_queue < 0) {
		register_event(this, EVENT_WRITE_QUEUE, 1.0, true, &event_write_queue);
	}
#else
	SCSI_HOST::write_dma_io8(addr, data);
#endif
}

void TOWNS_SCSI_HOST::write_dma_io8(uint32_t addr, uint32_t data)
{
	SCSI_HOST::write_dma_io8(addr, data);
}

uint32_t TOWNS_SCSI_HOST::read_dma_io16(uint32_t addr)
{
#if 0
	pair32_t d;
	d.d = 0;
	if(!read_queue->empty()) {
		d.b.h = read_queue->read() & 0xff;
			
	}
	if(!read_queue->empty()) {
		d.b.l = read_queue->read() & 0xff;
	}
//	if(event_read_queue < 0) {
//		register_event(this, EVENT_READ_QUEUE, 1.0, true, &event_read_queue);
//	}
	return d.d;
#else
//	out_debug_log(_T("READ DMA16"));
	return SCSI_HOST::read_dma_io8(addr);
#endif
}

uint32_t TOWNS_SCSI_HOST::read_dma_io8(uint32_t addr)
{
#if 0
	uint8_t val;
	if(!read_queue->empty()) {
		val = read_queue->read() & 0xff;
	}
//	if(event_read_queue < 0) {
//		register_event(this, EVENT_READ_QUEUE, 1.0, true, &event_read_queue);
//	}
	return val;
#else
//	out_debug_log(_T("READ DMA8"));
	return SCSI_HOST::read_dma_io8(addr);
#endif
}

uint32_t TOWNS_SCSI_HOST::read_signal(int ch)
{
	return SCSI_HOST::read_signal(ch);
}
	
void TOWNS_SCSI_HOST::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
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
//						set_irq(false);
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
			if(prev_status != req_status) {
				write_signals(&outputs_req, req_status ? 0xffffffff : 0);
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
	return SCSI_HOST::write_signal(id, data, mask);	
}

void TOWNS_SCSI_HOST::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_WRITE_QUEUE:
		if(!(write_queue->empty())) {
			uint32_t data = write_queue->read() & 0xff;
			write_signals(&outputs_dat, data);
			#ifdef SCSI_HOST_AUTO_ACK
				// set ack to clear req signal immediately
			if(bsy_status && !io_status) {
				this->write_signal(SIG_SCSI_ACK, 1, 1);
			}
			#endif
		}
		if(write_queue->empty()) {
			// Data end
			if(event_write_queue >= 0) {
				cancel_event(this, event_write_queue);
			}
			event_write_queue = -1;
		}
		break;
	default:
		break;
	}
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
	if(!read_queue->process_state(state_fio, loading)) {
 		return false;
 	}
	if(!write_queue->process_state(state_fio, loading)) {
 		return false;
 	}
	state_fio->StateValue(event_read_queue);
	state_fio->StateValue(event_write_queue);
	state_fio->StateValue(selected);
	return true;
}
}
