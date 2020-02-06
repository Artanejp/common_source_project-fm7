/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI base initiator ]
*/

#ifndef _TOWNS_SCSI_HOST_H_
#define _TOWNS_SCSI_HOST_H_

#include "../scsi_host.h"

class FIFO;
namespace FMTOWNS {
class TOWNS_SCSI_HOST : public SCSI_HOST
{
protected:
	FIFO *read_queue;
	FIFO *write_queue;
	int event_read_queue;
	int event_write_queue;
public:
	TOWNS_SCSI_HOST(VM_TEMPLATE* parent_vm, EMU* parent_emu) : SCSI_HOST(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns SCSI HOST"));
		read_queue = NULL;
		write_queue = NULL;
	}
	~TOWNS_SCSI_HOST() {}

	// common functions
	virtual void reset();
	virtual void initialize();
	virtual void release();
	virtual void event_callback(int event_id, int err);
	
	virtual void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io16(uint32_t addr);
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int ch);
	virtual bool process_state(FILEIO* state_fio, bool loading);
};
}
#endif
