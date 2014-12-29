/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ 8bit i/o bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_MODE	0

#ifndef IO_ADDR_MAX
#define IO_ADDR_MAX 0x10000
#endif
#define IO_ADDR_MASK (IO_ADDR_MAX - 1)

class IO : public DEVICE
{
private:
	DEVICE *d_cpu;
	
	// i/o map
	struct {
		DEVICE* dev;
		uint32 addr;
		bool is_flipflop;
	} wr_table[IO_ADDR_MAX];
	
	struct {
		DEVICE* dev;
		uint32 addr;
		bool value_registered;
		uint32 value;
	} rd_table[IO_ADDR_MAX];
	
	// vram
#ifdef _X1TURBO_FEATURE
	uint8 vram[0x18000];
#else
	uint8 vram[0xc000];
#endif
	bool vram_mode, signal;
	
	uint8* vram_b;
	uint8* vram_r;
	uint8* vram_g;
	
	uint8 vdisp;
	
	uint32 prev_clock, vram_wait_index;
	bool column40;
#ifdef _X1TURBO_FEATURE
	uint8 crtc_regs[18];
	int crtc_ch;
	bool hireso;
#endif
	void write_port8(uint32 addr, uint32 data, bool is_dma, int* wait);
	uint32 read_port8(uint32 addr, bool is_dma, int* wait);
	int get_vram_wait();
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(wr_table, 0, sizeof(wr_table));
		memset(rd_table, 0, sizeof(rd_table));
		
		// vm->dummy must be generated first !
		for(int i = 0; i < IO_ADDR_MAX; i++) {
			wr_table[i].dev = rd_table[i].dev = vm->dummy;
			wr_table[i].addr = rd_table[i].addr = i;
		}
	}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void write_io8w(uint32 addr, uint32 data, int* wait);
	uint32 read_io8w(uint32 addr, int* wait);
	void write_dma_io8w(uint32 addr, uint32 data, int* wait);
	uint32 read_dma_io8w(uint32 addr, int* wait);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8* get_vram()
	{
		return vram;
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
	
	void set_iovalue_single_r(uint32 addr, uint32 value);
	void set_iovalue_range_r(uint32 s, uint32 e, uint32 value);
	void set_flipflop_single_rw(uint32 addr, uint32 value);
	void set_flipflop_range_rw(uint32 s, uint32 e, uint32 value);
};

#endif

