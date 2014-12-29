/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ 8bit i/o bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_MIO	0

#ifndef IO_ADDR_MAX
#define IO_ADDR_MAX 0x100
#endif
#define IO_ADDR_MASK (IO_ADDR_MAX - 1)

class IO : public DEVICE
{
private:
	// i/o map
	typedef struct {
		DEVICE* dev;
		uint32 addr;
	} iomap_t;
	
	iomap_t write_table[IO_ADDR_MAX];
	iomap_t read_table[IO_ADDR_MAX];
	
	// i/o mapped memory
	uint8* ram;
	bool mio;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// vm->dummy must be generated first !
		for(int i = 0; i < IO_ADDR_MAX; i++) {
			write_table[i].dev = read_table[i].dev = vm->dummy;
			write_table[i].addr = read_table[i].addr = i;
		}
	}
	~IO() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_ram_ptr(uint8* ptr)
	{
		ram = ptr;
	}
	void set_iomap_single_r(uint32 addr, DEVICE* device);
	void set_iomap_single_w(uint32 addr, DEVICE* device);
	void set_iomap_single_rw(uint32 addr, DEVICE* device);
	void set_iomap_alias_r(uint32 addr, DEVICE* device, uint32 alias);
	void set_iomap_alias_w(uint32 addr, DEVICE* device, uint32 alias);
	void set_iomap_alias_rw(uint32 addr, DEVICE* device, uint32 alias);
	void set_iomap_range_r(uint32 s, uint32 e, DEVICE* device);
	void set_iomap_range_w(uint32 s, uint32 e, DEVICE* device);
	void set_iomap_range_rw(uint32 s, uint32 e, DEVICE* device);
};

#endif

