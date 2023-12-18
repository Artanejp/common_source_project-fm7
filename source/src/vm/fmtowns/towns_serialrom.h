/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom interface]
	History:
	2023.12.18  Sprit I/O interface from serialrom.h .
*/

#pragma once

#include "device.h"

class SERIALROM;
class DEBUGGER;
namespace FMTOWNS {

class TOWNS_SERIAL_ROM : public DEVICE
{
protected:
	SERIALROM*	d_rom;
	DEBUGGER*   d_debugger;

	uint8_t prev_value;
	bool load_ok;
	//uint16_t machine_id;
	//uint16_t cpu_id;

	const uint8_t REG_VALUE_DATA	= 0x01;	// RO
	const uint8_t REG_VALUE_CS		= 0x20;	// WO
	const uint8_t REG_VALUE_CLK		= 0x40;	// RW
	const uint8_t REG_VALUE_RESET	= 0x80;	// RW

	virtual void initialize_rom();
	virtual void __FASTCALL write_to_rom(uint8_t data);

public:
	TOWNS_SERIAL_ROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		//machine_id = 0x0100;
		//cpu_id = 0x01;
		d_rom = NULL;
		d_debugger = NULL;
		set_device_name(_T("FM Towns SERIAL ROM Interface"));
	}
	~TOWNS_SERIAL_ROM() {}

	virtual void initialize() override;
	virtual void reset() override;
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_io8(uint32_t addr) override;

	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	bool is_debugger_available() override
	{
		return true;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	uint64_t get_debug_data_addr_space() override
	{
		return 1;
	}
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// Unique function
	void set_context_serialrom(SERIALROM* dev)
	{
		d_rom = dev;
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void set_machine_id(uint16_t val)
	{
		//machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		//cpu_id = val & 0x07;
	}
};

}
