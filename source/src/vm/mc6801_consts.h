#ifndef __MC6801_CONSTS_H__
#define __MC6801_CONSTS_H__
#define CT	counter.w.l
#define CTH	counter.w.h
#define CTD	counter.d
#define OC	output_compare.w.l
#define OCH	output_compare.w.h
#define OCD	output_compare.d
#define TOH	timer_over.w.l
#define TOD	timer_over.d

#define SET_TIMER_EVENT { \
	timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD; \
}

#define CLEANUP_COUNTERS() { \
	OCH -= CTH; \
	TOH -= CTH; \
	CTH = 0; \
	SET_TIMER_EVENT; \
}

#define MODIFIED_counters { \
	OCH = (OC >= CT) ? CTH : CTH + 1; \
	SET_TIMER_EVENT; \
}

#define TCSR_OLVL	0x01
#define TCSR_IEDG	0x02
#define TCSR_ETOI	0x04
#define TCSR_EOCI	0x08
#define TCSR_EICI	0x10
#define TCSR_TOF	0x20
#define TCSR_OCF	0x40
#define TCSR_ICF	0x80

#define TRCSR_WU	0x01
#define TRCSR_TE	0x02
#define TRCSR_TIE	0x04
#define TRCSR_RE	0x08
#define TRCSR_RIE	0x10
#define TRCSR_TDRE	0x20
#define TRCSR_ORFE	0x40
#define TRCSR_RDRF	0x80

#define P3CSR_LE		0x08
#define P3CSR_IS3_ENABLE	0x40
#define P3CSR_IS3_FLAG		0x80

/****************************************************************************/
/* MC6801/HD6301 internal i/o port                                          */
/****************************************************************************/

/* take interrupt */
#define TAKE_ICI	enter_interrupt(0xfff6)
#define TAKE_OCI	enter_interrupt(0xfff4)
#define TAKE_TOI	enter_interrupt(0xfff2)
#define TAKE_SCI	enter_interrupt(0xfff0)
#define TAKE_TRAP	enter_interrupt(0xffee)

#endif /* __MC6801_CONSTS_H__ */
