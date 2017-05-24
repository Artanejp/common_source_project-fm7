/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#ifndef _I8080_H_ 
#define _I8080_H_

#include "i8080_base.h"

class I8080 : public I8080_BASE
{
private:
	/* ---------------------------------------------------------------------------
	virtual machine interfaces
	--------------------------------------------------------------------------- */
	
	// memory
	inline uint8_t RM8(uint16_t addr)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint8_t val = d_mem->read_data8w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data8(addr);
#endif
	}
	inline void WM8(uint16_t addr, uint8_t val)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data8(addr, val);
#endif
	}
	
	inline uint16_t RM16(uint16_t addr)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16_t val = d_mem->read_data16w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data16(addr);
#endif
	}
	inline void WM16(uint16_t addr, uint16_t val)
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data16w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data16(addr, val);
#endif
	}
	inline uint8_t FETCHOP()
	{
		int wait;
		uint8_t val = d_mem->fetch_op(PC++, &wait);
#ifdef I8080_MEMORY_WAIT
		count -= wait;
#endif
		return val;
	}
	inline uint8_t FETCH8()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint8_t val = d_mem->read_data8w(PC++, &wait);
		count -= wait;
		return val;
#else
		return d_mem->read_data8(PC++);
#endif
	}
	
	inline uint16_t FETCH16()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16_t val = d_mem->read_data16w(PC, &wait);
		count -= wait;
#else
		uint16_t val = d_mem->read_data16(PC);
#endif
		PC += 2;
		return val;
	}
	inline uint16_t POP16()
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		uint16_t val = d_mem->read_data16w(SP, &wait);
		count -= wait;
#else
		uint16_t val = d_mem->read_data16(SP);
#endif
		SP += 2;
		return val;
	}
	inline void PUSH16(uint16_t val)
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
	inline uint8_t IN8(uint8_t addr)
	{
#ifdef I8080_IO_WAIT
		int wait;
		uint8_t val = d_io->read_io8w(addr, &wait);
		count -= wait;
		return val;
#else
		return d_io->read_io8(addr);
#endif
	}
	inline void OUT8(uint8_t addr, uint8_t val)
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
	inline uint32_t ACK_INTR()
	{
		return d_pic->get_intr_ack();
	}
	/* ---------------------------------------------------------------------------
	opecodes
	--------------------------------------------------------------------------- */
	void run_one_opecode();
	void OP(uint8_t code);
	
public:
	I8080(VM* parent_vm, EMU* parent_emu) : I8080_BASE(parent_vm, parent_emu)
	{
#ifdef HAS_I8085
		set_device_name(_T("i8085 CPU"));
#endif
	}
	~I8080() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif


