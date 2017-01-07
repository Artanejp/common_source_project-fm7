/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.07 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

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
// 00010000 - 3fffffff : EXTRA RAM (for i386 native mode.Page size is 1MB.)
// 40000000 - 7fffffff : External I/O BOX.Not accessible.
// 80000000 - bfffffff : VRAM (Reserved 01020000H bytes with earlier Towns.)
// c0000000 - c21fffff : ROM card and dictionaly/font/DOS ROMs.
// c2200000 - c2200fff : PCM RAM (Banked).
// c2201000 - fffbffff : Reserved.
// FFFC0000 - FFFFFFFF : Towns System ROM.

enum {
	TOWNS_MEMORY_TYPE_FORBID = 0,
	TOWNS_MEMORY_TYPE_PAGE0  = 1, // - 0xfffff
	TOWNS_MEMORY_TYPE_INTRAM,     // 0x100000 -
	TOWNS_MEMORY_TYPE_EXT_MMIO,
	TOWNS_MEMORY_TYPE_VRAM,
	TOWNS_MEMORY_TYPE_SPRITE,
	TOWNS_MEMORY_TYPE_ROMCARD,
	TOWNS_MEMORY_TYPE_MSDOS,
	TOWNS_MEMORY_TYPE_DICTROM,
	TOWNS_MEMORY_TYPE_KANJIFONT,
	TOWNS_MEMORY_TYPE_DICTLEARN,
	TOWNS_MEMORY_TYPE_WAVERAM,
	TOWNS_MEMORY_TYPE_SYSTEM_ROM,
};

	
class MEMORY : public DEVICE
{
private:
	I386 *d_cpu;

	DEVICE *d_vram;

	uint8_t  read_banktype_head[256]; // xx000000 - xxffffff
	uint32_t read_offset_head[256];
	uint8_t  write_banktype_head[256]; // xx000000 - xxffffff
	uint32_t write_offset_head[256];

	uint8_t *read_bank_adrs_cx[0x400 * 16];   // C0000000 - C3FFFFFF : Per 4KB
	uint8_t *write_bank_adrs_cx[0x400 * 16];  // C0000000 - C3FFFFFF : Per 4KB
	uint8_t device_type_adrs_cx[0x400 * 16];  // C0000000 - C3FFFFFF : Per 4KB

	uint8_t *read_bank_adrs_fx[0x1000]; // FF000000 - FFFFFFFF : Per 4KB

	uint8_t *extram_base; // 0x100000 - : 2MB / 4MB / 6MB
	int32_t extram_pages; //
	uint8_t *extram_adrs[4096]; // 256MB / 64KB

	uint8_t msdos_rom[0x80000]; // MSDOS ROM. READ ONLY.
	uint8_t dict_rom[0x80000];  // Dictionary rom. READ ONLY.
	uint8_t font_rom[0x40000]; // Font ROM. READ ONLY.
	uint8_t system_rom[0x40000]; // System ROM. READ ONLY.
	uint8_t machine_id[2];	// MACHINE ID

	// memory
	uint8_t protect, rst;
	uint8_t mainmem, rplane, wplane;
	uint8_t dma_addr_reg, dma_wrap_reg;
	uint32_t dma_addr_mask;
	
	void update_bank();
	void update_dma_addr_mask();
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("MEMORY"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
#if defined(HAS_I286)
	void set_context_cpu(I286* device)
#else
	void set_context_cpu(I386* device)
#endif
	{
		d_cpu = device;
	}
	void set_machine_id(uint8_t id)
	{
		machine_id = id;
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_chregs_ptr(uint8_t* ptr)
	{
		chreg = ptr;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_cvram()
	{
		return cvram;
	}
#ifdef _FMR60
	uint8_t* get_avram()
	{
		return avram;
	}
#else
	uint8_t* get_kvram()
	{
		return kvram;
	}
#endif
	void draw_screen();
};

#endif

