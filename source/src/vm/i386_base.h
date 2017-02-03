/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#ifndef _I386_BASE_H_ 
#define _I386_BASE_H_

//#include "vm.h"
//#include "../emu.h"
#include <assert.h>
#include "device.h"

#define SIG_I386_A20	1

class VM;
class EMU;
class DEBUGGER;
class I386_BASE : public DEVICE
{
protected:
	DEVICE *d_mem, *d_io, *d_pic;

	DEVICE *d_bios;
	DEVICE *d_dma;
	void *opaque;
	
public:
	I386_BASE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_bios = NULL;
		d_dma = NULL;
		set_device_name(_T("i80x86 CPU BASE"));
	}
	~I386_BASE() {}
	
	// common functions
	virtual void initialize();
	void release();
	virtual void reset();
	virtual int run(int cycles);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int cycles);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
	
	virtual void save_state(FILEIO* state_fio);
	virtual bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
	virtual void set_context_bios(DEVICE* device) {}
	virtual void set_context_dma(DEVICE* device) {}
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
};

#endif
