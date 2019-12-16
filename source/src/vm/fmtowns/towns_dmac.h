#pragma once

#include "../vm.h"
#include "../upd71071.h"

namespace FMTOWNS {
class TOWNS_DMAC : publiuc UPD71071
{
public:
	TOWNS_DMAC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : UPD71071(parent_vm, parent_emu)
	{
		set_device_name(_T("FM-Towns uPD71071 DMAC"));
	}
	~TOWNS_DMAC() {}
	// common functions
	virtual void initialize();
	virtual void reset();
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual void __FASTCALL do_dma();

	virtual bool process_state(FILEIO* state_fio, bool loading);
};

}
