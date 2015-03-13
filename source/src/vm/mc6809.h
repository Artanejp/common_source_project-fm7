/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

#ifndef _MC6809_H_ 
#define _MC6809_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"
#include "../fileio.h"

class MC6809 : public DEVICE
{
private:
	// context
	DEVICE *d_mem;
	
	// registers
	pair pc; 	/* Program counter */
	pair ppc;	/* Previous program counter */
	pair acc;	/* Accumulator a and b */
	pair dp;	/* Direct Page register (page in MSB) */
	pair u, s;	/* Stack pointers */
	pair x, y;	/* Index registers */
	volatile uint8 cc;
	pair ea;	/* effective address */
	
	uint32 int_state;
	int icount;
	int extra_icount;
	inline uint32 RM16(uint32 Addr);
	inline void WM16(uint32 Addr, pair *p);
	
	// opcodes
	void run_one_opecode();
	void op(uint8 ireg);
	void fetch_effective_address();
  public:	
	 volatile void abx();
	 volatile void adca_di();
	 volatile void adca_ex();
	 volatile void adca_im();
	 volatile void adca_ix();
	 volatile void adcb_di();
	 volatile void adcb_ex();
	 volatile void adcb_im();
	 volatile void adcb_ix();
	 volatile void adda_di();
	 volatile void adda_ex();
	 volatile void adda_im();
	 volatile void adda_ix();
	 volatile void addb_di();
	 volatile void addb_ex();
	 volatile void addb_im();
	 volatile void addb_ix();
	 volatile void addd_di();
	 volatile void addd_ex();
	 volatile void addd_im();
	 volatile void addd_ix();
	 volatile void anda_di();
	 volatile void anda_ex();
	 volatile void anda_im();
	 volatile void anda_ix();
	 volatile void andb_di();
	 volatile void andb_ex();
	 volatile void andb_im();
	 volatile void andb_ix();
	 volatile void andcc();
	 volatile void asla();
	 volatile void aslb();
	 volatile void aslcc_in();
	 volatile void asl_di();
	 volatile void asl_ex();
	 volatile void asl_ix();
	 volatile void asra();
	 volatile void asrb();
	 volatile void asr_di();
	 volatile void asr_ex();
	 volatile void asr_ix();
	 volatile void bcc();
	 volatile void bcs();
	 volatile void beq();
	 volatile void bge();
	 volatile void bgt();
	 volatile void bhi();
	 volatile void bita_di();
	 volatile void bita_ex();
	 volatile void bita_im();
	 volatile void bita_ix();
	 volatile void bitb_di();
	 volatile void bitb_ex();
	 volatile void bitb_im();
	 volatile void bitb_ix();
	 volatile void ble();
	 volatile void bls();
	 volatile void blt();
	 volatile void bmi();
	 volatile void bne();
	 volatile void bpl();
	 volatile void bra();
	 volatile void brn();
	 volatile void bsr();
	 volatile void bvc();
	 volatile void bvs();
	 volatile void clca();
	 volatile void clcb();
	 volatile void clra();
	 volatile void clrb();
	 volatile void clr_di();
	 volatile void clr_ex();
	 volatile void clr_ix();
	 volatile void cmpa_di();
	 volatile void cmpa_ex();
	 volatile void cmpa_im();
	 volatile void cmpa_ix();
	 volatile void cmpb_di();
	 volatile void cmpb_ex();
	 volatile void cmpb_im();
	 volatile void cmpb_ix();
	 volatile void cmpd_di();
	 volatile void cmpd_ex();
	 volatile void cmpd_im();
	 volatile void cmpd_ix();
	 volatile void cmps_di();
	 volatile void cmps_ex();
	 volatile void cmps_im();
	 volatile void cmps_ix();
	 volatile void cmpu_di();
	 volatile void cmpu_ex();
	 volatile void cmpu_im();
	 volatile void cmpu_ix();
	 volatile void cmpx_di();
	 volatile void cmpx_ex();
	 volatile void cmpx_im();
	 volatile void cmpx_ix();
	 volatile void cmpy_di();
	 volatile void cmpy_ex();
	 volatile void cmpy_im();
	 volatile void cmpy_ix();
	 volatile void coma();
	 volatile void comb();
	 volatile void com_di();
	 volatile void com_ex();
	 volatile void com_ix();
	 volatile void cwai();
	 volatile void daa();
	 volatile void dcca();
	 volatile void dccb();
	 volatile void dcc_di();
	 volatile void dcc_ex();
	 volatile void dcc_ix();
	 volatile void deca();
	 volatile void decb();
	 volatile void dec_di();
	 volatile void dec_ex();
	 volatile void dec_ix();
	 volatile void eora_di();
	 volatile void eora_ex();
	 volatile void eora_im();
	 volatile void eora_ix();
	 volatile void eorb_di();
	 volatile void eorb_ex();
	 volatile void eorb_im();
	 volatile void eorb_ix();
	 volatile void exg();
	 volatile void flag8_im();
	 volatile void flag16_im();
	 volatile void illegal();
	 volatile void inca();
	 volatile void incb();
	 volatile void inc_di();
	 volatile void inc_ex();
	 volatile void inc_ix();
	 volatile void jmp_di();
	 volatile void jmp_ex();
	 volatile void jmp_ix();
	 volatile void jsr_di();
	 volatile void jsr_ex();
	 volatile void jsr_ix();
	 volatile void lbcc();
	 volatile void lbcs();
	 volatile void lbeq();
	 volatile void lbge();
	 volatile void lbgt();
	 volatile void lbhi();
	 volatile void lble();
	 volatile void lbls();
	 volatile void lblt();
	 volatile void lbmi();
	 volatile void lbne();
	 volatile void lbpl();
	 volatile void lbra();
	 volatile void lbrn();
	 volatile void lbsr();
	 volatile void lbvc();
	 volatile void lbvs();
	 volatile void lda_di();
	 volatile void lda_ex();
	 volatile void lda_im();
	 volatile void lda_ix();
	 volatile void ldb_di();
	 volatile void ldb_ex();
	 volatile void ldb_im();
	 volatile void ldb_ix();
	 volatile void ldd_di();
	 volatile void ldd_ex();
	 volatile void ldd_im();
	 volatile void ldd_ix();
	 volatile void lds_di();
	 volatile void lds_ex();
	 volatile void lds_im();
	 volatile void lds_ix();
	 volatile void ldu_di();
	 volatile void ldu_ex();
	 volatile void ldu_im();
	 volatile void ldu_ix();
	 volatile void ldx_di();
	 volatile void ldx_ex();
	 volatile void ldx_im();
	 volatile void ldx_ix();
	 volatile void ldy_di();
	 volatile void ldy_ex();
	 volatile void ldy_im();
	 volatile void ldy_ix();
	 volatile void leas();
	 volatile void leau();
	 volatile void leax();
	 volatile void leay();
	 volatile void lsra();
	 volatile void lsrb();
	 volatile void lsr_di();
	 volatile void lsr_ex();
	 volatile void lsr_ix();
	 volatile void mul();
	 volatile void nega();
	 volatile void negb();
	 volatile void neg_di();
	 volatile void neg_ex();
	 volatile void neg_ix();
	 volatile void ngca();
	 volatile void ngcb();
	 volatile void ngc_di();
	 volatile void ngc_ex();
	 volatile void ngc_ix();
	 volatile void nop();
	 volatile void ora_di();
	 volatile void ora_ex();
	 volatile void ora_im();
	 volatile void ora_ix();
	 volatile void orb_di();
	 volatile void orb_ex();
	 volatile void orb_im();
	 volatile void orb_ix();
	 volatile void orcc();
	 volatile void pref10();
	 volatile void pref11();
	 volatile void pshs();
	 volatile void pshu();
	 volatile void puls();
	 volatile void pulu();
	 volatile void rola();
	 volatile void rolb();
	 volatile void rol_di();
	 volatile void rol_ex();
	 volatile void rol_ix();
	 volatile void rora();
	 volatile void rorb();
	 volatile void ror_di();
	 volatile void ror_ex();
	 volatile void ror_ix();
	 volatile void rst();
	 volatile void rti();	
	 volatile void rts();	
	 volatile void sbca_di();
	 volatile void sbca_ex();
	 volatile void sbca_im();
	 volatile void sbca_ix();
	 volatile void sbcb_di();
	 volatile void sbcb_ex();
	 volatile void sbcb_im();
	 volatile void sbcb_ix();
	 volatile void sex();
	 volatile void sta_di();
	 volatile void sta_ex();
	 volatile void sta_im();
	 volatile void sta_ix();
	 volatile void stb_di();
	 volatile void stb_ex();
	 volatile void stb_im();
	 volatile void stb_ix();
	 volatile void std_di();
	 volatile void std_ex();
	 volatile void std_im();
	 volatile void std_ix();
	 volatile void sts_di();
	 volatile void sts_ex();
	 volatile void sts_im();
	 volatile void sts_ix();
	 volatile void stu_di();
	 volatile void stu_ex();
	 volatile void stu_im();
	 volatile void stu_ix();
	 volatile void stx_di();
	 volatile void stx_ex();
	 volatile void stx_im();
	 volatile void stx_ix();
	 volatile void sty_di();
	 volatile void sty_ex();
	 volatile void sty_im();
	 volatile void sty_ix();
	 volatile void suba_di();
	 volatile void suba_ex();
	 volatile void suba_im();
	 volatile void suba_ix();
	 volatile void subb_di();
	 volatile void subb_ex();
	 volatile void subb_im();
	 volatile void subb_ix();
	 volatile void subd_di();
	 volatile void subd_ex();
	 volatile void subd_im();
	 volatile void subd_ix();
	 volatile void swi2();
	 volatile void swi3();
	 volatile void swi();
	 volatile void sync();
	 volatile void tfr();
	 volatile void trap();
	 volatile void tsta();
	 volatile void tstb();
	 volatile void tst_di();
	 volatile void tst_ex();
	 volatile void tst_ix();
	
public:
	MC6809(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MC6809() {}
	
	// common functions
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
        void save_state(FILEIO* state_fio);
        bool load_state(FILEIO* state_fio);
        void set_extra_clock(int clock)
        {
		                extra_icount += clock;
	}
        int get_extra_clock()
        {
	        return extra_icount;
	}
        uint32 get_pc()
	{
		return ppc.w.l;
	}
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
};

#endif

