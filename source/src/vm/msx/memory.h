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
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x8000];
#if defined(_PX7)
	uint8_t ram[0x8000];
#endif	
public:
	SLOT0(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Slot #0"));
	}
	~SLOT0() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

// slot #1

class SLOT1 : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x10000];
	bool inserted;
#if defined(_MSX2)
	uint8_t mapper[2];
#endif
	
public:
	SLOT1(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Slot #1"));
	}
	~SLOT1() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void open_cart(const _TCHAR *file_path);
	void close_cart();
	bool is_cart_inserted()
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
	
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x2000];
	
	bool clock;
	bool exv, ack;
	bool super_impose;
	bool req_intr;
	bool pc4, mute_l, mute_r;
	
public:
	SLOT2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Slot #2"));
	}
	~SLOT2() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
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
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x8000];
	
public:
	SLOT2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Slot #2"));
	}
	~SLOT2() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
};
#endif

// slot #3

class SLOT3 : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x10000];
	uint8_t ram[0x20000];
	bool inserted;
	uint8_t mapper[4];
	
public:
	SLOT3(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Slot #3"));
	}
	~SLOT3() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void open_cart(const _TCHAR *file_path);
	void close_cart();
	bool is_cart_inserted()
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
	bool access[MAX_DRIVE]
#endif
	uint32_t slot_select;
	void update_map(uint32_t val);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
#if !defined(_PX7)
	void initialize();
	void release();
#endif
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
#if !defined(_PX7)
	uint32_t read_signal(int id);
	bool bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_slot(int drv, DEVICE *device)
	{
		d_slot[drv] = device;
	}
#if !defined(_PX7)
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
#endif
};

#endif

