/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/msx/memory.h

	modified by umaiboux
	Date   : 2016.03.xx-

	[ memory ]
*/

#ifndef _MEMORY_EX_H_
#define _MEMORY_EX_H_

#define USE_MEGAROM

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#ifdef USE_MEGAROM
#include "sound_cart.h"
#endif

#define SIG_LDC_EXV	0
#define SIG_LDC_ACK	1
#define SIG_LDC_MUTE	2

#define SIG_MEMORY_SEL	0

#if defined(FDD_PATCH_SLOT)
class DISK;
#endif

// MAIN ROM 32K
// or MAIN ROM 32K + MAIN RAM 32K

class SLOT_MAINROM : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x8000];
#ifdef MAINROM_PLUS_RAM_32K
	uint8_t ram[0x8000];
#endif
public:
	SLOT_MAINROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Main ROM"));
	}
	~SLOT_MAINROM() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

// Cart

class SLOT_CART : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
#ifdef USE_MEGAROM
	uint8_t rom[1024*1024*4];
	int32_t type;
	SOUND_CART *d_sound;
	bool bank_scc;
#else
	uint8_t rom[0x10000];
#endif
	bool inserted;
	
public:
	SLOT_CART(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_MEGAROM
	event_register_id = -1;
#endif
		set_device_name(_T("ROM Cartridge"));
	}
	~SLOT_CART() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
#ifdef USE_MEGAROM
	void event_callback(int event_id, int err);
	int event_register_id;
#endif
	
	// unique functions
	void open_cart(const _TCHAR *file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
#ifdef USE_MEGAROM
	void set_context_sound(SOUND_CART *device)
	{
		d_sound = device;
	}
#endif
	bool load_cart(const _TCHAR *file_path/*, uint8_t *rom*/);
};

// MSXDOS2

#if defined(MSXDOS2_SLOT)
class SLOT_MSXDOS2 : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x10000];
	uint8_t mapper[2];
	
public:
	SLOT_MSXDOS2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MSX-DOS2"));
	}
	~SLOT_MSXDOS2() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};
#endif

// LD Control

#if defined(LDC_SLOT)
class SLOT_LDC : public DEVICE
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
	SLOT_LDC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("LDC Control"));
	}
	~SLOT_LDC() {}
	
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
#endif

// SUBROM

#if defined(SUBROM_SLOT)
class SLOT_SUBROM : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
#if defined(_MSX2_VARIANTS)
	uint8_t rom[0x4000];
#else
	uint8_t rom[0xc000];
#endif
	
public:
	SLOT_SUBROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Sub ROM"));
	}
	~SLOT_SUBROM() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
};
#endif

// FDD with ROM-PATCH

#if defined(FDD_PATCH_SLOT)
class SLOT_FDD_PATCH : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x4000];
	
public:
	SLOT_FDD_PATCH(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("FDD I/F"));
	}
	~SLOT_FDD_PATCH() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
};
#endif

// MAPPER RAM

#if defined(MAPPERRAM_SLOT)
class SLOT_MAPPERRAM : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t ram[MAPPERRAM_SIZE];
	uint8_t mapper[4];
	
public:
	SLOT_MAPPERRAM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Mapper RAM"));
	}
	~SLOT_MAPPERRAM() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	bool process_state(FILEIO* state_fio, bool loading);
};
#endif

// NORMAL RAM 64K

#if defined(RAM64K_SLOT)
class SLOT_RAM64K : public DEVICE
{
private:
	uint8_t ram[0x10000];
	
public:
	SLOT_RAM64K(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RAM 64KB"));
	}
	~SLOT_RAM64K() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};
#endif

// FIRMWARE 32K

#if defined(FIRMWARE32K1_SLOT)
class SLOT_FIRMWARE32K : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x8000];
	const _TCHAR* m_filename;
	
public:
	SLOT_FIRMWARE32K(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Firmware 32KB"));
	}
	~SLOT_FIRMWARE32K() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);

	// unique function
	void set_context_filename(const _TCHAR* filename)
	{
		m_filename = filename;
	}
};
#endif

// MSX-MUSIC

#if defined(MSXMUSIC_SLOT)
class SLOT_MSXMUSIC : public DEVICE
{
private:
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	uint8_t rom[0x4000];
	
public:
	SLOT_MSXMUSIC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MSX-MUSIC"));
	}
	~SLOT_MSXMUSIC() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
};
#endif

// memory bus

class MEMORY_EX : public DEVICE
{
private:
	DEVICE *d_slot[4*4];
	DEVICE *d_map[4];
#if defined(FDD_PATCH_SLOT)
	DISK* disk[MAX_DRIVE];
	DEVICE *d_fdpat;
	bool access[MAX_DRIVE];
#endif
#if defined(FDD_PATCH_SLOT) && defined(_MSX_VDP_MESS)
	DEVICE *d_vdp;
#endif
#if defined(MAPPERRAM_SLOT)
	DEVICE *d_mapper;
#endif
	bool expanded[4];
	uint8_t psl;
	uint8_t ssl[4];
	//uint32_t slot_select;
	void update_map(uint32_t val);
	
public:
	MEMORY_EX(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		ssl[0] = ssl[1] = ssl[2] = ssl[3] = 0;
		expanded[0] = expanded[1] = expanded[2] = expanded[3] = false;
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY_EX() {}
	
	// common functions
#if defined(FDD_PATCH_SLOT)
	void initialize();
	void release();
#endif
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
#if defined(FDD_PATCH_SLOT)
	uint32_t read_signal(int id);
	bool bios_ret_z80(uint16_t PC, pair32_t* af, pair32_t* bc, pair32_t* de, pair32_t* hl, pair32_t* ix, pair32_t* iy, uint8_t* iff1);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_slot_dummy(DEVICE *device)
	{
		int i;
		for(i=0; i<16; i++) d_slot[i] = device;
	}
	void set_context_slot(int drv, DEVICE *device)
	{
		d_slot[drv & 0x0f] = device;
		if (drv & 0x80) expanded[drv & 0x03] = true;
	}
#if defined(FDD_PATCH_SLOT)
	void set_context_fdd_patch(DEVICE *device)
	{
		d_fdpat = device;
	}
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);
#endif
#if defined(MAPPERRAM_SLOT)
	void set_context_mapper(DEVICE *device)
	{
		d_mapper = device;
	}
#endif
#if defined(FDD_PATCH_SLOT) && defined(_MSX_VDP_MESS)
	void set_context_vdp(DEVICE *device)
	{
		d_vdp = device;
	}
#endif
};

#endif

