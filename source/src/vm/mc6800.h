/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ MC6800 ]
*/

#ifndef _MC6800_H_ 
#define _MC6800_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#if defined(HAS_MC6801) || defined(HAS_HD6301)
#define SIG_MC6801_PORT_1	0
#define SIG_MC6801_PORT_2	1
#define SIG_MC6801_PORT_3	2
#define SIG_MC6801_PORT_4	3
#define SIG_MC6801_PORT_3_SC1	4
#define SIG_MC6801_PORT_3_SC2	5
#define SIG_MC6801_SIO_RECV	6

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class FIFO;
#endif

class MC6800 : public DEVICE
{
private:
	DEVICE *d_mem;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
#endif
	
	pair pc;
	uint16 prevpc;
	pair sp;
	pair ix;
	pair acc_d;
	pair ea;
	
	uint8 cc;
	int wai_state;
	int int_state;
	
	int icount;
	
	uint32 RM(uint32 Addr);
	void WM(uint32 Addr, uint32 Value);
	uint32 RM16(uint32 Addr);
	void WM16(uint32 Addr, pair *p);
	
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	// data
	typedef struct {
		uint8 wreg;
		uint8 rreg;
		uint8 ddr;
		uint8 latched_data;
		bool latched;
		// output signals
		outputs_t outputs;
		bool first_write;
	} port_t;
	port_t port[4];
	
	uint8 p3csr;
	bool p3csr_is3_flag_read;
	bool sc1_state;
	bool sc2_state;
	
	// timer
	pair counter;
	pair output_compare;
	pair timer_over;
	uint8 tcsr;
	uint8 pending_tcsr;
	uint16 input_capture;
#ifdef HAS_HD6301
	uint16 latch09;
#endif
	uint32 timer_next;
	
	// serial i/o
	outputs_t outputs_sio;
	FIFO *recv_buffer;
	uint8 trcsr, rdr, tdr;
	bool trcsr_read_tdre, trcsr_read_orfe, trcsr_read_rdrf;
	uint8 rmcr;
	int sio_counter;
	
	// memory controller
	uint8 ram_ctrl;
	uint8 ram[128];
	
	uint32 mc6801_io_r(uint32 offset);
	void mc6801_io_w(uint32 offset, uint32 data);
	void increment_counter(int amount);
#endif
	
	void run_one_opecode();
	void enter_interrupt(uint16 irq_vector);
	void insn(uint8 code);
	
	void aba();
	void abx();
	void adca_di();
	void adca_ex();
	void adca_im();
	void adca_ix();
	void adcb_di();
	void adcb_ex();
	void adcb_im();
	void adcb_ix();
	void adda_di();
	void adda_ex();
	void adda_im();
	void adda_ix();
	void addb_di();
	void addb_ex();
	void addb_im();
	void addb_ix();
	void addd_di();
	void addd_ex();
	void addd_im();
	void addd_ix();
	void adx_ex();
	void adx_im();
	void aim_di();
	void aim_ix();
	void nim_ix();
	void anda_di();
	void anda_ex();
	void anda_im();
	void anda_ix();
	void andb_di();
	void andb_ex();
	void andb_im();
	void andb_ix();
	void asl_ex();
	void asl_ix();
	void asla();
	void aslb();
	void asld();
	void asr_ex();
	void asr_ix();
	void asra();
	void asrb();
	void bcc();
	void bcs();
	void beq();
	void bge();
	void bgt();
	void bhi();
	void bita_di();
	void bita_ex();
	void bita_im();
	void bita_ix();
	void bitb_di();
	void bitb_ex();
	void bitb_im();
	void bitb_ix();
	void ble();
	void bls();
	void blt();
	void bmi();
	void bne();
	void bpl();
	void bra();
	void brn();
	void bsr();
	void bvc();
	void bvs();
	void cba();
	void clc();
	void cli();
	void clr_ex();
	void clr_ix();
	void clra();
	void clrb();
	void clv();
	void cmpa_di();
	void cmpa_ex();
	void cmpa_im();
	void cmpa_ix();
	void cmpb_di();
	void cmpb_ex();
	void cmpb_im();
	void cmpb_ix();
	void cmpx_di();
	void cmpx_ex();
	void cmpx_im();
	void cmpx_ix();
	void com_ex();
	void com_ix();
	void coma();
	void comb();
	void daa();
	void dec_ex();
	void dec_ix();
	void deca();
	void decb();
	void des();
	void dex();
	void eim_di();
	void eim_ix();
	void xim_ix();
	void eora_di();
	void eora_ex();
	void eora_im();
	void eora_ix();
	void eorb_di();
	void eorb_ex();
	void eorb_im();
	void eorb_ix();
	void illegal();
	void inc_ex();
	void inc_ix();
	void inca();
	void incb();
	void ins();
	void inx();
	void jmp_ex();
	void jmp_ix();
	void jsr_di();
	void jsr_ex();
	void jsr_ix();
	void lda_di();
	void lda_ex();
	void lda_im();
	void lda_ix();
	void ldb_di();
	void ldb_ex();
	void ldb_im();
	void ldb_ix();
	void ldd_di();
	void ldd_ex();
	void ldd_im();
	void ldd_ix();
	void lds_di();
	void lds_ex();
	void lds_im();
	void lds_ix();
	void ldx_di();
	void ldx_ex();
	void ldx_im();
	void ldx_ix();
	void lsr_ex();
	void lsr_ix();
	void lsra();
	void lsrb();
	void lsrd();
	void mul();
	void neg_ex();
	void neg_ix();
	void nega();
	void negb();
	void nop();
	void oim_di();
	void oim_ix();
	void oim_ix_mb8861();
	void ora_di();
	void ora_ex();
	void ora_im();
	void ora_ix();
	void orb_di();
	void orb_ex();
	void orb_im();
	void orb_ix();
	void psha();
	void pshb();
	void pshx();
	void pula();
	void pulb();
	void pulx();
	void rol_ex();
	void rol_ix();
	void rola();
	void rolb();
	void ror_ex();
	void ror_ix();
	void rora();
	void rorb();
	void rti();
	void rts();
	void sba();
	void sbca_di();
	void sbca_ex();
	void sbca_im();
	void sbca_ix();
	void sbcb_di();
	void sbcb_ex();
	void sbcb_im();
	void sbcb_ix();
	void sec();
	void sei();
	void sev();
	void slp();
	void sta_di();
	void sta_ex();
	void sta_im();
	void sta_ix();
	void stb_di();
	void stb_ex();
	void stb_im();
	void stb_ix();
	void std_di();
	void std_ex();
	void std_im();
	void std_ix();
	void sts_di();
	void sts_ex();
	void sts_im();
	void sts_ix();
	void stx_di();
	void stx_ex();
	void stx_im();
	void stx_ix();
	void suba_di();
	void suba_ex();
	void suba_im();
	void suba_ix();
	void subb_di();
	void subb_ex();
	void subb_im();
	void subb_ix();
	void subd_di();
	void subd_ex();
	void subd_im();
	void subd_ix();
	void swi();
	void tab();
	void tap();
	void tba();
	void tim_di();
	void tim_ix();
	void tmm_ix();
	void tpa();
	void tst_ex();
	void tst_ix();
	void tsta();
	void tstb();
	void tsx();
	void txs();
	void undoc1();
	void undoc2();
	void wai();
	void xgdx();
	void cpx_di();
	void cpx_ex();
	void cpx_im();
	void cpx_ix();
	
public:
	MC6800(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#if defined(HAS_MC6801) || defined(HAS_HD6301)
		for(int i = 0; i < 4; i++) {
			init_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		init_output_signals(&outputs_sio);
#endif
	}
	~MC6800() {}
	
	// common functions
	void initialize();
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	void release();
#endif
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 get_pc()
	{
		return prevpc;
	}
	uint32 get_next_pc()
	{
		return pc.w.l;
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
	// implement 16bit/32bit functions because this cpu is big endian
	void debug_write_data16(uint32 addr, uint32 data);
	uint32 debug_read_data16(uint32 addr);
	void debug_write_data32(uint32 addr, uint32 data);
	uint32 debug_read_data32(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer);
	int debug_dasm(uint32 pc, _TCHAR *buffer);
#endif
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	void set_context_port1(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port2(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_port3(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift);
	}
	void set_context_port4(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift);
	}
	void set_context_sio(DEVICE* device, int id)
	{
		register_output_signal(&outputs_sio, device, id, 0xff);
	}
#endif
};

#endif

