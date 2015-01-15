/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#ifndef _I8080_H_ 
#define _I8080_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8080_INTR	0
#ifdef HAS_I8085
#define SIG_I8085_RST5	1
#define SIG_I8085_RST6	2
#define SIG_I8085_RST7	3
#define SIG_I8085_SID	4
#endif

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I8080 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_pic;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	
	// output signals
	outputs_t outputs_busack;
	outputs_t outputs_sod;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	int count;
	pair regs[4];
	uint16 SP, PC, prevPC;
	uint16 IM, RIM_IEN;
	bool HALT, BUSREQ, SID, afterEI;
	
	/* ---------------------------------------------------------------------------
	virtual machine interfaces
	--------------------------------------------------------------------------- */
	
	// memory
	inline uint8 RM8(uint16 addr)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint8 val = d_mem->read_data8w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data8(addr);
#endif
	}
	inline void WM8(uint16 addr, uint8 val)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data8(addr, val);
#endif
	}
	
	inline uint16 RM16(uint16 addr)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16 val = d_mem->read_data16w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data16(addr);
#endif
	}
	inline void WM16(uint16 addr, uint16 val)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data16w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data16(addr, val);
#endif
	}
	inline uint8 FETCHOP()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint8 val = d_mem->read_data8w(PC++, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data8(PC++);
#endif
	}
	inline uint8 FETCH8()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint8 val = d_mem->read_data8w(PC++, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data8(PC++);
#endif
	}
	inline uint16 FETCH16()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16 val = d_mem->read_data16w(PC, &wait);
		count -= wait;
#else
		uint16 val = d_mem->read_data16(PC);
#endif
		PC += 2;
		return val;
	}
	inline uint16 POP16()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16 val = d_mem->read_data16w(SP, &wait);
		count -= wait;
#else
		uint16 val = d_mem->read_data16(SP);
#endif
		SP += 2;
		return val;
	}
	inline void PUSH16(uint16 val)
	{
		SP -= 2;
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data16w(SP, val, &wait);
		count -= wait;
#else
		d_mem->write_data16(SP, val);
#endif
	}
	
	// i/o
	inline uint8 IN8(uint8 addr)
	{
#ifdef I8080_IO_WAIT
		int wait;
		uint8 val = d_io->read_io8w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_io->read_io8(addr);
#endif
	}
	inline void OUT8(uint8 addr, uint8 val)
	{
#ifdef I8080_IO_WAIT
		int wait;
		d_io->write_io8w(addr, val, &wait);
		count -= wait;
#else
		d_io->write_io8(addr, val);
#endif
	}
	
	// interrupt
	inline uint32 ACK_INTR()
	{
		return d_pic->intr_ack();
	}
	
	/* ---------------------------------------------------------------------------
	opecodes
	--------------------------------------------------------------------------- */
	
	void run_one_opecode();
	void OP(uint8 code);
	
public:
	I8080(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		BUSREQ = false;
		SID = true;
		init_output_signals(&outputs_busack);
		init_output_signals(&outputs_sod);
	}
	~I8080() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	void set_intr_line(bool line, bool pending, uint32 bit);
	uint32 get_pc()
	{
		return prevPC;
	}
	uint32 get_next_pc()
	{
		return PC;
	}
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32 debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32 debug_data_addr_mask()
	{
		return 0xffff;
	}
	void debug_write_data8(uint32 addr, uint32 data);
	uint32 debug_read_data8(uint32 addr);
	void debug_write_io8(uint32 addr, uint32 data);
	uint32 debug_read_io8(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32 pc, _TCHAR *buffer, size_t buffer_len);
#endif
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void set_context_busack(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_context_sod(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_sod, device, id, mask);
	}
};

#endif


