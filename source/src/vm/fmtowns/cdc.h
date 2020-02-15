/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[FM-Towns CD Controller]
*/
#pragma once

#include "../../common.h"
#include "../device.h"

#define SIG_TOWNS_CDC_DRQ 1
#define SIG_TOWNS_CDC_IRQ 2
#define SIG_TOWNS_CDC_BSY 3
#define SIG_TOWNS_CDC_CD  4
#define SIG_TOWNS_CDC_IO  5
#define SIG_TOWNS_CDC_MSG 6
#define SIG_TOWNS_CDC_REQ 7
#define SIG_TOWNS_CDC_ACK 8
#define SIG_TOWNS_CDC_CDROM_DONE 9

class SCSI_HOST;
class FIFO;

namespace FMTOWNS {
	class TOWNS_CDROM;
}

namespace FMTOWNS {
class CDC : public DEVICE {
protected:
	outputs_t output_dma_line;
	outputs_t output_dma_intr;
	outputs_t output_submpu_intr;

	DEVICE*    d_dmac;
	SCSI_HOST* d_scsi_host;
	TOWNS_CDROM* d_cdrom;
	
	uint8_t param_queue[8];
	int param_ptr;
	FIFO* stat_fifo;

	int readptr;
	bool has_status;
	int extra_status;
	bool submpu_ready;
	bool software_transfer_phase;
	bool dma_transfer;
	bool pio_transfer;

	bool dma_intr;
	bool submpu_intr;
	bool dma_intr_mask;
	bool submpu_intr_mask;
	
	bool busy_status;
	bool cd_status;
	bool io_status;
	bool msg_status;
	bool req_status;
	bool ack_status;

	uint8_t w_regs[16];
	bool command_type_play;
	bool stat_reply_intr;
	
	virtual void read_cdrom(bool req_reply);
	virtual void stop_cdda(bool req_reply);
	virtual void stop_cdda2(bool req_reply);
	virtual void unpause_cdda(bool rea_reply);
	virtual void play_cdda(bool req_reply);
	virtual void write_status(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	virtual void enqueue_command_play(uint8_t cmd);
	virtual void enqueue_command_status(uint8_t cmd);

public:
	CDC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&output_dma_line);
		initialize_output_signals(&output_dma_intr);
		initialize_output_signals(&output_submpu_intr);
		
		set_device_name(_T("FM-Towns CD-ROM controller"));
	}
	~CDC() { }

	virtual void initialize();
	virtual void release();
	virtual void reset();

	virtual void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);

	virtual void __FASTCALL write_io8(uint32_t address, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t address);

	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);
	
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	virtual void set_context_scsi_host(SCSI_HOST* dev);
	virtual void set_context_cdrom(TOWNS_CDROM* dev);

	void set_context_dmac(DEVICE *dev)
	{
		d_dmac = dev;
	}
	void set_context_dmareq_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_dma_line, dev, id, mask);
	}
	
	void set_context_dmaint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_dma_intr, dev, id, mask);
	}
	
	void set_context_mpuint_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_submpu_intr, dev, id, mask);
	}
};

}
