/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#pragma once

#include "../device.h"

#define SIG_SERIALROM_CLK	        1
#define SIG_SERIALROM_CS	        2
#define SIG_SERIALROM_RESET	        3
#define SIG_SERIALROM_DATA	        4

namespace FMTOWNS {

class SERIAL_ROM : public DEVICE
{
private:
	uint8_t read_rom_bits(uint8_t pos);
protected:
	bool cs;
	bool clk;
	bool reset_reg;

	uint8_t rom_addr;
	uint8_t rom[32];

	uint16_t machine_id;
	uint8_t cpu_id;

	inline void __FASTCALL pos2addr(uint8_t pos, uint8_t& nibble, uint8_t& bit)
	{
		nibble = (32 - 1) - ((pos >> 3) & (32 - 1));
		bit = pos & 7;
		return;
	}
public:
	SERIAL_ROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		machine_id = 0x0100;
		cpu_id = 0x01;
		set_device_name(_T("FMTOWNS_SERIAL_ROM"));
	}
	~SERIAL_ROM() {}

	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;

	bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
};

}
