#pragma once
#include "i86.h"

class V30 : public I8086
{
	public:
	V30(VM_TEMPLATE* parent_vm, EMU* parent_emu) : I8086(parent_vm, parent_emu)
	{
		set_device_name(_T("NEC V30 CPU"));
	}
	~V30() {}
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual int __FASTCALL run(int icount);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);

};
