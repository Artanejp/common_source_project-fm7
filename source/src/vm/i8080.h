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
	uint8_t RM8(uint16_t addr)  override
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
	void WM8(uint16_t addr, uint8_t val)  override
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data8(addr, val);
#endif
	}
	
	uint16_t RM16(uint16_t addr) override
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
	void WM16(uint16_t addr, uint16_t val)  override
	{
#ifdef I8080_MEMORY_WAIT
		int wait;
		d_mem->write_data16w(addr, val, &wait);
		count -= wait;
#else
		d_mem->write_data16(addr, val);
#endif
	}
	uint8_t FETCHOP()  override
	{
		int wait;
		uint8_t val = d_mem->fetch_op(PC++, &wait);
#ifdef I8080_MEMORY_WAIT
		count -= wait;
#endif
		return val;
	}
	uint8_t FETCH8()  override
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
	
	uint16_t FETCH16()  override
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
	uint16_t POP16()  override
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
	void PUSH16(uint16_t val)  override
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
	uint8_t IN8(uint8_t addr)  override
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
	void OUT8(uint8_t addr, uint8_t val)  override
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

	
	void dec_count(uint8_t code) override;
	void check_reg_c(uint8_t val) override;
	void check_reg_e(uint8_t val) override;
	void check_reg_l(uint8_t val) override;
	void check_reg_sp(uint8_t val) override;
	void INSN_0x08(void) override;
	void INSN_0x10(void) override;
	void RLDE(void) override;
	void RIM(void) override;
	void _DAA(void) override;
	void LDEH(void) override;
	void CMA(void) override;
	void SIM(void) override;
	void LDES(void) override;
	void INSN_0xcb(void) override;
	void INSN_0xd9(void) override;
	void INSN_0xdd(void) override;
	void INSN_0xed(void) override;
	void INSN_0xfd(void) override;


	void JMP(uint8_t c) override;
	void CALL(uint8_t c) override;
	void ANA(uint8_t n) override;
	/* ---------------------------------------------------------------------------
	opecodes
	--------------------------------------------------------------------------- */
	void run_one_opecode();
	//void OP(uint8_t code);
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


