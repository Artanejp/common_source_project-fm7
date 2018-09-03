/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.07 -

	[ memory ]
*/

#ifndef _TOWNS_MEMORY_H_
#define _TOWNS_MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

//#define SIG_MEMORY_DISP		0
//#define SIG_MEMORY_VSYNC	1

class I386;
// Bank size = 1GB / 1MB.
// Page 0 (0000:00000 - 0000:fffff) is another routine.
#define TOWNS_BANK_SIZE 1024
// Page0 size is 1MB / 2KB.
#define TOWNS_BANK000_BANK_SIZE 512

// C000:00000 - C1f0:fffff is 32MB / 32KB 
#define TOWNS_BANKC0x_BANK_SIZE 1024
// C200:00000 - C230:fffff is 4MB / 2KB 
#define TOWNS_BANKC2x_BANK_SIZE 2048

// MAP:
// 00000000 - 000fffff : SYSTEM RAM PAGE 0 (Similar to FMR-50).
// 00100000 - 3fffffff : EXTRA RAM (for i386 native mode.Page size is 1MB.)
// 40000000 - 7fffffff : External I/O BOX.Not accessible.
// 80000000 - bfffffff : VRAM (Reserved 01020000H bytes with earlier Towns.)
// c0000000 - c21fffff : ROM card and dictionaly/font/DOS ROMs.
// c2200000 - c2200fff : PCM RAM (Banked).
// c2201000 - fffbffff : Reserved.
// FFFC0000 - FFFFFFFF : Towns System ROM.

enum {
	TOWNS_MEMORY_TYPE_NORMAL = 0,
	TOWNS_MEMORY_TYPE_FMR_VRAM,
	TOWNS_MEMORY_TYPE_PAGE0F8,
	TOWNS_MEMORY_TYPE_EXT_MMIO,
	TOWNS_MEMORY_TYPE_LEARN_RAM,
	TOWNS_MEMORY_TYPE_WAVERAM,
	TOWNS_MEMORY_TYPE_SYSTEM_ROM,
};

// Please set from config
#define TOWNS_EXTRAM_PAGES 6

class TOWNS_VRAM;
class TOWNS_MEMORY : public DEVICE
{
protected:
	I386 *d_cpu;

	TOWNS_VRAM* d_vram;
	TOWNS_MMIO* d_mmio;           // 0x40000000 - 0x7fffffff : MMIO
	TOWNS_SPRITE* d_sprite;       // 0x81000000 - 0x8101ffff ?
	TOWNS_ROM_CARD* d_romcard[2]; // 0xc0000000 - 0xc0ffffff / 0xc1000000 - 0xc1ffffff
	TOWNS_PCM* d_pcm;             // 0xc2200000 - 0xc2200fff 

	bool bankf8_ram;
	bool bank0_dict;
	// RAM
	uint8_t ram_page0[0xc0000];       // 0x00000000 - 0x000bffff : RAM
	uint8_t ram_0f0[0x8000];      // 0x000f0000 - 0x000f7fff
	uint8_t ram_0f8[0x8000];      // 0x000f8000 - 0x000fffff : RAM/ROM

	uint8_t ram_cmos[0x2000]; // OK? Learn RAM

	uint8_t *extram; // 0x00100000 - (0x3fffffff) : Size is defined by extram_size;
	uint32_t extram_size;

	uint32_t vram_wait_val;
	uint32_t mem_wait_val;

	// ROM
	uint8_t rom_msdos[0x80000];   // 0xc2000000 - 0xc207ffff
	uint8_t rom_dict[0x80000];    // 0xc2080000 - 0xc20fffff
	uint8_t rom_font1[0x40000];   // 0xc2100000 - 0xc23f0000
	uint8_t rom_system[0x40000];  // 0xfffc0000 - 0xffffffff
#if 0
	uint8_t rom_font20[0x80000];
#endif
	// misc
	uint8_t machine_id;
	uint32_t dicrom_bank;
	uint32_t vram_size; // Normally 512KB.

	uint8_t* read_bank_adrs_cx[0x100000]; // Per 4KB.
	uint8_t* write_bank_adrs_cx[0x100000]; // Per 4KB.
	DEVICE*   device_bank_adrs_cx[0x100000]; // Per 4KB.
	uint32_t type_bank_adrs_cx[0x100000]; // Per 4KB.
public:
	TOWNS_MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("FMTOWNS_MEMORY"));
		d_cpu = NULL;
		d_vram = NULL;
		d_mmio = NULL;
		d_pcm = NULL;
		d_sprite = NULL;
		d_romcard[0] = d_romcard[1] = NULL;
		machine_id = 0;
	}
	~TOWNS_MEMORY() {}
	
	// common functions
	void initialize();
	void reset();

	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	// Using [read|write]_data[16|32] to be faster memory access.
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	// With Wait
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t write_data8w(uint32_t addr, int* wait);
	void write_data16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t write_data16w(uint32_t addr, int* wait);
	void write_data32w(uint32_t addr, uint32_t data, int* wait);
	uint32_t write_data32w(uint32_t addr, int* wait);
	
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	// Using [read|write]_dma_data16 for DMAC 16bit mode (SCSI/CDROM?).
	void write_dma_data16(uint32_t addr, uint32_t data);
	uint32_t read_dma_data16(uint32_t addr);
	
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(I386* device)
	{
		d_cpu = device;
	}
	void set_machine_id(uint8_t id)
	{
		machine_id = id;
	}
	void set_context_vram(TOWNS_VRAM* device)
	{
		d_vram = device;
	}
	void set_context_mmio(DEVICE* device)
	{
		d_mmio = device;
	}
	void set_context_sprite(DEVICE* device)
	{
		d_sprite = device;
	}
	void set_context_romcard(DEVICE* device, int num)
	{
		d_romcard[num & 1] = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	
};

#endif

