/*
	Skelton for retropc emulator

	Origin : MESS 0.152
	Author : Takeda.Toshiya
	Date   : 2016.03.17-

	[ uPD7810 ]
*/

#ifndef _UPD7810_H_ 
#define _UPD7810_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_UPD7810_INTF1	0
#define SIG_UPD7810_INTF2	1
#define SIG_UPD7810_INTF0	2
#define SIG_UPD7810_INTFE1	3
#define SIG_UPD7810_NMI		4

class DEBUGGER;

class UPD7810 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	DEBUGGER *d_debugger;
	outputs_t outputs_to;
	outputs_t outputs_txd;
	void *opaque;
	uint64_t total_icount;
	uint64_t prev_total_icount;
	int icount;
	bool busreq, rxd_status;
	
	int __FASTCALL run_one_opecode();

	void process_state_cpustate(FILEIO* state_fio, bool loading);
public:
	UPD7810(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		total_icount = prev_total_icount = 0;
		d_debugger = NULL;
		initialize_output_signals(&outputs_to);
		initialize_output_signals(&outputs_txd);
		set_device_name(_T("uPD7810/7811 MCU"));
	}
	~UPD7810() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int __FASTCALL run(int clock);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc();
	uint32_t get_next_pc();

	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);

	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}

	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}

	void set_context_to(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_to, device, id, mask);
	}
	void set_context_txd(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_txd, device, id, mask);
	}
};

#endif

