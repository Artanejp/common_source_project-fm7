/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.19-

	[ sasi hdd ]
*/

#ifndef _SASI_H_
#define _SASI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

class SASI : public DEVICE
{
private:
	DEVICE *d_pic;
	
	void check_cmd();
	void set_status(uint8_t err);
	void set_drq(bool val);
	bool seek(int drv);
	bool flush(int drv);
	bool format(int drv);
	
	uint8_t buffer[256];
	int phase;
	int sector;
	int blocks;
	uint8_t cmd[6];
	int cmd_ptr;
	int unit;
	int buffer_ptr;
	uint8_t status;
	uint8_t status_irq_drq;
	uint8_t error;
	uint8_t status_buf[4];
	int status_ptr;
	uint8_t datareg;
	uint8_t maskreg;
	
	typedef struct {
		FILEIO *fio;
		bool access;
	} drive_t;
	drive_t drive[2];
	
public:
	SASI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI I/F"));
	}
	~SASI() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

