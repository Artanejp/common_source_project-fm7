/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom template: Four wires serial ROM for FM Towns]
*/

#pragma once

#include "device.h"

#define SIG_SERIALROM_CLK	        1
#define SIG_SERIALROM_CS	        2
#define SIG_SERIALROM_RESET	        3
#define SIG_SERIALROM_DATA	        4

class DEBUGGER;
class SERIALROM : public DEVICE
{
protected:
	DEBUGGER* d_debugger;
	uint32_t rom_size;
	uint32_t addr_mask;
	uint32_t bit_mask;
	uint8_t *rom;

	bool cs;
	bool clk;
	bool reset_reg;
	bool prev_reset;

	uint32_t rom_addr;

	/*!
	  @brief		convert bit address to nibble (a.k.a byte position) and bit position
	  @param pos	bit address (incremented by clk)
	  @param nibble	returned rom address (by bytes).
	  @param bit	returned rom bit position within this nibble.
	*/
	virtual void __FASTCALL pos2addr(uint32_t pos, uint32_t& nibble, uint8_t& bit);
	/*!
	  @brief		read rom bit at bit address
	  @param pos	bit address (incremented by clk)
	  @return		0x00000000 when this bit is '0'
	  @return		0xffffffff when this bit is '1'
	*/
	virtual uint32_t __FASTCALL read_rom_bit(uint32_t pos);
	/*!
	  @brief		write bit to  rom at bit address
	  @param pos	bit address (incremented by clk)
	  @param data	bit data true is '1', false is '0'.
	  @note			This is dummy function.Please modify when you need.
	*/
	virtual void __FASTCALL write_rom_bit(uint32_t pos, bool data)
	{
	}
	/*!
	  @brief		reset device with checking CS when rise up RESET.
	  @note			When CS is enabled, rise up (0 to 1) reset, reset device.
	*/
	virtual void check_and_reset_device();
public:
	SERIALROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		rom = NULL;
		d_debugger = NULL;
		rom_size = 0;
		addr_mask = 0;
		__USE_DEBUGGER = false;
		set_device_name(_T("SERIAL ROM"));
	}
	~SERIALROM() {}

	virtual void initialize() override;
	virtual void release() override;

	/*!
	  @brief	write signal 3bit (or 4bit) bus.
	  @param id SIG_SERIALROM_CLK	Address count up signal. when riseup and CS has enabled,
									increment bit address. When CS has not enabled,
									not effects, only setting CLK.
	  @param id	SIG_SERIALROM_CS	Setting CS.
	  @param id	SIG_SERIALROM_RESET	Set RESET status, then call check_and_reset_device() .
	  @param id	SIG_SERIALROM_DATA	Writing data bit, but this has no mean of ROM.
	  @param data	signal data.
	  @param mask	signal mask.
	*/
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	/*!
	  @brief	Read signals of this ROM.
	  @param	id	SIG_SERIAL_ROM_CLK		Read rom CLK bit.
	  @param	id	SIG_SERIAL_ROM_CS		Read CS bit.
	  @param	id	SIG_SERIAL_ROM_RESET	Read rom RESET register bit.
	  @param	id	SIG_SERIAL_ROM_DATA		Read rom DATA bit.
	  @return	0xffffffff  When bit status is '1'.
	  @return	0x00000000  When bit status is '0'.
	*/
	virtual uint32_t __FASTCALL read_signal(int ch) override;

	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr) override;
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;

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
		return rom_size;
	}
	virtual bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function (available to inherit)
	virtual uint32_t load_data(const uint8_t* data, uint32_t size);
	virtual uint32_t allocate_memory(uint32_t size);
	virtual bool is_initialized(uint32_t size);

	// unique function (*NOT* available to inherit. Should call before initialize().)
	void set_memory_size(uint32_t size)
	{
		rom_size = size;
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}

};
