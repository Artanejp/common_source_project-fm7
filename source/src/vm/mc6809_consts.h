/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

// Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17

#ifndef _MC6809_CONSTS_H
#define _MC6809_CONSTS_H

#define MC6809_IRQ_BIT	1	/* IRQ line number  */
#define MC6809_FIRQ_BIT	2	/* FIRQ line number */
#define MC6809_NMI_BIT	4	/* NMI line number  */
#define MC6809_HALT_BIT	8	/* HALT line number  */

/* flag bits in the cc register */
#define MC6809_CWAI	 0x0010	/* set when CWAI is waiting for an interrupt */
#define MC6809_SYNC	 0x0020	/* set when SYNC is waiting for an interrupt */
#define MC6809_CWAI_IN	 0x0040	/* set when CWAI is waiting for an interrupt */
#define MC6809_CWAI_OUT	 0x0080	/* set when CWAI is waiting for an interrupt */
#define MC6809_SYNC_IN	 0x0100	/* set when SYNC is waiting for an interrupt */
#define MC6809_SYNC_OUT	 0x0200	/* set when SYNC is waiting for an interrupt */
#define MC6809_LDS	 0x0400	/* set when LDS occured at least once */
#define MC6809_NMI_LC	 0x1000	/* NMI割り込み信号3サイクル未満 */
#define MC6809_FIRQ_LC	 0x2000	/* FIRQ割り込み信号3サイクル未満 */
#define MC6809_IRQ_LC	 0x4000	/* IRQ割り込み信号3サイクル未満 */
#define MC6809_INSN_HALT 0x8000	/* IRQ割り込み信号3サイクル未満 */

#define CC_C	0x01		/* Carry */
#define CC_V	0x02		/* Overflow */
#define CC_Z	0x04		/* Zero */
#define CC_N	0x08		/* Negative */
#define CC_II	0x10		/* Inhibit IRQ */
#define CC_H	0x20		/* Half (auxiliary) carry */
#define CC_IF	0x40		/* Inhibit FIRQ */
#define CC_E	0x80		/* entire state pushed */

      
#endif //#ifndef _MC6809_CONSTS_H
