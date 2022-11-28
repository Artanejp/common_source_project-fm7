/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ pac slot 2 base class ]
*/

#ifndef _PAC2DEV_H_
#define _PAC2DEV_H_

#include "../vm.h"
#include "../../emu.h"

class PAC2DEV
{
protected:
	VM_TEMPLATE* vm;
	EMU* emu;
public:
	PAC2DEV(VM_TEMPLATE* parent_vm, EMU* parent_emu) : vm(parent_vm), emu(parent_emu)
	{
		set_device_name(_T("PAC2 Base Device"));
	}
	~PAC2DEV(void) {}
	
	virtual void initialize(int id) {}
	virtual void release() {}
	virtual void reset() {}
	virtual void write_io8(uint32_t addr, uint32_t data) {}
	virtual uint32_t read_io8(uint32_t addr) { return 0xff; }
	virtual bool process_state(FILEIO* state_fio, bool loading) { return true; }
	
	virtual void set_device_name(const _TCHAR* format, ...)
	{
		if(format != NULL) {
			va_list ap;
			_TCHAR buffer[1024];
			
			va_start(ap, format);
			my_vstprintf_s(buffer, 1024, format, ap);
			va_end(ap);
			
			my_tcscpy_s(this_device_name, 128, buffer);
#ifdef _USE_QT
//			emu->get_osd()->set_vm_node(this_device_id, buffer);
#endif
		}
	}
	virtual const _TCHAR *get_device_name()
	{
		return (const _TCHAR *)this_device_name;
	}
	_TCHAR this_device_name[128];
};

#endif

