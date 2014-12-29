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
	uint8 cc;
	pair ea;	/* effective address */
	
	uint8 int_state;
	int icount;
	
	inline uint32 RM16(uint32 Addr);
	inline void WM16(uint32 Addr, pair *p);
	
	// opcodes
	void run_one_opecode();
	void op(uint8 ireg);
	inline void fetch_effective_address();
	inline void abx();
	inline void adca_di();
	inline void adca_ex();
	inline void adca_im();
	inline void adca_ix();
	inline void adcb_di();
	inline void adcb_ex();
	inline void adcb_im();
	inline void adcb_ix();
	inline void adda_di();
	inline void adda_ex();
	inline void adda_im();
	inline void adda_ix();
	inline void addb_di();
	inline void addb_ex();
	inline void addb_im();
	inline void addb_ix();
	inline void addd_di();
	inline void addd_ex();
	inline void addd_im();
	inline void addd_ix();
	inline void anda_di();
	inline void anda_ex();
	inline void anda_im();
	inline void anda_ix();
	inline void andb_di();
	inline void andb_ex();
	inline void andb_im();
	inline void andb_ix();
	inline void andcc();
	inline void asla();
	inline void aslb();
	inline void asl_di();
	inline void asl_ex();
	inline void asl_ix();
	inline void asra();
	inline void asrb();
	inline void asr_di();
	inline void asr_ex();
	inline void asr_ix();
	inline void bcc();
	inline void bcs();
	inline void beq();
	inline void bge();
	inline void bgt();
	inline void bhi();
	inline void bita_di();
	inline void bita_ex();
	inline void bita_im();
	inline void bita_ix();
	inline void bitb_di();
	inline void bitb_ex();
	inline void bitb_im();
	inline void bitb_ix();
	inline void ble();
	inline void bls();
	inline void blt();
	inline void bmi();
	inline void bne();
	inline void bpl();
	inline void bra();
	inline void brn();
	inline void bsr();
	inline void bvc();
	inline void bvs();
	inline void clra();
	inline void clrb();
	inline void clr_di();
	inline void clr_ex();
	inline void clr_ix();
	inline void cmpa_di();
	inline void cmpa_ex();
	inline void cmpa_im();
	inline void cmpa_ix();
	inline void cmpb_di();
	inline void cmpb_ex();
	inline void cmpb_im();
	inline void cmpb_ix();
	inline void cmpd_di();
	inline void cmpd_ex();
	inline void cmpd_im();
	inline void cmpd_ix();
	inline void cmps_di();
	inline void cmps_ex();
	inline void cmps_im();
	inline void cmps_ix();
	inline void cmpu_di();
	inline void cmpu_ex();
	inline void cmpu_im();
	inline void cmpu_ix();
	inline void cmpx_di();
	inline void cmpx_ex();
	inline void cmpx_im();
	inline void cmpx_ix();
	inline void cmpy_di();
	inline void cmpy_ex();
	inline void cmpy_im();
	inline void cmpy_ix();
	inline void coma();
	inline void comb();
	inline void com_di();
	inline void com_ex();
	inline void com_ix();
	inline void cwai();
	inline void daa();
	inline void deca();
	inline void decb();
	inline void dec_di();
	inline void dec_ex();
	inline void dec_ix();
	inline void eora_di();
	inline void eora_ex();
	inline void eora_im();
	inline void eora_ix();
	inline void eorb_di();
	inline void eorb_ex();
	inline void eorb_im();
	inline void eorb_ix();
	inline void exg();
	inline void illegal();
	inline void inca();
	inline void incb();
	inline void inc_di();
	inline void inc_ex();
	inline void inc_ix();
	inline void jmp_di();
	inline void jmp_ex();
	inline void jmp_ix();
	inline void jsr_di();
	inline void jsr_ex();
	inline void jsr_ix();
	inline void lbcc();
	inline void lbcs();
	inline void lbeq();
	inline void lbge();
	inline void lbgt();
	inline void lbhi();
	inline void lble();
	inline void lbls();
	inline void lblt();
	inline void lbmi();
	inline void lbne();
	inline void lbpl();
	inline void lbra();
	inline void lbrn();
	inline void lbsr();
	inline void lbvc();
	inline void lbvs();
	inline void lda_di();
	inline void lda_ex();
	inline void lda_im();
	inline void lda_ix();
	inline void ldb_di();
	inline void ldb_ex();
	inline void ldb_im();
	inline void ldb_ix();
	inline void ldd_di();
	inline void ldd_ex();
	inline void ldd_im();
	inline void ldd_ix();
	inline void lds_di();
	inline void lds_ex();
	inline void lds_im();
	inline void lds_ix();
	inline void ldu_di();
	inline void ldu_ex();
	inline void ldu_im();
	inline void ldu_ix();
	inline void ldx_di();
	inline void ldx_ex();
	inline void ldx_im();
	inline void ldx_ix();
	inline void ldy_di();
	inline void ldy_ex();
	inline void ldy_im();
	inline void ldy_ix();
	inline void leas();
	inline void leau();
	inline void leax();
	inline void leay();
	inline void lsra();
	inline void lsrb();
	inline void lsr_di();
	inline void lsr_ex();
	inline void lsr_ix();
	inline void mul();
	inline void nega();
	inline void negb();
	inline void neg_di();
	inline void neg_ex();
	inline void neg_ix();
	inline void nop();
	inline void ora_di();
	inline void ora_ex();
	inline void ora_im();
	inline void ora_ix();
	inline void orb_di();
	inline void orb_ex();
	inline void orb_im();
	inline void orb_ix();
	inline void orcc();
	inline void pref10();
	inline void pref11();
	inline void pshs();
	inline void pshu();
	inline void puls();
	inline void pulu();
	inline void rola();
	inline void rolb();
	inline void rol_di();
	inline void rol_ex();
	inline void rol_ix();
	inline void rora();
	inline void rorb();
	inline void ror_di();
	inline void ror_ex();
	inline void ror_ix();
	inline void rti();
	inline void rts();
	inline void sbca_di();
	inline void sbca_ex();
	inline void sbca_im();
	inline void sbca_ix();
	inline void sbcb_di();
	inline void sbcb_ex();
	inline void sbcb_im();
	inline void sbcb_ix();
	inline void sex();
	inline void sta_di();
	inline void sta_ex();
	inline void sta_im();
	inline void sta_ix();
	inline void stb_di();
	inline void stb_ex();
	inline void stb_im();
	inline void stb_ix();
	inline void std_di();
	inline void std_ex();
	inline void std_im();
	inline void std_ix();
	inline void sts_di();
	inline void sts_ex();
	inline void sts_im();
	inline void sts_ix();
	inline void stu_di();
	inline void stu_ex();
	inline void stu_im();
	inline void stu_ix();
	inline void stx_di();
	inline void stx_ex();
	inline void stx_im();
	inline void stx_ix();
	inline void sty_di();
	inline void sty_ex();
	inline void sty_im();
	inline void sty_ix();
	inline void suba_di();
	inline void suba_ex();
	inline void suba_im();
	inline void suba_ix();
	inline void subb_di();
	inline void subb_ex();
	inline void subb_im();
	inline void subb_ix();
	inline void subd_di();
	inline void subd_ex();
	inline void subd_im();
	inline void subd_ix();
	inline void swi2();
	inline void swi3();
	inline void swi();
	inline void sync();
	inline void tfr();
	inline void tsta();
	inline void tstb();
	inline void tst_di();
	inline void tst_ex();
	inline void tst_ix();
	
public:
	MC6809(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MC6809() {}
	
	// common functions
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
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

