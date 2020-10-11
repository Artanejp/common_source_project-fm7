/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.06.06 -

	[ SERIAL I/O REDIRECTOR ]

	This is redirection from some outputs to osd depended midi port.
*/

#ifndef _SIO_REDIRECTOR_H_
#define _SIO_REDIRECTOR_H_

#include "midi_redirector.h"

#define SIG_SIO_RESET						1
#define SIG_SIO_SEND						2
#define SIG_SIO_ENABLE_TO_SEND				3
#define SIG_SIO_ENABLE_TO_RECEIVE			4
#define SIG_SIO_REMAIN_SEND					5
#define SIG_SIO_REMAIN_RECEIVE				6

class  DLL_PREFIX SIO_REDIRECTOR : public MIDI_REDIRECTOR {
public:
	SIO_REDIRECTOR(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MIDI_REDIRECTOR(parent_vm, parent_emu)
	{
		set_device_name(_T("SERIAL I/O REDIRECTOR"));
	}
	~SIO_REDIRECTOR() {}
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
};

#endif /* _SIO_REDIRECTOR_H_ */
