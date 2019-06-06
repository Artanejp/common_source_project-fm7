/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.06.06 -

	[ MIDI REDIRECTOR ]

	This is redirection from some outputs to osd depended midi port.
*/
#include "fifo.h"
#include "fileio.h"
#include "vm_template.h"
#include "osd.h"
#include "midi_redirector.h"

void MIDI_REDIRECTOR::initialize()
{
	if(send_queue_size > 0) {
		send_queue = new FIFO(send_queue_size);
	}
	if(recv_queue_size > 0) {
		recv_queue = new FIFO(recv_queue_size);
	}
	
}

void MIDI_REDIRECTOR::release()
{
	if(send_queue != NULL) {
		send_queue->release();
		delete send_queue;
		send_queue = NULL;
	}
	if(recv_queue != NULL) {
		recv_queue->release();
		delete recv_queue;
		recv_queue = NULL;
	}
}

void MIDI_REDIRECTOR::reset()
{
	// Reset queue must do via write_signal().
}

uint32_t MIDI_REDIRECTOR::read_signal(int id)
{
	int size;
	switch(id) {
	case SIG_MIDI_REMAIN_SEND:
		if(send_queue_size <= 0) return 0;
		if(send_queue != NULL) {
			size = send_queue->count();
			if(size < 0) size = 0;
			size = send_queue_size - size;
			if(size < 0) size = 0;
			return (uint32_t)size;
		}
		break;
	case SIG_MIDI_REMAIN_RECEIVE:
		if(recv_queue_size <= 0) return 0;
		if(recv_queue != NULL) {
			size = recv_queue->count();
			if(size < 0) size = 0;
			size = send_queue_size - size;
			if(size < 0) size = 0;
			return (uint32_t)size;
		}
		break;
	case SIG_MIDI_ENABLE_TO_SEND:
		return (send_enable) ? 0xffffffff : 0;
		break;
	case SIG_MIDI_ENABLE_TO_RECEIVE:
		return (recv_enable) ? 0xffffffff : 0;
		break;
	default:
		return 0;
	}
}
void MIDI_REDIRECTOR::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MIDI_RESET:
		if(send_queue != NULL) {
			send_queue->clear();
		}
		if(recv_queue != NULL) {
			recv_queue->clear();
		}
		// ToDo: Reset event to both DEVICE and OSD.
		break;
	case SIG_MIDI_ENABLE_TO_SEND:
		send_enable = ((data & mask) != 0) ? true : false;
		break;
	case SIG_MIDI_ENABLE_TO_RECEIVE:
		recv_enable = ((data & mask) != 0) ? true : false;
		break;
	case SIG_MIDI_SEND:
		if(send_enable) {
			if(send_queue != NULL) {
				bool stat = true;
				while(!(send_queue->empty())) {
					int d = send_queue->read_not_remove(1);
					stat = osd->push_midi_data(send_port_num, d & mask);
					if(!(stat)) {
						break; //
					}
					d = send_queue->read(); // Dummy read
				}
				if(!stat) { // Pushing failed before data write
					if(!(send_queue->full())) {
						send_queue->write(data & mask);
						write_signals(&outputs_send_ack, 0xffffffff);
					} else { // FIFO FULL
						write_signals(&outputs_send_error, 0xffffffff);
					}
				} else { // Before pushing all queued data is completed
					stat = osd->push_midi_data(send_port_num, data & mask);
					if(!(stat)) {
						if(!(send_queue->full())) {
							send_queue->write(data & mask);
							write_signals(&outputs_send_ack, 0xffffffff);
						} else { // Queue full
							write_signals(&outputs_send_error, 0xffffffff);
						}							
					} else { // OK
						write_signals(&outputs_send_ack, 0xffffffff);
					}
				}				
				if(send_queue->empty()) {
					write_signals(&outputs_send_empty, 0xffffffff);
				}
				if(send_queue->full()) {
					write_signals(&outputs_send_full, 0xffffffff);
				}
			} else {
				if(!(osd->push_midi_data(send_port_num, data & mask))) {
					write_signals(&outputs_send_empty, 0xffffffff);
				} else {
					write_signals(&outputs_send_ack, 0xffffffff);
				}
			}
		}
		break;
	default:
		break;
	}
}

bool MIDI_REDIRECTOR::push_receive_data(int port_num, uint32_t data)
{
	if((d_receiver == NULL) || (receiver_id < 0)) {
		return false; // redirection failed.
	}
	if(port_num != recv_port_num) {
		return false; // port number don't match.
	}
	if(!recv_enable) {
		return false; // Do not accept to receive.
	}
	if(recv_queue == NULL) {
		d_receiver->write_signal(receiver_id, data, receiver_mask); // Queue don't exists
		write_signals(&outputs_receive_event, 0xffffffff);
		return true;
	}
	if(recv_queue->empty()) {
		d_receiver->write_signal(receiver_id, data, receiver_mask); // Write immediatry
		write_signals(&outputs_receive_event, 0xffffffff);
		return true;
	}
	while(!(recv_queue->empty())) {
		d_receiver->write_signal(receiver_id, (uint32_t)(recv_queue->read()), receiver_mask); // Queue don't exists
	}
	d_receiver->write_signal(receiver_id, data, receiver_mask); // Write immediatry
	write_signals(&outputs_receive_event, 0xffffffff);
	return true;
}


int MIDI_REDIRECTOR::bind_receive_port(void)
{
	if(recv_port_num >= 0) return recv_port_num; // Already bind to OSD.
	int n;
	n = osd->bind_midi_receiver_port(this);
	if(n < 0) { // Failed to bind
		recv_port_num = -1;
		return -1;
	}
	recv_port_num = n;
	return n;
}

int MIDI_REDIRECTOR::bind_send_port(void)
{
	if(send_port_num >= 0) return send_port_num; // Already bind to OSD.
	int n;
	n = osd->bind_midi_send_to(this);
	if(n < 0) { // Failed to bind
		send_port_num = -1;
		return -1;
	}
	send_port_num = n;
	return n;
}

#define STATE_VERSION	1

bool MIDI_REDIRECTOR::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!send_queue->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	if(!recv_queue->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateValue(receiver_id);
	state_fio->StateValue(receiver_mask);
	state_fio->StateValue(send_port_num);
	state_fio->StateValue(recv_port_num);
	state_fio->StateValue(send_queue_size);
	state_fio->StateValue(recv_queue_size);
	state_fio->StateValue(send_enable);
	state_fio->StateValue(recv_enable);
	
 	// post process
	if(loading) {
	}
	return true;
}
