/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.14-

	[ memory ]
*/

#ifndef _EX80BS_MEMORY_H_
#define _EX80BS_MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_COLUMN	0

namespace EX80BS {
class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
	DEVICE *d_pio;
	
	uint32_t column;
	bool kana;
	uint8_t pressed;
	uint8_t irq_bits;
	
	void update_kb();
	uint8_t mon[0x800];
	uint8_t prom1[0x400];
	uint8_t prom2[0x400];
	uint8_t bas1[0x1000];
	uint8_t bas2[0x2800];
	uint8_t ram[0x4000];
	uint8_t vram[0x400];
	
	uint8_t wdmy[0x400];
	uint8_t rdmy[0x400];
	uint8_t* wbank[64];
	uint8_t* rbank[64];
	
	int boot_mode;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_frame() override;
	
	void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_data8(uint32_t addr) override;
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) override;
	uint32_t get_intr_ack() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	uint8_t* get_ram()
	{
		return ram;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	void key_down(int code);
	void load_binary(const _TCHAR* file_path);
	void save_binary(const _TCHAR* file_path);
};
}

#endif

