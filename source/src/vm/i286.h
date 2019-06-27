/*
	Skelton for retropc emulator

	Origin : MAME i286 core
	Author : Takeda.Toshiya
	Date  : 2012.10.18-

	[ i286 ]
*/

#ifndef _I286_H_ 
#define _I286_H_

#include "./i86.h"
#define SIG_I286_A20	1

class DEBUGGER;

class I80286 : public I8086
{
public:
	I80286(VM_TEMPLATE* parent_vm, EMU* parent_emu) : I8086(parent_vm, parent_emu)
	{
		set_device_name(_T("80286 CPU"));
	}
	~I80286() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
	uint32_t __FASTCALL read_signal(int id);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int icount);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
	uint32_t __FASTCALL translate_address(int segment, uint32_t offset);


	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io16(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);

	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
};

#endif
