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

class TMS9995 : public DEVICE
{
private:
	// contexts
	DEVICE *d_mem, *d_io;
	
	// clocks
	int count, period;
	// register
	uint16 WP, PC, prevPC, ST;
	uint8 RAM[256];
	// interrupt
	uint8 irq_level, int_state, int_latch;
	bool int_pending, int_enabled;
	// counter/timer
	uint16 dec_count, dec_interval;
	int dec_timer;
	bool dec_enabled;
	// status
	uint16 mode;
	uint8 lastparity;
	bool nmi, mid, idle;
	
	// memory functions
	uint16 RM16(uint16 addr);
	void WM16(uint16 addr, uint16 val);
	uint8 RM8(uint16 addr);
	void WM8(uint32 addr, uint8 val);
	inline uint16 FETCH16();
	
	// i/o functions
	uint16 IN8(int addr);
	void OUT8(uint16 addr, uint16 val);
	inline void EXTOUT8(uint16 addr);
	uint16 RCRU(uint16 addr, int bits);
	void WCRU(uint16 addr, int bits, uint16 val);
	
	// cpu internal control
	void set_irq_line(int irqline, bool state);
	void update_int();
	void update_dec();
	void contextswitch(uint16 addr);
	
	// opecode functions
	void run_one_opecode();
	void execute(uint16 op);
	void h0040(uint16 op);
	void h0100(uint16 op);
	void h0200(uint16 op);
	void h0400(uint16 op);
	void h0800(uint16 op);
	void h1000(uint16 op);
	void h2000(uint16 op);
	void xop(uint16 op);
	void ldcr_stcr(uint16 op);
	void h4000w(uint16 op);
	void h4000b(uint16 op);
	void illegal(uint16 op);
	uint16 decipheraddr(uint16 op);
	uint16 decipheraddrbyte(uint16 op);
	
	// status functions
	inline void setstat();
	inline void getstat();
	inline uint16 logical_right_shift(uint16 val, int c);
	inline int16 arithmetic_right_shift(int16 val, int c);
	inline void setst_lae(int16 val);
	inline void setst_byte_laep(int8 val);
	inline void setst_e(uint16 val, uint16 to);
	inline void setst_c_lae(uint16 to, uint16 val);
	inline int16 setst_add_laeco(int a, int b);
	inline int16 setst_sub_laeco(int a, int b);
	inline int8 setst_addbyte_laecop(int a, int b);
	inline int8 setst_subbyte_laecop(int a, int b);
	inline void setst_laeo(int16 val);
	inline uint16 setst_sra_laec(int16 a, uint16 c);
	inline uint16 setst_srl_laec(uint16 a,uint16 c);
	inline uint16 setst_src_laec(uint16 a,uint16 c);
	inline uint16 setst_sla_laeco(uint16 a, uint16 c);
	
public:
	TMS9995(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// init registers
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
	}
	~TMS9995() {}
	
	// common function
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 get_pc()
	{
		return prevPC;
	}
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
};

#endif
