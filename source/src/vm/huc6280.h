/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#ifndef _HUC6280_H_ 
#define _HUC6280_H_

#include "device.h"

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class  DLL_PREFIX HUC6280_BASE : public DEVICE
{
protected:
	DEVICE *d_mem, *d_io;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
//#endif
	void *opaque;
	int icount;
	bool busreq;
	int exec_call(void);
	int exec_call_debug(void);
	uint64_t total_icount;
	uint64_t prev_total_icount;

	virtual int __FASTCALL run_one_opecode();
	
public:
	HUC6280_BASE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		total_icount = prev_total_icount = 0;
		d_debugger = NULL;
		set_device_name(_T("HuC6280 CPU"));
	}
	~HUC6280_BASE() {}
	
	// common functions
	virtual void initialize() override;
	virtual void release() override;
	virtual void reset() override;
	virtual int __FASTCALL run(int clock)  override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t get_pc() override;
	uint32_t get_next_pc() override;
//#ifdef USE_DEBUGGER
	bool is_cpu() override
	{
		return true;
	}
	bool is_debugger_available() override
	{
		return true;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask() override
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask() override
	{
		return 0x1fffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_debug_data8(uint32_t addr) override;
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_debug_io8(uint32_t addr) override;
	bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0) override;
//#endif
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	uint8_t irq_status_r(uint16_t offset);
	void irq_status_w(uint16_t offset, uint8_t data);
	uint8_t timer_r(uint16_t offset);
	void timer_w(uint16_t offset, uint8_t data);
};

class HUC6280 : public HUC6280_BASE
{
private:
protected:
	int __FASTCALL run_one_opecode() override;
public:
	HUC6280(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : HUC6280_BASE(parent_vm, parent_emu) {
		set_device_name(_T("HuC6280 CPU"));
	}
	~HUC6280() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void reset() override;
	int __FASTCALL run(int clock) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

