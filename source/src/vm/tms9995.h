/*
	Skelton for retropc emulator

	Origin : MAME TMS99xx Core
	Author : Takeda.Toshiya
	Date   : 2007.07.14 -

	[ TMS9995 ]
*/

#ifndef _TMS9995_
#define _TMS9995_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_TMS9995_NMI		0
#define SIG_TMS9995_INT1	1
#define SIG_TMS9995_INT4	2

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class TMS9995 : public DEVICE
{
private:
	// contexts
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_tmp, *d_io_tmp;
#endif
	
	// clocks
#ifdef USE_DEBUGGER
	uint64_t total_count;
	uint64_t prev_total_count;
#endif
	int count, period;
	// register
	uint16_t WP, PC, prevPC, ST;
	uint8_t RAM[256];
	// interrupt
	uint8_t irq_level, int_state, int_latch;
	bool int_pending, int_enabled;
	// counter/timer
	uint16_t dec_count, dec_interval;
	int dec_timer;
	bool dec_enabled;
	// status
	uint16_t mode;
	uint8_t lastparity;
	bool nmi, mid, idle;
	
	// memory functions
	uint16_t RM16(uint16_t addr);
	void WM16(uint16_t addr, uint16_t val);
	uint8_t RM8(uint16_t addr);
	void WM8(uint32_t addr, uint8_t val);
	inline uint16_t FETCH16();
	
	// i/o functions
	uint16_t IN8(int addr);
	void OUT8(uint16_t addr, uint16_t val);
	inline void EXTOUT8(uint16_t addr);
	uint16_t RCRU(uint16_t addr, int bits);
	void WCRU(uint16_t addr, int bits, uint16_t val);
	
	// cpu internal control
	void set_irq_line(int irqline, bool state);
	void update_int();
	void update_dec();
	void contextswitch(uint16_t addr);
	
	// opecode functions
	void run_one_opecode();
#ifdef USE_DEBUGGER
	void run_one_opecode_tmp();
#endif
	void execute(uint16_t op);
	void h0040(uint16_t op);
	void h0100(uint16_t op);
	void h0200(uint16_t op);
	void h0400(uint16_t op);
	void h0800(uint16_t op);
	void h1000(uint16_t op);
	void h2000(uint16_t op);
	void xop(uint16_t op);
	void ldcr_stcr(uint16_t op);
	void h4000w(uint16_t op);
	void h4000b(uint16_t op);
	void illegal(uint16_t op);
	uint16_t decipheraddr(uint16_t op);
	uint16_t decipheraddrbyte(uint16_t op);
	
	// status functions
	inline void setstat();
	inline void getstat();
	inline uint16_t logical_right_shift(uint16_t val, int c);
	inline int16_t arithmetic_right_shift(int16_t val, int c);
	inline void setst_lae(int16_t val);
	inline void setst_byte_laep(int8_t val);
	inline void setst_e(uint16_t val, uint16_t to);
	inline void setst_c_lae(uint16_t to, uint16_t val);
	inline int16_t setst_add_laeco(int a, int b);
	inline int16_t setst_sub_laeco(int a, int b);
	inline int8_t setst_addbyte_laecop(int a, int b);
	inline int8_t setst_subbyte_laecop(int a, int b);
	inline void setst_laeo(int16_t val);
	inline uint16_t setst_sra_laec(int16_t a, uint16_t c);
	inline uint16_t setst_srl_laec(uint16_t a,uint16_t c);
	inline uint16_t setst_src_laec(uint16_t a,uint16_t c);
	inline uint16_t setst_sla_laeco(uint16_t a, uint16_t c);
	
public:
	TMS9995(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// init registers
#ifdef USE_DEBUGGER
		total_count = prev_total_count = 0;
#endif
		WP = PC = ST = 0;
		memset(RAM, 0, sizeof(RAM));
		irq_level = int_state = int_latch = 0;
		int_pending = false;
		int_enabled = true;
		dec_count = dec_interval = 0;
		dec_timer = 0;
		dec_enabled = false;
		mode = 0;
		lastparity = 0;
		nmi = mid = idle = false;
		set_device_name(_T("TMS9995 CPU"));
	}
	~TMS9995() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
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
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif
