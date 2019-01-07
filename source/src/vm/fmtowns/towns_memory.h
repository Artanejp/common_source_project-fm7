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
	TOWNS_MEMORY_NORMAL = 0,
	TOWNS_MEMORY_FMR_VRAM,
	TOWNS_MEMORY_FMR_TEXT,
	TOWNS_MEMORY_FMR_VRAM_RESERVE,
	TOWNS_MEMORY_SPRITE_ANKCG1,
	TOWNS_MEMORY_ANKCG2,
	
	TOWNS_MEMORY_MMIO_0CC,
	TOWNS_MEMORY_DICT_0D0,
	TOWNS_MEMORY_CMOS_0D8,
	TOWNS_MEMORY_PAGE0F8,
	
	TOWNS_MEMORY_TYPE_EXT_MMIO,
	TOWNS_MEMORY_TYPE_LEARN_RAM,
	TOWNS_MEMORY_TYPE_WAVERAM,
	TOWNS_MEMORY_TYPE_SYSTEM_ROM,
};

// Please set from config
#define TOWNS_EXTRAM_PAGES 6

class I386;
class BEEP;
class TOWNS_VRAM;
class TOWNS_SPRITE;
class TOWNS_ROM_CARD;
class TOWNS_CMOS;
class TOWNS_PCM;
class TOWNS_MEMORY : public DEVICE
{
protected:
	I386 *d_cpu;

	TOWNS_VRAM* d_vram;
	TOWNS_SPRITE* d_sprite;       // 0x81000000 - 0x8101ffff ?
	TOWNS_ROM_CARD* d_romcard[2]; // 0xc0000000 - 0xc0ffffff / 0xc1000000 - 0xc1ffffff
	TOWNS_CMOS* d_cmos;             // 0xc2140000 - 0xc2141fff 
	TOWNS_PCM* d_pcm;             // 0xc2200000 - 0xc2200fff 
	BEEP* d_beep;
	
	bool bankc0_vram;
	bool bankf8_ram;
	bool bankd0_dict;
	bool ankcg_enabled;
	uint8_t dict_bank;

	uint16_t machine_id;
	uint8_t cpu_id;

	// ToDo: around DMA
	uint32_t dma_addr_mask;
	//uint8_t dma_addr_reg;
	//uint8_t dma_wrap_reg;
	
	// RAM
	uint8_t ram_page0[0xc0000];       // 0x00000000 - 0x000bffff : RAM
	//uint8_t vram_plane[0x8000 * 8]; // 0x000c0000 - 0x000c7fff : Plane Accessed VRAM
	//uint8_t text_ram[0x1000];       // 0x000c8000 - 0x000c8fff : Character VRAM
	//uint8_t vram_reserved[0x1000];    // 0x000c9000 - 0x000c9fff : Resetved
	uint8_t ram_0c0[0x8000];          // 0x000ca000 - 0x000cafff : ANKCG1 / IO / RAM
	uint8_t ram_0c8[0x2000];          // 0x000ca000 - 0x000cafff : ANKCG1 / IO / RAM
	
	//uint8_t sprite_ram[0x1000];     // 0x000ca000 - 0x000cafff : Sprite RAM
	//uint8_t ank_cg1[0x800];         // 0x000ca000 - 0x000ca7ff : ANK CG ROM (FONTROM[0x3d000 - 0x3d7ff])
	//uint8_t ank_cg2[0x1000];        // 0x000cb000 - 0x000cbfff : ANK CG ROM (FONTROM[0x3d800 - 0x3e7ff])
	uint8_t ram_0ca[0x1000];          // 0x000ca000 - 0x000cafff : ANKCG1 / IO / RAM
	uint8_t ram_0cb[0x1000];          // 0x000cb000 - 0x000cbfff : ANKCG2 / RAM
	uint8_t ram_0cc[0x4000];          // 0x000cc000 - 0x000cffff : MMIO / RAM
	uint8_t ram_0d0[0x8000];          // 0x000d0000 - 0x000d7fff : RAM / BANKED DICTIONARY
	uint8_t ram_0d8[0x2000];          // 0x000d8000 - 0x000d9fff : RAM / CMOS
	uint8_t ram_0da[0x16000];         // 0x000da000 - 0x000effff : RAM
	uint8_t ram_0f0[0x8000];          // 0x000f0000 - 0x000f7fff
	uint8_t ram_0f8[0x8000];          // 0x000f8000 - 0x000fffff : RAM/ROM

	uint8_t *extram;                  // 0x00100000 - (0x3fffffff) : Size is defined by extram_size;
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
	uint32_t vram_size; // Normally 512KB.

	uint8_t* read_bank_adrs_cx[0x100000]; // Per 4KB.
	uint8_t* write_bank_adrs_cx[0x100000]; // Per 4KB.
	DEVICE*   device_bank_adrs_cx[0x100000]; // Per 4KB.
	uint32_t type_bank_adrs_cx[0x100000]; // Per 4KB.

	void     write_data_base(uint32_t addr, uint32_t data, int* wait, int wordsize);
	uint32_t read_data_base(uint32_t addr, int* wait, int wordsize);
	bool     check_bank(uint32_t addr, uint32_t *mask, uint32_t *offset, void** readfn, void** writefn, void** readp, void** writep);
	virtual void initialize_tables(void);
	
	virtual uint32_t read_mmio(uint32_t addr, int *wait, bool *hit);
	virtual void     write_mmio(uint32_t addr, uint32_t data, int *wait, bool *hit);

public:
	TOWNS_MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("FMTOWNS_MEMORY"));
		d_cpu = NULL;
		d_vram = NULL;
		d_pcm = NULL;
		d_sprite = NULL;
		d_romcard[0] = d_romcard[1] = NULL;
		d_beep = NULL;
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
	
	virtual void     write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
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
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
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

