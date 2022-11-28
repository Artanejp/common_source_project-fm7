/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2018.05.25 -

	[ SASI I/F ]
*/

#ifndef _SASI_H_
#define _SASI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SASI_BSY	0
#define SIG_SASI_CXD	1
#define SIG_SASI_IXO	2
#define SIG_SASI_MSG	3
#define SIG_SASI_REQ	4
#define SIG_SASI_ACK	5
#define SIG_SASI_TC	6

class SASI : public DEVICE
{
private:
	DEVICE *d_host;
	DEVICE *d_dma, *d_pic;
	
	// sasi
	uint8_t control, prev_control;
	bool bsy_status, prev_bsy_status;
	bool cxd_status, prev_cxd_status;
	bool ixo_status, prev_ixo_status;
	bool msg_status, prev_msg_status;
	bool req_status, prev_req_status;
	bool ack_status, prev_ack_status;
	bool irq_status, drq_status;
	
	void update_signal();
	
public:
	SASI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		control = 0x00;
		bsy_status = prev_bsy_status = false;
		cxd_status = prev_cxd_status = false;
		ixo_status = prev_ixo_status = false;
		msg_status = prev_msg_status = false;
		req_status = prev_req_status = false;
		ack_status = prev_ack_status = false;
		irq_status = drq_status = false;
		set_device_name(_T("SASI I/F"));
	}
	~SASI() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_host(DEVICE* device)
	{
		d_host = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

