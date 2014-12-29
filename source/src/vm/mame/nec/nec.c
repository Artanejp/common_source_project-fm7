/****************************************************************************

    NEC V20/V30/V33 emulator

    ---------------------------------------------

    V20 = uPD70108 = 8-bit data bus @ 5MHz or 8MHz
    V20HL = uPD70108H = V20 with EMS support (24-bit address bus)

    V25 = uPD70320 = V20 with on-chip features:
            - 256 bytes on-chip RAM
            - 8 register banks
            - 4-bit input port
            - 20-bit I/O port
            - 2 channel serial interface
            - interrupt controller
            - 2 channel DMA controller
            - 2 channel 16-bit timer
            - new instructions: BTCLR, RETRBI, STOP, BRKCS, TSKSW,
                                MOVSPA, MOVSPB

    V25+ = uPD70325 = V25 @ 8MHz or 10MHz plus changes:
            - faster DMA
            - improved serial interface

    ---------------------------------------------

    V30 = uPD70116 = 16-bit data bus version of V20
    V30HL = uPD70116H = 16-bit data bus version of V20HL
    V30MX = V30HL with separate address and data busses

    V35 = uPD70330 = 16-bit data bus version of V25

    V35+ = uPD70335 = 16-bit data bus version of V25+

    ---------------------------------------------

    V40 = uPD70208 = 8-bit data bus @ 10MHz
    V40HL = uPD70208H = V40 with support up to 20Mhz

    ---------------------------------------------

    V50 = uPD70216 = 16-bit data bus version of V40
    V50HL = uPD70216H = 16-bit data bus version of V40HL

    ---------------------------------------------

    V41 = uPD70270

    V51 = uPD70280



    V33A = uPD70136A

    V53A = uPD70236A



    Instruction differences:
        V20, V30, V40, V50 have dedicated emulation instructions
            (BRKEM, RETEM, CALLN)

        V33A, V53A has dedicated address mode instructions
            (BRKXA, RETXA)



    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
    on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
    on the i286 emulator by Fabrice Frances which had initial work based on
    David Hedley's pcemu(!).

    This new core features 99% accurate cycle counts for each processor,
    there are still some complex situations where cycle counts are wrong,
    typically where a few instructions have differing counts for odd/even
    source and odd/even destination memory operands.

    Flag settings are also correct for the NEC processors rather than the
    I86 versions.

    Changelist:

    22/02/2003:
        Removed cycle counts from memory accesses - they are certainly wrong,
        and there is already a memory access cycle penalty in the opcodes
        using them.

        Fixed save states.

        Fixed ADJBA/ADJBS/ADJ4A/ADJ4S flags/return values for all situations.
        (Fixes bugs in Geostorm and Thunderblaster)

        Fixed carry flag on NEG (I thought this had been fixed circa Mame 0.58,
        but it seems I never actually submitted the fix).

        Fixed many cycle counts in instructions and bug in cycle count
        macros (odd word cases were testing for odd instruction word address
        not data address).

    Todo!
        Double check cycle timing is 100%.

****************************************************************************/

//#include "emu.h"
//#include "debugger.h"

//typedef UINT8 BOOLEAN;
//typedef UINT8 BYTE;
//typedef UINT16 WORD;
//typedef UINT32 DWORD;

#include "nec.h"
#include "necpriv.h"

//extern int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, const nec_config *config);

INLINE void prefetch(nec_state_t *nec_state)
{
	nec_state->prefetch_count--;
}

static void do_prefetch(nec_state_t *nec_state, int previous_ICount)
{
	int diff = previous_ICount - (int) nec_state->icount;

	/* The implementation is not accurate, but comes close.
     * It does not respect that the V30 will fetch two bytes
     * at once directly, but instead uses only 2 cycles instead
     * of 4. There are however only very few sources publicly
     * available and they are vague.
     */
	while (nec_state->prefetch_count<0)
	{
		nec_state->prefetch_count++;
		if (diff>nec_state->prefetch_cycles)
			diff -= nec_state->prefetch_cycles;
		else
			nec_state->icount -= nec_state->prefetch_cycles;
	}

	if (nec_state->prefetch_reset)
	{
		nec_state->prefetch_count = 0;
		nec_state->prefetch_reset = 0;
		return;
	}

	while (diff>=nec_state->prefetch_cycles && nec_state->prefetch_count < nec_state->prefetch_size)
	{
		diff -= nec_state->prefetch_cycles;
		nec_state->prefetch_count++;
	}

}

INLINE UINT8 fetch(nec_state_t *nec_state)
{
	prefetch(nec_state);
	return nec_state->program->read_data8((Sreg(PS)<<4)+nec_state->ip++);
}

INLINE UINT16 fetchword(nec_state_t *nec_state)
{
	UINT16 r = FETCH();
	r |= (FETCH()<<8);
	return r;
}

#include "necinstr.h"
#include "necmacro.h"
#include "necea.h"
#include "necmodrm.h"

static UINT8 parity_table[256];

static UINT8 fetchop(nec_state_t *nec_state)
{
	prefetch(nec_state);
	return nec_state->program->read_data8(( Sreg(PS)<<4)+nec_state->ip++);
}



/***************************************************************************/

static CPU_RESET( nec )
{
	memset( &nec_state->regs.w, 0, sizeof(nec_state->regs.w));

	nec_state->ip = 0;
	nec_state->TF = 0;
	nec_state->IF = 0;
	nec_state->DF = 0;
	nec_state->MF = 1;	// brkem should set to 0 when implemented
	nec_state->SignVal = 0;
	nec_state->AuxVal = 0;
	nec_state->OverVal = 0;
	nec_state->ZeroVal = 1;
	nec_state->CarryVal = 0;
	nec_state->ParityVal = 1;
	nec_state->pending_irq = 0;
	nec_state->nmi_state = 0;
	nec_state->irq_state = 0;
	nec_state->poll_state = 1;
	nec_state->halted = 0;

	Sreg(PS) = 0xffff;
	Sreg(SS) = 0;
	Sreg(DS0) = 0;
	Sreg(DS1) = 0;

	CHANGE_PC;
}

static void nec_interrupt(nec_state_t *nec_state, unsigned int_num, INTSOURCES source)
{
    UINT32 dest_seg, dest_off;

    i_pushf(nec_state);
	nec_state->TF = nec_state->IF = 0;

	if (source == INT_IRQ)	/* get vector */
		int_num = nec_state->pic->intr_ack();

    dest_off = read_mem_word(int_num*4);
    dest_seg = read_mem_word(int_num*4+2);

	PUSH(Sreg(PS));
	PUSH(nec_state->ip);
	nec_state->ip = (WORD)dest_off;
	Sreg(PS) = (WORD)dest_seg;
	CHANGE_PC;
}

static void nec_trap(nec_state_t *nec_state)
{
	nec_instruction[fetchop(nec_state)](nec_state);
	nec_interrupt(nec_state, NEC_TRAP_VECTOR, BRK);
}

static void external_int(nec_state_t *nec_state)
{
	if (nec_state->pending_irq & NMI_IRQ)
	{
		nec_interrupt(nec_state, NEC_NMI_VECTOR, NMI_IRQ);
		nec_state->pending_irq &= ~NMI_IRQ;
	}
	else if (nec_state->pending_irq)
	{
		/* the actual vector is retrieved after pushing flags */
		/* and clearing the IF */
		nec_interrupt(nec_state, (UINT32)-1, INT_IRQ);
		nec_state->irq_state = CLEAR_LINE;
		nec_state->pending_irq &= ~INT_IRQ;
	}
}

/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

#include "necinstr.c"

/*****************************************************************************/

static void set_irq_line(nec_state_t *nec_state, int irqline, int state)
{
	switch (irqline)
	{
		case 0:
			nec_state->irq_state = state;
			if (state == CLEAR_LINE)
				nec_state->pending_irq &= ~INT_IRQ;
			else
			{
				nec_state->pending_irq |= INT_IRQ;
				nec_state->halted = 0;
			}
			break;
		case INPUT_LINE_NMI:
			if (nec_state->nmi_state == state) return;
		    nec_state->nmi_state = state;
			if (state != CLEAR_LINE)
			{
				nec_state->pending_irq |= NMI_IRQ;
				nec_state->halted = 0;
			}
			break;
		case NEC_INPUT_LINE_POLL:
			nec_state->poll_state = state;
			break;
	}
}

static nec_state_t *nec_init()
{
	nec_state_t *nec_state = (nec_state_t *)calloc(1, sizeof(nec_state_t));

	unsigned int i, j, c;

	static const WREGS wreg_name[8]={ AW, CW, DW, BW, SP, BP, IX, IY };
	static const BREGS breg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;
		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = breg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = wreg_name[(i & 0x38) >> 3];
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = wreg_name[i & 7];
		Mod_RM.RM.b[i] = breg_name[i & 7];
	}

	memset(nec_state, 0, sizeof(*nec_state));

	return nec_state;
}



static CPU_EXECUTE( necv )
{
	int prev_ICount;

	if (nec_state->halted || nec_state->busreq)
	{
		nec_state->icount = 0;
		return 1;
	}

	if (icount == -1) {
		nec_state->icount = 1;
	} else {
		nec_state->icount += icount;
	}
	int base_icount = nec_state->icount;

	while(nec_state->icount > 0 && !nec_state->busreq) {
		/* Dispatch IRQ */
		if (nec_state->pending_irq && nec_state->no_interrupt==0)
		{
			if (nec_state->pending_irq & NMI_IRQ)
				external_int(nec_state);
			else if (nec_state->IF)
				external_int(nec_state);
		}

		/* No interrupt allowed between last instruction and this one */
		if (nec_state->no_interrupt)
			nec_state->no_interrupt--;

		nec_state->prevpc = PC(nec_state);
		prev_ICount = nec_state->icount;
		nec_instruction[fetchop(nec_state)](nec_state);
		do_prefetch(nec_state, prev_ICount);
	}

	int passed_icount = base_icount - nec_state->icount;
	if (nec_state->icount > 0 && nec_state->busreq) {
		nec_state->icount = 0;
	}
	return passed_icount;
}

/* Wrappers for the different CPU types */
static CPU_INIT( v20 )
{
	nec_state_t *nec_state = nec_init();

	nec_state->chip_type=V20_TYPE;
	nec_state->prefetch_size = 4;		/* 3 words */
	nec_state->prefetch_cycles = 4;		/* four cycles per byte */

	return nec_state;
}

static CPU_INIT( v30 )
{
	nec_state_t *nec_state = nec_init();

	nec_state->chip_type=V30_TYPE;
	nec_state->prefetch_size = 6;		/* 3 words */
	nec_state->prefetch_cycles = 2;		/* two cycles per byte / four per word */

	return nec_state;
}

static CPU_INIT( v33 )
{
	nec_state_t *nec_state = nec_init();

	nec_state->chip_type=V33_TYPE;
	nec_state->prefetch_size = 6;
	/* FIXME: Need information about prefetch size and cycles for V33.
     * complete guess below, nbbatman will not work
     * properly without. */
	nec_state->prefetch_cycles = 1;		/* two cycles per byte / four per word */

	return nec_state;
}
