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
protected:
	/* ---------------------------------------------------------------------------
	virtual machine interfaces
	--------------------------------------------------------------------------- */
	
	// memory
	uint8_t __FASTCALL RM8(uint16_t addr)  override
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
	void __FASTCALL WM8(uint16_t addr, uint8_t val)  override
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data8(addr, val);
#endif
	}
	
	uint16_t __FASTCALL RM16(uint16_t addr) override
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
	void __FASTCALL WM16(uint16_t addr, uint16_t val)  override
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data16w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data16(addr, val);
#endif
	}
	uint8_t __FASTCALL FETCHOP()  override
	{
		int wait;
		uint8_t val = d_mem->fetch_op(PC++, &wait);
#ifdef I8080_MEMORY_WAIT
		count -= wait;
#endif
		return val;
	}
	uint8_t __FASTCALL FETCH8()  override
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
	
	uint16_t __FASTCALL FETCH16()  override
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
	uint16_t __FASTCALL POP16()  override
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
	void __FASTCALL PUSH16(uint16_t val)  override
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
	uint8_t __FASTCALL IN8(uint8_t addr)  override
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
	void __FASTCALL OUT8(uint8_t addr, uint8_t val)  override
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
	uint32_t ACK_INTR()  override
	{
		return d_pic->get_intr_ack();
	}

	
	void __FASTCALL dec_count(uint8_t code) override;
	void __FASTCALL check_reg_c(uint8_t val) override;
	void __FASTCALL check_reg_e(uint8_t val) override;
	void __FASTCALL check_reg_l(uint8_t val) override;
	void __FASTCALL check_reg_sp(uint8_t val) override;
	void __FASTCALL INSN_0x08(void) override;
	void __FASTCALL INSN_0x10(void) override;
	void __FASTCALL RLDE(void) override;
	void __FASTCALL RIM(void) override;
	void __FASTCALL _DAA(void) override;
	void __FASTCALL LDEH(void) override;
	void __FASTCALL CMA(void) override;
	void __FASTCALL SIM(void) override;
	void __FASTCALL LDES(void) override;
	void __FASTCALL INSN_0xcb(void) override;
	void __FASTCALL INSN_0xd9(void) override;
	void __FASTCALL INSN_0xdd(void) override;
	void __FASTCALL INSN_0xed(void) override;
	void __FASTCALL INSN_0xfd(void) override;


	void __FASTCALL JMP(uint8_t c) override;
	void __FASTCALL CALL(uint8_t c) override;
	void __FASTCALL ANA(uint8_t n) override;
	/* ---------------------------------------------------------------------------
	opecodes
	--------------------------------------------------------------------------- */
	void run_one_opecode();
	void check_interrupt();
	//void OP(uint8_t code);
public:
	I8080(VM_TEMPLATE* parent_vm, EMU* parent_emu) : I8080_BASE(parent_vm, parent_emu)
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
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif


