/*
	Pioneer PX-7 Emulator 'ePX-7'

	Author : Takeda.Toshiya
	Date   : 2014.01.09-

	modified by umaiboux

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SLOT2_EXV	0
#define SIG_SLOT2_ACK	1
#define SIG_SLOT2_MUTE	2

#define SIG_MEMORY_SEL	0

class SLOT_MAIN : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x8000];
	uint8 ram[0x8000];
	
public:
	SLOT_MAIN(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT_MAIN() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
};

#define MAX_TRACKS 1024

class SLOT_SUB : public DEVICE
{
private:
	DEVICE *d_cpu, *d_ldp, *d_vdp;
	
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x2000];
	
	bool clock;
	bool exv, ack;
	bool super_impose;
	bool req_intr;
	bool pc4, mute_l, mute_r;
	
public:
	SLOT_SUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT_SUB() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_ldp(DEVICE* device)
	{
		d_ldp = device;
	}
	void set_context_vdp(DEVICE* device)
	{
		d_vdp = device;
	}
};

class SLOT_CART : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x10000];
	
	bool inserted;
	
public:
	SLOT_CART(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT_CART() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	
	// unique functions
	void open_cart(_TCHAR *file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
};

class MEMORY : public DEVICE
{
private:
	DEVICE *d_slot[4];
	DEVICE *d_map[4];
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int* wait);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_slot(int drv, DEVICE *device)
	{
		d_slot[drv] = device;
	}
};

#endif

