/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ MC6800 ]
*/

#ifndef _MC6800_H_ 
#define _MC6800_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

//#if defined(HAS_MC6801) || defined(HAS_HD6301)
//#define SIG_MC6801_PORT_1	0
//#define SIG_MC6801_PORT_2	1
//#define SIG_MC6801_PORT_3	2
//#define SIG_MC6801_PORT_4	3
//#define SIG_MC6801_PORT_3_SC1	4
//#define SIG_MC6801_PORT_3_SC2	5
//#define SIG_MC6801_SIO_RECV	6

//class FIFO;
//#endif

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class MC6800 : public DEVICE
{
private:

protected:
	DEVICE *d_mem;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
//#endif
	static const uint8_t flags8i[256];
	static const uint8_t flags8d[256];
#define XX 5 // invalid opcode unknown cc
	const uint8_t cycles[256] = {
//#if defined(HAS_MC6800)
		XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
		2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
		7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
		6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
		2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
		3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
		5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5,XX,XX, 6, 7,
		4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4,XX,XX, 5, 6
	};
#undef XX // invalid opcode unknown cc
	
	pair32_t pc;
	uint16_t prevpc;
	pair32_t sp;
	pair32_t ix;
	pair32_t acc_d;
	pair32_t ea;
	
	uint8_t cc;
	int wai_state;
	int int_state;
	

	uint64_t total_icount;
	uint64_t prev_total_icount;

	int icount;
	bool one_more_insn;
	
	virtual uint32_t RM(uint32_t Addr);
	virtual void WM(uint32_t Addr, uint32_t Value);
	uint32_t RM16(uint32_t Addr);
	void WM16(uint32_t Addr, pair32_t *p);
	
	
	virtual void increment_counter(int amount);
	virtual void run_one_opecode();
	void enter_interrupt(uint16_t irq_vector);
	virtual void insn(uint8_t code);
	
	void aba();
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

	void eora_di();
	void eora_ex();
	void eora_im();
	void eora_ix();
	void eorb_di();
	void eorb_ex();
	void eorb_im();
	void eorb_ix();
	virtual void illegal();
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
	void neg_ex();
	void neg_ix();
	void nega();
	void negb();
	void nop();

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
	void pula();
	void pulb();
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
	void sta_di();
	void sta_ex();
	void sta_im();
	void sta_ix();
	void stb_di();
	void stb_ex();
	void stb_im();
	void stb_ix();
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
	void swi();
	void tab();
	void tap();
	void tba();

	void tpa();
	void tst_ex();
	void tst_ix();
	void tsta();
	void tstb();
	void tsx();
	void txs();
	void wai();
	unsigned Dasm680x(int subtype, _TCHAR *buf, unsigned pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol);

	bool __USE_DEBUGGER;
public:
	MC6800(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = NULL;
		d_debugger = NULL;
		d_mem_stored = NULL;
		__USE_DEBUGGER = false;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
//		for(int i = 0; i < 4; i++) {
//			initialize_output_signals(&port[i].outputs);
//			port[i].wreg = port[i].rreg = 0;//0xff;
//		}
//		initialize_output_signals(&outputs_sio);
//#endif
//#if defined(HAS_MC6801)
//		set_device_name(_T("MC6801 MPU"));
//#elif defined(HAS_HD6301)
//		set_device_name(_T("HD6301 MPU"));
//#else
		set_device_name(_T("MC6800 MPU"));
//#endif
	}
	~MC6800() {}
	
	// common functions
	virtual void initialize();
	virtual void reset();
	virtual int run(int clock);
	
	virtual void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc()
	{
		return prevpc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
//#ifdef USE_DEBUGGER
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
	// implement 16bit/32bit functions because this cpu is big endian
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_data32(uint32_t addr, uint32_t data);
	uint32_t read_debug_data32(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
//#endif
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
//#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
//#endif
};

#endif

