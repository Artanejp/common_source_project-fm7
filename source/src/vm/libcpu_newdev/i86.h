/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ 80x86 ]
*/

#ifndef _LIBNEWDEV_I86_H_ 
#define _LIBNEWDEV_I86_H_

#include "vm.h"
#include "../emu.h"
#include "./i86_base.h"

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I86 : public I86_BASE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	// opecode
	void run_one_opecode();
	void run_one_opecode_debugger();
	void instruction(uint8_t code);
	void _repc(int flagval);
	void _call_far();
	void _int();
	void _aad();
	void _call_d16();
	void _rep(int flagval);
	void _ffpre();
#if defined(HAS_V30)
	void _0fpre();    /* Opcode 0x0f */
#endif
public:
	I86(VM* parent_vm, EMU* parent_emu) : I86_BASE(parent_vm, parent_emu)
	{
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		busreq = false;
#if defined(HAS_I86)
		set_device_name(_T("i8086 CPU"));
#elif defined(HAS_V30)
		set_device_name(_T("V30 CPU"));
#else
		set_device_name(_T("i80186 CPU"));
#endif		
	}
	~I86() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xfffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xfffff;
	}
#endif
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif
