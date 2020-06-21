/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.06.06 -

	[ MIDI REDIRECTOR ]

	This is redirection from some outputs to osd depended midi port.
*/

#ifndef _MIDI_REDIRECTOR_H_
#define _MIDI_REDIRECTOR_H_

#include "device.h"

#define SIG_MIDI_RESET						1
#define SIG_MIDI_SEND						2
#define SIG_MIDI_ENABLE_TO_SEND				3
#define SIG_MIDI_ENABLE_TO_RECEIVE			4
#define SIG_MIDI_REMAIN_SEND				5
#define SIG_MIDI_REMAIN_RECEIVE				6

/*
  write_signal(SIG_MIDI_RESET, foo, mask):
	Reset data queue.
  write_signal(SIG_MIDI_ENABLE_TO_SEND, num, mask):
	Start/Stop sending (redirecting) data.If ((num & mask) != 0) starts, another stops.
  write_signal(SIG_MIDI_ENABLE_TO_RECEIVE, num, mask):
	Start/Stop receiving (redirecting) data.If ((num & mask) != 0) starts, another stops.
  write_signal(SIG_MIDI_SEND, data, mask):
	Send data to (virtual) midi devices via osd.Data must be BYTE.Not WORD or DWORD.
  read_signal(SIG_MIDI_ENABLE_TO_SEND):
	Query enabled to send any data to OSD.
  read_signal(SIG_MIDI_ENABLE_TO_SEND):
	Query enabled to receive any data to DEVICE.
  read_signal(SIG_MIDI_REMAIN_SEND):
	Query send buffer remains.
  read_signal(SIG_MIDI_REMAIN_RECEIVE):
	Query receive buffer remains.

  set_context_receiver(dev, id, mask):
	Set receiver (within some DEVICEs) port.This DON'T allow multiple device.
  set_context_empty_event(dev, id, mask):
	Set event sending queue is empty.This allow multiple device.
  set_context_full_event(dev, id, mask):
	Set event sending queue is full.This allow multiple device.
  unset_context_receiver(dev, id):
	Unset receiver (within some DEVICEs) port.This DON'T allow multiple device.
*/
class VM_TEMPLATE;
class EMU;
class FIFO;
class MIDI_REDIRECTOR : public DEVICE {
protected:
	FIFO* send_queue;
	FIFO* recv_queue;
	DEVICE	*d_receiver;
	int		 receiver_id;
	int send_port_num;
	int recv_port_num;
	int send_queue_size;
	int recv_queue_size;
	bool send_enable;
	bool recv_enable;
	
	uint32_t receiver_mask;
	outputs_t outputs_send_full;
	outputs_t outputs_send_empty;
	outputs_t outputs_send_ack;
	outputs_t outputs_send_error;
	outputs_t outputs_receive_event;

public:
	MIDI_REDIRECTOR(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_receiver = NULL;
		send_queue = NULL;
		recv_queue = NULL;
		receiver_id = -1;
		receiver_mask = 0xff;
		initialize_output_signals(&outputs_send_full);
		initialize_output_signals(&outputs_send_empty);
		initialize_output_signals(&outputs_send_ack);
		initialize_output_signals(&outputs_send_error);
		initialize_output_signals(&outputs_receive_event);
		send_port_num = -1;
		recv_port_num = -1;
		send_queue_size = 1024; // Maybe will change.
		recv_queue_size = 1024; // Maybe will change.
		send_enable = true;
		recv_enable = true;
		set_device_name(_T("MIDI REDIRECTOR"));
	}
	~MIDI_REDIRECTOR() {}

	virtual void reset();
	virtual void initialize();
	virtual void release();
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual bool process_state(FILEIO* state_fio, bool loading);

	// Unique functions
	// API for OSD.
	virtual bool __FASTCALL push_receive_data(int port_num, uint32_t data); // Push receive data by OSD.
	virtual int __FASTCALL bind_receive_port(void); // Request bind port from OSD to DEVICE or VM.
	virtual int __FASTCALL bind_send_port(void);    // Request bind port from DEVICE or VM to OSD.
	// *Don't have to use only* before initialize().Sometimes enable to change.
	void set_context_receiver(DEVICE* dev, int id, uint32_t mask = 0xff)
	{
		d_receiver = dev;
		receiver_id = id;
		receiver_mask = mask;
	}
	void unset_context_receiver(DEVICE* dev, int id)
	{
		if((dev == d_receiver) && (id == receiver_id)) {
			d_receiver = NULL;
			receiver_id = -1;
		}
	}
	// Below finctions *must* call before initialize().
	void set_send_queue_size(int size)
	{
		if(size > 0) send_queue_size = size;
	}
	void set_receive_queue_size(int size)
	{
		if(size > 0) send_queue_size = size;
	}
	void set_context_send_empty(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_send_empty, dev, id, mask);
	}
	void set_context_send_full(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_send_full, dev, id, mask);
	}
	void set_context_send_ack(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_send_ack, dev, id, mask);
	}
	void set_context_send_error(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_send_error, dev, id, mask);
	}
	void set_context_receive_event(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_receive_event, dev, id, mask);
	}
};

#endif /* _MIDI_REDIRECTOR_H_ */
