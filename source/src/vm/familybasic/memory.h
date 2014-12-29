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

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_apu, *d_ppu, *d_drec;
	
	_TCHAR save_file_name[_MAX_PATH];
	
	uint8* key_stat;
	uint32* joy_stat;
	
	uint8 header[16];
	uint8 rom[0x8000];
	uint8 ram[0x800];
	uint8 save_ram[0x2000];
	uint32 save_ram_crc32;
	
	uint8 *spr_ram;
	uint16 dma_addr;
	uint8 frame_irq_enabled;
	
	bool pad_strobe;
	uint8 pad1_bits, pad2_bits;
	
	bool kb_out;
	uint8 kb_scan;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_apu(DEVICE* device)
	{
		d_apu = device;
	}
	void set_context_ppu(DEVICE* device)
	{
		d_ppu = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_spr_ram_ptr(uint8* ptr)
	{
		spr_ram = ptr;
	}
	void load_rom_image(_TCHAR *file_name);
	void save_backup();
};

#endif
