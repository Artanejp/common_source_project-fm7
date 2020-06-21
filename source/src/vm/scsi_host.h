/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base initiator ]
*/

#ifndef _SCSI_HOST_H_
#define _SCSI_HOST_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
//class EMU;
//class VM;
class FIFO;

#define SIG_SCSI_HOST_DMAE 1
class SCSI_HOST : public DEVICE
{
protected: // Make pcotected because TOWNS's DMAC may transfer 16bit around SCSI.
	outputs_t outputs_irq;	// to adaptor
	outputs_t outputs_drq;
	
	outputs_t outputs_bsy;
	outputs_t outputs_cd;
	outputs_t outputs_io;
	outputs_t outputs_msg;
	outputs_t outputs_req;
	
	outputs_t outputs_dat;	// to devices
	outputs_t outputs_sel;
	outputs_t outputs_atn;
	outputs_t outputs_ack;
	outputs_t outputs_rst;
	FIFO* data_queue;
	
	uint32_t data_reg;
	uint32_t bsy_status, cd_status, io_status, msg_status, req_status, ack_status;
	bool access;
	bool is_16bit;
	bool is_dma;
	
	virtual void __FASTCALL set_irq(bool value);
	virtual void __FASTCALL set_drq(bool value);
	
public:
	SCSI_HOST(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_drq);
		
		initialize_output_signals(&outputs_bsy);
		initialize_output_signals(&outputs_cd);
		initialize_output_signals(&outputs_io);
		initialize_output_signals(&outputs_msg);
		initialize_output_signals(&outputs_req);
		
		initialize_output_signals(&outputs_dat);
		initialize_output_signals(&outputs_sel);
		initialize_output_signals(&outputs_atn);
		initialize_output_signals(&outputs_ack);
		initialize_output_signals(&outputs_rst);

		data_queue = NULL;
		set_device_name(_T("SCSI HOST"));
	}
	~SCSI_HOST() {}
	
	// common functions
	virtual void reset();
	virtual void initialize();
	virtual void release();
	virtual void event_callback(int id, int err);
/*
#ifdef SCSI_HOST_WIDE
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
#else
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
#endif
*/
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
	
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_bsy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_bsy, device, id, mask);
	}
	void set_context_cd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_cd, device, id, mask);
	}
	void set_context_io(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_io, device, id, mask);
	}
	void set_context_msg(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_msg, device, id, mask);
	}
	void set_context_req(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_req, device, id, mask);
	}
	void set_context_ack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ack, device, id, mask);
	}
	void set_context_target(DEVICE* device)
	{
#ifdef SCSI_HOST_WIDE
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xffff);
#else
		register_output_signal(&outputs_dat, device, SIG_SCSI_DAT, 0xff);
#endif
		register_output_signal(&outputs_sel, device, SIG_SCSI_SEL, 1);
		register_output_signal(&outputs_atn, device, SIG_SCSI_ATN, 1);
		register_output_signal(&outputs_ack, device, SIG_SCSI_ACK, 1);
		register_output_signal(&outputs_rst, device, SIG_SCSI_RST, 1);
	}
};

#endif

