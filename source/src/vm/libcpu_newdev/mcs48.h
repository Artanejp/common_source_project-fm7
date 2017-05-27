/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/

#ifndef _LIBNEWDEV_MCS84_H_ 
#define _LIBNEWDEV_MCS48_H_

#include "../vm.h"
#include "../emu.h"
#include "./device.h"
#include "./mcs48_base.h"
#define MCS48_PORT_P0	0x100	/* Not used */
#define MCS48_PORT_P1	0x101	/* P10-P17 */
#define MCS48_PORT_P2	0x102	/* P20-P28 */
#define MCS48_PORT_T0	0x110
#define MCS48_PORT_T1	0x111
#define MCS48_PORT_BUS	0x120	/* DB0-DB7 */
#define MCS48_PORT_PROG	0x121	/* PROG line to 8243 expander */

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class VM;
class EMU;
class MCS48 : public MCS48_BASE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	void *opaque;
public:
	MCS48(VM* parent_vm, EMU* parent_emu) : MCS48_BASE(parent_vm, parent_emu)
	{
	}
	~MCS48() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xfff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xff;
	}
#endif
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	// unique functions
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

