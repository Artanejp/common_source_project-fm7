#ifndef __LIBNEWDEV_LIBCPUI86_IX86_CPUSTAT_H__
#define __LIBNEWDEV_LIBCPUI86_IX86_CPUSTAT_H__


/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/
/* I86 registers */
union i80286basicregs
{                   /* eight general registers */
	UINT16 w[8];    /* viewed as 16 bits registers */
	UINT8  b[16];   /* or as 8 bit registers */
};

class DEVICE;
class EMU;
class DEBUGGER;
struct i80286_state
{
	i80286basicregs regs;
	UINT32  amask;          /* address mask */
	UINT32  pc;
	UINT32  prevpc;

	UINT16 ip;
	UINT32 sp;

	UINT16  flags;
	UINT16  msw;
	UINT32  base[4];
	UINT16  sregs[4];
	UINT16  limit[4];
	UINT8 rights[4];
	bool valid[4];
	struct {
		UINT32 base;
		UINT16 limit;
	} gdtr, idtr;
	struct {
		UINT16 sel;
		UINT32 base;
		UINT16 limit;
		UINT8 rights;
	} ldtr, tr;
	
	DEVICE *pic;
	DEVICE *program;
	DEVICE *io;
//#ifdef I86_PSEUDO_BIOS
	DEVICE *bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	DEVICE *dma;
//#endif
//#ifdef USE_DEBUGGER
	EMU *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
//#endif
	INT32   AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal; /* 0 or non-0 valued flags */
	UINT8   ParityVal;
	UINT8   TF, IF;     /* 0 or 1 valued flags */
	UINT8   MF;         /* V30 mode flag */
	
	UINT8 int_vector;
	INT8    nmi_state;
	INT8    irq_state;
	INT8    test_state;
	UINT8 rep_in_progress;
	INT32   extra_cycles;       /* extra cycles for interrupts */

	int halted;         /* Is the CPU halted ? */
	int busreq;
	int trap_level;
	int shutdown;

	int icount;
	char seg_prefix;
	UINT8   prefix_seg;
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
	UINT8 ea_seg;   /* effective segment of the address */
};
#define i8086_state i80286_state
#define i8086basicregs i80286basicregs

struct i80x86_timing
{
	int     id;

	UINT8   exception, iret;                                /* exception, IRET */
	UINT8   int3, int_imm, into_nt, into_t;                 /* INTs */
	UINT8   override;                                       /* segment overrides */
	UINT8   flag_ops, lahf, sahf;                           /* flag operations */
	UINT8   aaa, aas, aam, aad;                             /* arithmetic adjusts */
	UINT8   daa, das;                                       /* decimal adjusts */
	UINT8   cbw, cwd;                                       /* sign extension */
	UINT8   hlt, load_ptr, lea, nop, wait, xlat;            /* misc */

	UINT8   jmp_short, jmp_near, jmp_far;                   /* direct JMPs */
	UINT8   jmp_r16, jmp_m16, jmp_m32;                      /* indirect JMPs */
	UINT8   call_near, call_far;                            /* direct CALLs */
	UINT8   call_r16, call_m16, call_m32;                   /* indirect CALLs */
	UINT8   ret_near, ret_far, ret_near_imm, ret_far_imm;   /* returns */
	UINT8   jcc_nt, jcc_t, jcxz_nt, jcxz_t;                 /* conditional JMPs */
	UINT8   loop_nt, loop_t, loope_nt, loope_t;             /* loops */

	UINT8   in_imm8, in_imm16, in_dx8, in_dx16;             /* port reads */
	UINT8   out_imm8, out_imm16, out_dx8, out_dx16;         /* port writes */

	UINT8   mov_rr8, mov_rm8, mov_mr8;                      /* move, 8-bit */
	UINT8   mov_ri8, mov_mi8;                               /* move, 8-bit immediate */
	UINT8   mov_rr16, mov_rm16, mov_mr16;                   /* move, 16-bit */
	UINT8   mov_ri16, mov_mi16;                             /* move, 16-bit immediate */
	UINT8   mov_am8, mov_am16, mov_ma8, mov_ma16;           /* move, AL/AX memory */
	UINT8   mov_sr, mov_sm, mov_rs, mov_ms;                 /* move, segment registers */
	UINT8   xchg_rr8, xchg_rm8;                             /* exchange, 8-bit */
	UINT8   xchg_rr16, xchg_rm16, xchg_ar16;                /* exchange, 16-bit */

	UINT8   push_r16, push_m16, push_seg, pushf;            /* pushes */
	UINT8   pop_r16, pop_m16, pop_seg, popf;                /* pops */

	UINT8   alu_rr8, alu_rm8, alu_mr8;                      /* ALU ops, 8-bit */
	UINT8   alu_ri8, alu_mi8, alu_mi8_ro;                   /* ALU ops, 8-bit immediate */
	UINT8   alu_rr16, alu_rm16, alu_mr16;                   /* ALU ops, 16-bit */
	UINT8   alu_ri16, alu_mi16, alu_mi16_ro;                /* ALU ops, 16-bit immediate */
	UINT8   alu_r16i8, alu_m16i8, alu_m16i8_ro;             /* ALU ops, 16-bit w/8-bit immediate */
	UINT8   mul_r8, mul_r16, mul_m8, mul_m16;               /* MUL */
	UINT8   imul_r8, imul_r16, imul_m8, imul_m16;           /* IMUL */
	UINT8   div_r8, div_r16, div_m8, div_m16;               /* DIV */
	UINT8   idiv_r8, idiv_r16, idiv_m8, idiv_m16;           /* IDIV */
	UINT8   incdec_r8, incdec_r16, incdec_m8, incdec_m16;   /* INC/DEC */
	UINT8   negnot_r8, negnot_r16, negnot_m8, negnot_m16;   /* NEG/NOT */

	UINT8   rot_reg_1, rot_reg_base, rot_reg_bit;           /* reg shift/rotate */
	UINT8   rot_m8_1, rot_m8_base, rot_m8_bit;              /* m8 shift/rotate */
	UINT8   rot_m16_1, rot_m16_base, rot_m16_bit;           /* m16 shift/rotate */

	UINT8   cmps8, rep_cmps8_base, rep_cmps8_count;         /* CMPS 8-bit */
	UINT8   cmps16, rep_cmps16_base, rep_cmps16_count;      /* CMPS 16-bit */
	UINT8   scas8, rep_scas8_base, rep_scas8_count;         /* SCAS 8-bit */
	UINT8   scas16, rep_scas16_base, rep_scas16_count;      /* SCAS 16-bit */
	UINT8   lods8, rep_lods8_base, rep_lods8_count;         /* LODS 8-bit */
	UINT8   lods16, rep_lods16_base, rep_lods16_count;      /* LODS 16-bit */
	UINT8   stos8, rep_stos8_base, rep_stos8_count;         /* STOS 8-bit */
	UINT8   stos16, rep_stos16_base, rep_stos16_count;      /* STOS 16-bit */
	UINT8   movs8, rep_movs8_base, rep_movs8_count;         /* MOVS 8-bit */
	UINT8   movs16, rep_movs16_base, rep_movs16_count;      /* MOVS 16-bit */

	void *  check1;                                         /* marker to make sure we line up */

	UINT8   ins8, rep_ins8_base, rep_ins8_count;            /* (80186) INS 8-bit */
	UINT8   ins16, rep_ins16_base, rep_ins16_count;         /* (80186) INS 16-bit */
	UINT8   outs8, rep_outs8_base, rep_outs8_count;         /* (80186) OUTS 8-bit */
	UINT8   outs16, rep_outs16_base, rep_outs16_count;      /* (80186) OUTS 16-bit */
	UINT8   push_imm, pusha, popa;                          /* (80186) PUSH immediate, PUSHA/POPA */
	UINT8   imul_rri8, imul_rmi8;                           /* (80186) IMUL immediate 8-bit */
	UINT8   imul_rri16, imul_rmi16;                         /* (80186) IMUL immediate 16-bit */
	UINT8   enter0, enter1, enter_base, enter_count, leave; /* (80186) ENTER/LEAVE */
	UINT8   bound;                                          /* (80186) BOUND */

	void *  check2;                                         /* marker to make sure we line up */
};

#endif
