/*
	ASCII MSX1 Emulator 'yaMSX1'
	ASCII MSX2 Emulator 'yaMSX2'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : Takeda.Toshiya
	Date   : 2014.01.09-

	modified by tanam
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

#if !defined(_PX7)
class DISK;
#endif

// slot #0

class SLOT0 : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x8000];
#if defined(_PX7)
	uint8 ram[0x8000];
#endif	
public:
	SLOT0(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT0() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

// slot #1

class SLOT1 : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x10000];
	bool inserted;
#if defined(_MSX2)
	uint8 mapper[2];
#endif
	
public:
	SLOT1(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT1() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void open_cart(_TCHAR *file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
};

// slot #2

#if defined(_PX7)
class SLOT2 : public DEVICE
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
	SLOT2(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT2() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
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
#else
class SLOT2 : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x8000];
	
public:
	SLOT2(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT2() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
};
#endif

// slot #3

class SLOT3 : public DEVICE
{
private:
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	uint8 rom[0x10000];
	uint8 ram[0x20000];
	bool inserted;
	uint8 mapper[4];
	
public:
	SLOT3(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SLOT3() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void open_cart(_TCHAR *file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
};

// memory bus

class MEMORY : public DEVICE
{
private:
	DEVICE *d_slot[4];
	DEVICE *d_map[4];
#if !defined(_PX7)
	DISK* disk[MAX_DRIVE];
#endif
	uint32 slot_select;
	void update_map(uint32 val);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
#if !defined(_PX7)
	void initialize();
	void release();
#endif
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int* wait);
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
#if !defined(_PX7)
	bool bios_ret_z80(uint16 PC, pair* af, pair* bc, pair* de, pair* hl, pair* ix, pair* iy, uint8* iff1);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_slot(int drv, DEVICE *device)
	{
		d_slot[drv] = device;
	}
#if !defined(_PX7)
	void open_disk(int drv, _TCHAR path[], int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
#endif
};

#endif

