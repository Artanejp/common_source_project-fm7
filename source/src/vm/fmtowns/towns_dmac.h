#pragma once

#include "../vm.h"
#include "../upd71071.h"

// Using original signal using after 1 << 12.
#define SIG_TOWNS_DMAC_ADDR_REG     4096
#define SIG_TOWNS_DMAC_WRAP_REG     4100
#define SIG_TOWNS_DMAC_ADDR_MASK    4104
#define SIG_TOWNS_DMAC_HIGH_ADDRESS 4108

namespace FMTOWNS {
class TOWNS_DMAC : public UPD71071
{
protected:
	uint8_t dma_addr_reg;
	uint8_t dma_wrap_reg;
	uint32_t dma_addr_mask;
	uint32_t dma_high_address;
#if 0
	virtual bool __FASTCALL do_dma_per_channel(int _ch);
	virtual bool __FASTCALL do_dma_prologue(int c);
#endif
#if 1
	virtual void __FASTCALL do_dma_verify_16bit(int c);
	virtual void __FASTCALL do_dma_dev_to_mem_16bit(int c);
	virtual void __FASTCALL do_dma_mem_to_dev_16bit(int c);
#endif
	virtual void __FASTCALL do_dma_inc_dec_ptr_8bit(int c);
	virtual void __FASTCALL do_dma_inc_dec_ptr_16bit(int c);
public:
	TOWNS_DMAC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : UPD71071(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns uPD71071 DMAC"));
	}
	~TOWNS_DMAC() {}
	// common functions
	virtual void initialize();
	virtual void reset();
//	virtual void __FASTCALL do_dma();
	
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual bool process_state(FILEIO* state_fio, bool loading);

	// for debug
	virtual void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr);
	virtual void __FASTCALL write_via_debugger_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_via_debugger_data16(uint32_t addr);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	
};

}
