/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#ifndef _I8080_BASE_H_ 
#define _I8080_BASE_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_I8080_INTR	0
//#ifdef HAS_I8085
#define SIG_I8085_RST5	1
#define SIG_I8085_RST6	2
#define SIG_I8085_RST7	3
#define SIG_I8085_SID	4
//#endif
#define SIG_I8080_INTE	5

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class  DLL_PREFIX I8080_BASE : public DEVICE
{
protected:
	bool _HAS_I8085;
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_pic;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
//#endif
	
	// output signals
	outputs_t outputs_busack;
	outputs_t outputs_sod;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	int count;
	pair32_t regs[4];
	uint16_t SP, PC, prevPC;
	uint16_t IM, RIM_IEN;
	bool afterHALT, BUSREQ, SID, afterEI;
	
	static const int cc_op_8080[0x100];
	static const int cc_op_8085[0x100];
	
	static const uint8_t ZS[256];
	static const uint8_t ZSP[256];
	static const uint16_t DAA[2048];

	virtual void __FASTCALL dec_count(uint8_t code) {}
	virtual void __FASTCALL check_reg_c(uint8_t val) {}
	virtual void __FASTCALL check_reg_e(uint8_t val) {}
	virtual void __FASTCALL check_reg_l(uint8_t val) {}
	virtual void __FASTCALL check_reg_sp(uint8_t val) {}
	virtual void __FASTCALL INSN_0x08(void) {}
	virtual void __FASTCALL INSN_0x10(void) {}
	virtual void __FASTCALL RLDE(void) {}
	virtual void __FASTCALL RIM(void) {}
	virtual void __FASTCALL _DAA(void) {}
	virtual void __FASTCALL LDEH(void) {}
	virtual void __FASTCALL CMA(void) {}
	virtual void __FASTCALL SIM(void) {}
	virtual void __FASTCALL LDES(void) {}
	virtual void __FASTCALL INSN_0xcb(void) {}
	virtual void __FASTCALL INSN_0xd9(void) {}
	virtual void __FASTCALL INSN_0xdd(void) {}
	virtual void __FASTCALL INSN_0xed(void) {}
	virtual void __FASTCALL INSN_0xfd(void) {}


	virtual void __FASTCALL JMP(uint8_t c);
	virtual void __FASTCALL CALL(uint8_t c);
	virtual void __FASTCALL ANA(uint8_t n);

	virtual uint8_t __FASTCALL RM8(uint16_t addr) { return 0xff;}
	virtual void __FASTCALL WM8(uint16_t addr, uint8_t val) {}
	virtual uint16_t __FASTCALL RM16(uint16_t addr) { return 0xffff;}
	virtual void __FASTCALL WM16(uint16_t addr, uint16_t val) {}
	virtual uint8_t __FASTCALL IN8(uint8_t addr) { return 0xff; }
	virtual void __FASTCALL OUT8(uint8_t addr, uint8_t val) {}
	virtual uint8_t __FASTCALL FETCHOP() { return 0xff;}
	virtual uint8_t __FASTCALL FETCH8() { return 0xff;}
	virtual uint16_t __FASTCALL FETCH16()  { return 0xffff; }
	virtual uint16_t __FASTCALL POP16() { return 0xff;}
	virtual void __FASTCALL PUSH16(uint16_t val) {}
	virtual uint32_t __FASTCALL ACK_INTR() {return 0xffffffff; }
	
	void __FASTCALL DSUB();

	inline void __FASTCALL INT(uint16_t v);
	inline void __FASTCALL RST(uint16_t n);

	void __FASTCALL OP(uint8_t code);

	uint64_t total_count;
	uint64_t prev_total_count;
public:
	I8080_BASE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		BUSREQ = false;
		SID = true;
		initialize_output_signals(&outputs_busack);
		initialize_output_signals(&outputs_sod);
		d_mem = d_pic = d_io = NULL;
		d_mem_stored = d_io_stored = NULL;
		d_debugger = NULL;
		total_count = prev_total_count = 0;
		set_device_name(_T("i8080 CPU"));
	}
	~I8080_BASE() {}
	virtual void initialize();
	virtual void reset();
	virtual __FASTCALL int run(int clock);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit);
	uint32_t get_pc()
	{
		return prevPC;
	}
	uint32_t get_next_pc()
	{
		return PC;
	}
//#ifdef USE_DEBUGGER
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
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device, uint32_t bit = 0xfffffffff)
	{
		d_pic = device;
	}
	void set_context_busack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_context_sod(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_sod, device, id, mask);
	}

};

inline void I8080_BASE::INT(uint16_t v)
{												\
	if(afterHALT) {
		PC++; afterHALT = 0;
	}
	PUSH16(PC); PC = (v);
}

inline void I8080_BASE::RST(uint16_t n)
{				
	PUSH16(PC); 
	PC = 8 * n; 
}

#endif
