/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2004.09.10 -

	[ MZ-1E30 (SASI) ]
*/

#ifndef _MZ1E30_H_
#define _MZ1E30_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

class MZ1E30 : public DEVICE
{
private:
	// rom file
	uint8_t *rom_buffer;
	uint32_t rom_address, rom_size;
	
	// sasi
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
	
	struct {
		FILEIO *fio;
		bool access;
	} drive[2];
	
	void check_cmd();
	void set_status(uint8_t err);
	void set_drq(bool val);
	bool seek(int drv);
	bool flush(int drv);
	bool format(int drv);
	
public:
	MZ1E30(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ1E30() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	uint32_t read_signal(int ch);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

