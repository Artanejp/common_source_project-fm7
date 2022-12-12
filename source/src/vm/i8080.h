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
#define SIG_I8080_INTE	5

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
	
#ifdef USE_DEBUGGER
	uint64_t total_count;
	uint64_t prev_total_count;
#endif
	int count;
	pair32_t regs[4];
	uint16_t SP, PC, prevPC;
	uint16_t IM, RIM_IEN;
	bool afterHALT, BUSREQ, SID, afterEI;
	
	/* ---------------------------------------------------------------------------
	virtual machine interfaces
	--------------------------------------------------------------------------- */
	
	// memory
	inline uint8_t RM8(uint16_t addr)
	{
		int wait = 0;
		uint8_t val = d_mem->read_data8w(addr, &wait);
		count -= wait;
		return val;
	}
	inline void WM8(uint16_t addr, uint8_t val)
	{
		int wait = 0;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
	}
	
	inline uint16_t RM16(uint16_t addr)
	{
		pair16_t val;
		val.b.l = RM8(addr    );
		val.b.h = RM8(addr + 1);
		return val.w;
	}
	inline void WM16(uint16_t addr, uint16_t val)
	{
		WM8(addr    , (val     ) & 0xff);
		WM8(addr + 1, (val >> 8) & 0xff);
	}
	inline uint8_t FETCHOP()
	{
		int wait = 0;
		uint8_t val = d_mem->fetch_op(PC++, &wait);
		count -= wait;
		return val;
	}
	inline uint8_t FETCH8()
	{
		return RM8(PC++);
	}
	inline uint16_t FETCH16()
	{
		uint16_t val = RM16(PC);
		PC += 2;
		return val;
	}
	inline uint16_t POP16()
	{
		uint16_t val = RM16(SP);
		SP += 2;
		return val;
	}
	inline void PUSH16(uint16_t val)
	{
		SP -= 2;
		WM16(SP, val);
	}
	
	// i/o
	inline uint8_t IN8(uint8_t addr)
	{
		int wait = 0;
		uint8_t val = d_io->read_io8w(addr, &wait);
		count -= wait;
		return val;
	}
	inline void OUT8(uint8_t addr, uint8_t val)
	{
		int wait = 0;
		d_io->write_io8w(addr, val, &wait);
		count -= wait;
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
	void check_interrupt();
	void OP(uint8_t code);
	
public:
	I8080(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_count = prev_total_count = 0;
#endif
		BUSREQ = false;
		SID = true;
		initialize_output_signals(&outputs_busack);
		initialize_output_signals(&outputs_sod);
		set_device_name(_T("8080 CPU"));
	}
	~I8080() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	uint32_t get_pc()
	{
		return prevPC;
	}
	uint32_t get_next_pc()
	{
		return PC;
	}
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
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
	void set_context_busack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_context_sod(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_sod, device, id, mask);
	}
};

#endif


