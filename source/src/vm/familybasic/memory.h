/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PPU;

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_apu, *d_drec, *d_opll;
	PPU *d_ppu;
	
	_TCHAR save_file_name[_MAX_PATH];
	
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	
	header_t header;
	uint32_t rom_size;
	uint32_t rom_mask;
//	uint8_t rom[0x8000];
	uint8_t *rom;
	uint8_t ram[0x800];
//	uint8_t save_ram[0x2000];
	uint8_t save_ram[0x10000];
	uint32_t save_ram_crc32;
	
	uint32_t banks[8];
	uint8_t *bank_ptr[8];
	uint8_t dummy[0x2000];
	
	uint8_t *spr_ram;
	uint16_t dma_addr;
	uint8_t frame_irq_enabled;
	
	bool pad_strobe;
	uint8_t pad1_bits, pad2_bits;
	
	bool kb_out;
	uint8_t kb_scan;
	
	void set_rom_bank(uint8_t bank, uint32_t bank_num);
	
	// mmc5
//	DEVICE *d_mmc5;
	uint32_t mmc5_wram_bank[8];
	uint8_t mmc5_chr_reg[8][2];
	uint32_t mmc5_value0;
	uint32_t mmc5_value1;
	uint8_t mmc5_wram_protect0;
	uint8_t mmc5_wram_protect1;
	uint8_t mmc5_prg_size;
	uint8_t mmc5_chr_size;
	uint8_t mmc5_gfx_mode;
//	uint8_t mmc5_split_control;
//	uint8_t mmc5_split_bank;
	uint8_t mmc5_irq_enabled;
	uint8_t mmc5_irq_status;
	uint32_t mmc5_irq_line;
	
	void mmc5_reset();
	uint32_t mmc5_lo_read(uint32_t addr);
	void mmc5_lo_write(uint32_t addr, uint32_t data);
//	uint32_t mmc5_save_read(uint32_t addr);
	void mmc5_save_write(uint32_t addr, uint32_t data);
	void mmc5_hi_write(uint32_t addr, uint32_t data);
	void mmc5_hsync(int v);
	void mmc5_set_cpu_bank(uint8_t bank, uint32_t bank_num);
	void mmc5_set_wram_bank(uint8_t bank, uint32_t bank_num);
	void mmc5_set_ppu_bank(uint8_t mode);
	
	// vrc7
	uint8_t vrc7_irq_enabled;
	uint8_t vrc7_irq_counter;
	uint8_t vrc7_irq_latch;
	
	void vrc7_reset();
	uint32_t vrc7_lo_read(uint32_t addr);
	void vrc7_lo_write(uint32_t addr, uint32_t data);
//	uint32_t vrc7_save_read(uint32_t addr);
//	void vrc7_save_write(uint32_t addr, uint32_t data);
	void vrc7_hi_write(uint32_t addr, uint32_t data);
	void vrc7_hsync(int v);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_apu(DEVICE* device)
	{
		d_apu = device;
	}
	void set_context_ppu(PPU* device)
	{
		d_ppu = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
//	void set_context_mmc5(DEVICE* device)
//	{
//		d_mmc5 = device;
//	}
	void set_context_opll(DEVICE* device)
	{
		d_opll = device;
	}
	void set_spr_ram_ptr(uint8_t* ptr)
	{
		spr_ram = ptr;
	}
	void load_rom_image(const _TCHAR *file_name);
	void save_backup();
	uint8_t mmc5_ppu_latch_render(uint8_t mode, uint32_t addr);
};

#endif
