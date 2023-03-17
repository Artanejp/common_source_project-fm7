/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#ifndef _I8080_H_
#define _I8080_H_

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

class  DLL_PREFIX I8080 : public DEVICE
{
protected:
	bool _HAS_I8085;
	bool _HAS_I8080;
	bool __FP200;
	uint32_t __CPU_START_ADDR;
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
	const int *cc_op;

	static const uint8_t ZS[256];
	static const uint8_t ZSP[256];
	static const uint16_t DAA[2048];

	void __FASTCALL RLDE_8085(void);
	void __FASTCALL RIM_8085(void);
	void __FASTCALL _DAA(void);
	void __FASTCALL CMA_8080(void);
	void __FASTCALL CMA_8085(void);
	void __FASTCALL JMP(uint8_t c);
	void __FASTCALL JMP_8085(uint8_t c);
	void __FASTCALL JMP_8080(uint8_t c);
	void __FASTCALL CALL(uint8_t c);
	void __FASTCALL CALL_8085(uint8_t c);
	void __FASTCALL CALL_8080(uint8_t c);
	void __FASTCALL ANA(uint8_t n);
	void __FASTCALL ANA_8085(uint8_t n);
	void __FASTCALL ANA_8080(uint8_t n);

	// memory
	inline uint8_t __FASTCALL RM8(uint16_t addr)
	{
		int wait = 0;
		uint8_t val = d_mem->read_data8w(addr, &wait);
		count -= wait;
		return val;
	}
	inline void __FASTCALL WM8(uint16_t addr, uint8_t val)
	{
		int wait = 0;
		d_mem->write_data8w(addr, val, &wait);
		count -= wait;
	}
	inline uint16_t __FASTCALL RM16(uint16_t addr)
	{
		pair16_t val;
		val.b.l = RM8(addr    );
		val.b.h = RM8(addr + 1);
		return val.w;
	}
	inline void __FASTCALL WM16(uint16_t addr, uint16_t val)
	{
		WM8(addr    , (val     ) & 0xff);
		WM8(addr + 1, (val >> 8) & 0xff);
	}
	inline uint8_t __FASTCALL FETCHOP()
	{
		int wait = 0;
		uint8_t val = d_mem->fetch_op(PC++, &wait);
		count -= wait;
		return val;
	}
	inline uint8_t __FASTCALL FETCH8()
	{
		return RM8(PC++);
	}

	inline uint16_t __FASTCALL FETCH16()
	{
		uint16_t val = RM16(PC);
		PC += 2;
		return val;
	}
	inline uint16_t __FASTCALL POP16()
	{
		uint16_t val = RM16(SP);
		SP += 2;
		return val;
	}
	inline void __FASTCALL PUSH16(uint16_t val)
	{
		SP -= 2;
		WM16(SP, val);
	}

	// i/o
	inline uint8_t __FASTCALL IN8(uint8_t addr)
	{
		int wait = 0;
		uint8_t val = d_io->read_io8w(addr, &wait);
		count -= wait;
		return val;
	}
	inline void __FASTCALL OUT8(uint8_t addr, uint8_t val)
	{
		int wait = 0;
		d_io->write_io8w(addr, val, &wait);
		count -= wait;
	}

	// interrupt
	inline uint32_t __FASTCALL ACK_INTR()
	{
		return d_pic->get_intr_ack();
	}

	void __FASTCALL DSUB_8085();

	inline void __FASTCALL __INT(uint16_t v)
	{
		PUSH16(PC);
		PC = (v);
	}

	inline void __FASTCALL __RST(uint16_t n)
	{
		PUSH16(PC);
		PC = 8 * n;
	}

	void __FASTCALL OP(uint8_t code);
	void __FASTCALL run_one_opecode();
	void check_interrupt();

	uint64_t total_count;
	uint64_t prev_total_count;
public:
	I8080(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		BUSREQ = false;
		SID = true;
		initialize_output_signals(&outputs_busack);
		initialize_output_signals(&outputs_sod);
		d_mem = d_pic = d_io = NULL;
		d_mem_stored = d_io_stored = NULL;
		d_debugger = NULL;
		total_count = prev_total_count = 0;

		_HAS_I8085 = false;
		_HAS_I8080 = true;
		__FP200 = false;
		__CPU_START_ADDR = 0;
		cc_op = cc_op_8080;

		set_device_name(_T("i8080 CPU"));
	}
	~I8080() {}

	void initialize() override;
	void reset() override;
	int __FASTCALL run(int clock) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int ch) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit) override;
	uint32_t get_pc() override
	{
		return prevPC;
	}
	uint32_t get_next_pc() override
	{
		return PC;
	}
//#ifdef USE_DEBUGGER
	bool is_cpu() override
	{
		return true;
	}
	bool is_debugger_available() override
	{
		return true;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask() override
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask() override
	{
		return 0xffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_debug_data8(uint32_t addr) override;
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_debug_io8(uint32_t addr) override;
	bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0) override;
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
	void set_context_intr(DEVICE* device, uint32_t bit = 0xffffffff) override
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
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}

};


#endif
