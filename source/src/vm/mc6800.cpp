/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.04.23-

	[ MC6800 ]
*/

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4996 )
#endif

#include "mc6800.h"
#if defined(HAS_MC6801) || defined(HAS_HD6301)
#include "../fifo.h"
#endif
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define INT_REQ_BIT	1
#define NMI_REQ_BIT	2

#define MC6800_WAI	8
#define HD6301_SLP	0x10

#define pPC	pc
#define pS	sp
#define pX	ix
#define pD	acc_d
#define pEA	ea

#define PC	pc.w.l
#define PCD	pc.d
#define S	sp.w.l
#define SD	sp.d
#define X	ix.w.l
#define D	acc_d.w.l
#define A	acc_d.b.h
#define B	acc_d.b.l
#define CC	cc

#define EAD	ea.d
#define EA	ea.w.l

/****************************************************************************/
/* memory                                                                   */
/****************************************************************************/

uint32_t MC6800::RM(uint32_t Addr)
{
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	if(Addr < 0x20) {
		return mc6801_io_r(Addr);
	} else if(Addr >= 0x80 && Addr < 0x100 && (ram_ctrl & 0x40)) {
		return ram[Addr & 0x7f];
	}
#endif
	return d_mem->read_data8(Addr);
}

void MC6800::WM(uint32_t Addr, uint32_t Value)
{
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	if(Addr < 0x20) {
		mc6801_io_w(Addr, Value);
	} else if(Addr >= 0x80 && Addr < 0x100 && (ram_ctrl & 0x40)) {
		ram[Addr & 0x7f] = Value;
	} else
#endif
	d_mem->write_data8(Addr, Value);
}

uint32_t MC6800::RM16(uint32_t Addr)
{
	uint32_t result = RM(Addr) << 8;
	return result | RM((Addr + 1) & 0xffff);
}

void MC6800::WM16(uint32_t Addr, pair32_t *p)
{
	WM(Addr, p->b.h);
	WM((Addr + 1) & 0xffff, p->b.l);
}

//#define M_RDOP(Addr)		d_mem->read_data8(Addr)
//#define M_RDOP_ARG(Addr)	d_mem->read_data8(Addr)
#define M_RDOP(Addr)		RM(Addr)
#define M_RDOP_ARG(Addr)	RM(Addr)

/* macros to access memory */
#define IMMBYTE(b)	b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)	w.b.h = M_RDOP_ARG(PCD); w.b.l = M_RDOP_ARG((PCD + 1) & 0xffff); PC += 2

#define PUSHBYTE(b)	WM(SD, b); --S
#define PUSHWORD(w)	WM(SD, w.b.l); --S; WM(SD, w.b.h); --S
#define PULLBYTE(b)	S++; b = RM(SD)
#define PULLWORD(w)	S++; w.b.h = RM(SD); S++; w.b.l = RM(SD)

/****************************************************************************/
/* MC6801/HD6301 internal i/o port                                          */
/****************************************************************************/

#if defined(HAS_MC6801) || defined(HAS_HD6301)

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

static const int RMCR_SS[] = { 16, 128, 1024, 4096 };

/* take interrupt */
#define TAKE_ICI	enter_interrupt(0xfff6)
#define TAKE_OCI	enter_interrupt(0xfff4)
#define TAKE_TOI	enter_interrupt(0xfff2)
#define TAKE_SCI	enter_interrupt(0xfff0)
#define TAKE_TRAP	enter_interrupt(0xffee)

uint32_t MC6800::mc6801_io_r(uint32_t offset)
{
	switch (offset) {
	case 0x00:
		// port1 data direction register
		return port[0].ddr;
	case 0x01:
		// port2 data direction register
		return port[1].ddr;
	case 0x02:
		// port1 data register
		return (port[0].rreg & ~port[0].ddr) | (port[0].wreg & port[0].ddr);
	case 0x03:
		// port2 data register
		return (port[1].rreg & ~port[1].ddr) | (port[1].wreg & port[1].ddr);
	case 0x04:
		// port3 data direction register (write only???)
		return port[2].ddr;
	case 0x05:
		// port4 data direction register
		return port[3].ddr;
	case 0x06:
		// port3 data register
		if(p3csr_is3_flag_read) {
			p3csr_is3_flag_read = false;
			p3csr &= ~P3CSR_IS3_FLAG;
		}
		if(port[2].latched) {
			port[2].latched = false;
			return (port[2].latched_data & ~port[2].ddr) | (port[2].wreg & port[2].ddr);
		}
		return (port[2].rreg & ~port[2].ddr) | (port[2].wreg & port[2].ddr);
	case 0x07:
		// port4 data register
		return (port[3].rreg & ~port[3].ddr) | (port[3].wreg & port[3].ddr);
	case 0x08:
		// timer control register
		pending_tcsr = 0;
		return tcsr;
	case 0x09:
		// free running counter (msb)
		if(!(pending_tcsr & TCSR_TOF)) {
			tcsr &= ~TCSR_TOF;
		}
		return counter.b.h;
	case 0x0a:
		// free running counter (lsb)
		return counter.b.l;
	case 0x0b:
		// output compare register (msb)
		if(!(pending_tcsr & TCSR_OCF)) {
			tcsr &= ~TCSR_OCF;
		}
		return output_compare.b.h;
	case 0x0c:
		// output compare register (lsb)
		if(!(pending_tcsr & TCSR_OCF)) {
			tcsr &= ~TCSR_OCF;
		}
		return output_compare.b.l;
	case 0x0d:
		// input capture register (msb)
		if(!(pending_tcsr & TCSR_ICF)) {
			tcsr &= ~TCSR_ICF;
		}
		return (input_capture >> 0) & 0xff;
	case 0x0e:
		// input capture register (lsb)
		return (input_capture >> 8) & 0xff;
	case 0x0f:
		// port3 control/status register
		p3csr_is3_flag_read = true;
		return p3csr;
	case 0x10:
		// rate and mode control register
		return rmcr;
	case 0x11:
		if(trcsr & TRCSR_TDRE) {
			trcsr_read_tdre = true;
		}
		if(trcsr & TRCSR_ORFE) {
			trcsr_read_orfe = true;
		}
		if(trcsr & TRCSR_RDRF) {
			trcsr_read_rdrf = true;
		}
		return trcsr;
	case 0x12:
		// receive data register
		if(trcsr_read_orfe) {
			trcsr_read_orfe = false;
			trcsr &= ~TRCSR_ORFE;
		}
		if(trcsr_read_rdrf) {
			trcsr_read_rdrf = false;
			trcsr &= ~TRCSR_RDRF;
		}
		return rdr;
	case 0x13:
		// transmit data register
		return tdr;
	case 0x14:
		// ram control register
		return (ram_ctrl & 0x40) | 0x3f;
	}
	return 0;
}

void MC6800::mc6801_io_w(uint32_t offset, uint32_t data)
{
	switch(offset) {
	case 0x00:
		// port1 data direction register
		port[0].ddr = data;
		break;
	case 0x01:
		// port2 data direction register
		port[1].ddr = data;
		break;
	case 0x02:
		// port1 data register
		if(port[0].wreg != data || port[0].first_write) {
			write_signals(&port[0].outputs, data);
			port[0].wreg = data;
			port[0].first_write = false;
		}
		break;
	case 0x03:
		// port2 data register
		if(port[1].wreg != data || port[1].first_write) {
			write_signals(&port[1].outputs, data);
			port[1].wreg = data;
			port[1].first_write = false;
		}
		break;
	case 0x04:
		// port3 data direction register
		port[2].ddr = data;
		break;
	case 0x05:
		// port4 data direction register
		port[3].ddr = data;
		break;
	case 0x06:
		// port3 data register
		if(p3csr_is3_flag_read) {
			p3csr_is3_flag_read = false;
			p3csr &= ~P3CSR_IS3_FLAG;
		}
		if(port[2].wreg != data || port[2].first_write) {
			write_signals(&port[2].outputs, data);
			port[2].wreg = data;
			port[2].first_write = false;
		}
		break;
	case 0x07:
		// port4 data register
		if(port[3].wreg != data || port[3].first_write) {
			write_signals(&port[3].outputs, data);
			port[3].wreg = data;
			port[3].first_write = false;
		}
		break;
	case 0x08:
		// timer control/status register
		tcsr = data;
		pending_tcsr &= tcsr;
		break;
	case 0x09:
		// free running counter (msb)
#ifdef HAS_HD6301
		latch09 = data & 0xff;
#endif
		CT = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;
#ifdef HAS_HD6301
	case 0x0a:
		// free running counter (lsb)
		CT = (latch09 << 8) | (data & 0xff);
		TOH = CTH;
		MODIFIED_counters;
		break;
#endif
	case 0x0b:
		// output compare register (msb)
		if(output_compare.b.h != data) {
			output_compare.b.h = data;
			MODIFIED_counters;
		}
        tcsr &=~TCSR_OCF;
		break;
	case 0x0c:
		// output compare register (lsb)
		if(output_compare.b.l != data) {
			output_compare.b.l = data;
			MODIFIED_counters;
		}
        tcsr &=~TCSR_OCF;
		break;
	case 0x0f:
		// port3 control/status register
		p3csr = (p3csr & P3CSR_IS3_FLAG) | (data & ~P3CSR_IS3_FLAG);
		break;
	case 0x10:
		// rate and mode control register
		rmcr = data;
		break;
	case 0x11:
		// transmit/receive control/status register
		trcsr = (trcsr & 0xe0) | (data & 0x1f);
		break;
	case 0x13:
		// transmit data register
		if(trcsr_read_tdre) {
			trcsr_read_tdre = false;
			trcsr &= ~TRCSR_TDRE;
		}
		tdr = data;
		break;
	case 0x14:
		// ram control register
		ram_ctrl = data;
		break;
	}
}
#endif

void MC6800::increment_counter(int amount)
{
#ifdef USE_DEBUGGER
	total_icount += amount;
#endif
	icount -= amount;
	
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	// timer
	if((CTD += amount) >= timer_next) {
		/* OCI */
		if( CTD >= OCD) {
			OCH++;	// next IRQ point
			tcsr |= TCSR_OCF;
			pending_tcsr |= TCSR_OCF;
		}
		/* TOI */
		if( CTD >= TOD) {
			TOH++;	// next IRQ point
			tcsr |= TCSR_TOF;
			pending_tcsr |= TCSR_TOF;
		}
		/* set next event */
		SET_TIMER_EVENT;
	}
	
	// serial i/o
	if((sio_counter -= amount) <= 0) {
		if((trcsr & TRCSR_TE) && !(trcsr & TRCSR_TDRE)) {
			write_signals(&outputs_sio, tdr);
			trcsr |= TRCSR_TDRE;
		}
		if((trcsr & TRCSR_RE) && !recv_buffer->empty()) {
			if(trcsr & TRCSR_WU) {
				// skip 10 bits
				trcsr &= ~TRCSR_WU;
				recv_buffer->read();
			} else if(!(trcsr & TRCSR_RDRF)) {
				// note: wait reveived data is read by cpu, so overrun framing error never occurs
				rdr = recv_buffer->read();
				trcsr |= TRCSR_RDRF;
			}
		}
		sio_counter += RMCR_SS[rmcr & 3];
	}
#endif
}


#define CLR_HNZVC	CC &= 0xd0
#define CLR_NZV		CC &= 0xf1
#define CLR_HNZC	CC &= 0xd2
#define CLR_NZVC	CC &= 0xf0
#define CLR_NZ		CC &= 0xf3
#define CLR_Z		CC &= 0xfb
#define CLR_NZC		CC &= 0xf2
#define CLR_ZC		CC &= 0xfa
#define CLR_C		CC &= 0xfe

#define SET_Z(a)	if(!(a)) SEZ
#define SET_Z8(a)	SET_Z((uint8_t)(a))
#define SET_Z16(a)	SET_Z((uint16_t)(a))
#define SET_N8(a)	CC |= (((a) & 0x80) >> 4)
#define SET_N16(a)	CC |= (((a) & 0x8000) >> 12)
#define SET_H(a,b,r)	CC |= ((((a) ^ (b) ^ (r)) & 0x10) << 1)
#define SET_C8(a)	CC |= (((a) & 0x100) >> 8)
#define SET_C16(a)	CC |= (((a) & 0x10000) >> 16)
#define SET_V8(a,b,r)	CC |= ((((a) ^ (b) ^ (r) ^ ((r) >> 1)) & 0x80) >> 6)
#define SET_V16(a,b,r)	CC |= ((((a) ^ (b) ^ (r) ^ ((r) >> 1)) & 0x8000) >> 14)

static const uint8_t flags8i[256] = {
	/* increment */
	0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

static const uint8_t flags8d[256] = {
	/* decrement */
	0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

#define SET_FLAGS8I(a)		{CC |= flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)		{CC |= flags8d[(a) & 0xff];}

/* combos */
#define SET_NZ8(a)		{SET_N8(a);  SET_Z8(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);  SET_Z8(r);  SET_V8(a,b,r);  SET_C8(r); }
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a,b,r); SET_C16(r);}

/* for treating an uint8_t as a signed int16_t */
#define SIGNED(b)	((int16_t)(b & 0x80 ? b | 0xff00 : b))

/* Macros for addressing modes */
#define DIRECT		IMMBYTE(EAD)
#define IMM8		EA = PC++
#define IMM16		{EA = PC; PC += 2;}
#define EXTENDED	IMMWORD(pEA)
#define INDEXED		{EA = X + (uint8_t)M_RDOP_ARG(PCD); PC++;}

/* macros to set status flags */
#define SEC	CC |= 0x01
#define CLC	CC &= 0xfe
#define SEZ	CC |= 0x04
#define CLZ	CC &= 0xfb
#define SEN	CC |= 0x08
#define CLN	CC &= 0xf7
#define SEV	CC |= 0x02
#define CLV	CC &= 0xfd
#define SEH	CC |= 0x20
#define CLH	CC &= 0xdf
#define SEI	CC |= 0x10
#define CLI	CC &= ~0x10

/* macros for convenience */
#define DIRBYTE(b)	{DIRECT;   b   = RM(EAD);  }
#define DIRWORD(w)	{DIRECT;   w.d = RM16(EAD);}
#define EXTBYTE(b)	{EXTENDED; b   = RM(EAD);  }
#define EXTWORD(w)	{EXTENDED; w.d = RM16(EAD);}

#define IDXBYTE(b)	{INDEXED;  b   = RM(EAD);  }
#define IDXWORD(w)	{INDEXED;  w.d = RM16(EAD);}

/* Macros for branch instructions */
#define BRANCH(f)	{IMMBYTE(t); if(f) {PC += SIGNED(t);}}
#define NXORV		((CC & 0x08) ^ ((CC & 0x02) << 2))

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */
#define XX 5 // invalid opcode unknown cc
static const uint8_t cycles[] = {
#if defined(HAS_MC6800)
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
#elif defined(HAS_MC6801)
	XX, 2,XX,XX, 3, 3, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2,
	 2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	 3, 3, 4, 4, 3, 3, 3, 3, 5, 5, 3,10, 4,10, 9,12,
	 2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	 2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	 6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	 6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 6, 3, 3,
	 3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4,
	 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 5, 5,
	 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	 3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
#elif defined(HAS_HD6301)
	XX, 1,XX,XX, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	 1, 1,XX,XX,XX,XX, 1, 1, 2, 2, 4, 1,XX,XX,XX,XX,
	 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	 1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1,10, 5, 7, 9,12,
	 1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	 1,XX,XX, 1, 1,XX, 1, 1, 1, 1, 1,XX, 1, 1,XX, 1,
	 6, 7, 7, 6, 6, 7, 6, 6, 6, 6, 6, 5, 6, 4, 3, 5,
	 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4, 3, 5,
	 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3, 5, 3, 3,
	 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4,
	 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5,
	 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 3,XX, 3, 3,
	 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
	 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5
#elif defined(HAS_MB8861)
	XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
	 2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	 4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
	 2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	 2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	 7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
	 6, 8, 8, 6, 6, 8, 6, 6, 6, 6, 6, 7, 6, 6, 3, 6,
	 2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
	 3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
	 5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
	 4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
	 2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
	 3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
	 5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 4,XX, 6, 7,
	 4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 7,XX, 5, 6
#endif
};
#undef XX // invalid opcode unknown cc


void MC6800::initialize()
{
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	recv_buffer = new FIFO(0x10000);
	ram_ctrl = 0xc0;
#endif
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_debugger->set_context_mem(d_mem);
#endif
}

#if defined(HAS_MC6801) || defined(HAS_HD6301)
void MC6800::release()
{
	recv_buffer->release();
	delete recv_buffer;
}
#endif

void MC6800::reset()
{
	CC = 0xc0;
	SEI; /* IRQ disabled */
	PCD = RM16(0xfffe);
//	S = X = D = EA = 0;
	SD = X = D = EAD = 0;
	
	wai_state = 0;
	int_state = 0;
	
	icount = 0;
	
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	for(int i = 0; i < 4; i++) {
		port[i].ddr = 0x00;
		port[i].first_write = true;
		port[i].latched = false;
	}
	p3csr = 0x00;
	p3csr_is3_flag_read = false;
	sc1_state = sc2_state = false;
	
	tcsr = pending_tcsr = 0x00;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	
	recv_buffer->clear();
	trcsr = TRCSR_TDRE;
	trcsr_read_tdre = trcsr_read_orfe = trcsr_read_rdrf = false;
	rmcr = 0x00;
	sio_counter = RMCR_SS[rmcr & 3];
	
	ram_ctrl |= 0x40;
#endif
}

void MC6800::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_CPU_IRQ:
		if(data & mask) {
			int_state |= INT_REQ_BIT;
		} else {
			int_state &= ~INT_REQ_BIT;
		}
		break;
	case SIG_CPU_NMI:
		if(data & mask) {
			int_state |= NMI_REQ_BIT;
		} else {
			int_state &= ~NMI_REQ_BIT;
		}
		break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case SIG_MC6801_PORT_1:
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_2:
		if((mask & 1) && (port[1].rreg & 1) != (data & 1) && (tcsr & 2) == ((data << 1) & 2)) {
			// active TIN edge in
			tcsr |= TCSR_ICF;
			pending_tcsr |= TCSR_ICF;
			input_capture = CT;
		}
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_3:
		port[2].rreg = (port[2].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_4:
		port[3].rreg = (port[3].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_3_SC1:
		if(sc1_state && !(data & mask)) {
			// SC1: H -> L
			if(!port[2].latched && (p3csr & P3CSR_LE)) {
				port[2].latched_data = port[2].rreg;
				port[2].latched = true;
				p3csr |= P3CSR_IS3_FLAG;
			}
		}
		sc1_state = ((data & mask) != 0);
		break;
	case SIG_MC6801_PORT_3_SC2:
		sc2_state = ((data & mask) != 0);
		break;
	case SIG_MC6801_SIO_RECV:
		recv_buffer->write(data & mask);
		break;
#endif
	}
}

int MC6800::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
#if defined(HAS_MC6801) || defined(HAS_HD6301)
		CLEANUP_COUNTERS();
#endif
		icount = 0;
		run_one_opecode();
		return -icount;
	} else {
		/* run cpu while given clocks */
#if defined(HAS_MC6801) || defined(HAS_HD6301)
		CLEANUP_COUNTERS();
#endif
		icount += clock;
		int first_icount = icount;
		
		while(icount > 0) {
			run_one_opecode();
		}
		return first_icount - icount;
	}
}

void MC6800::run_one_opecode()
{
	if(wai_state & (MC6800_WAI | HD6301_SLP)) {
		increment_counter(1);
	} else {
		do {
			one_more_insn = false;
#ifdef USE_DEBUGGER
			bool now_debugging = d_debugger->now_debugging;
			if(now_debugging) {
				d_debugger->check_break_points(PC);
				if(d_debugger->now_suspended) {
					d_debugger->now_waiting = true;
					emu->start_waiting_in_debugger();
					while(d_debugger->now_debugging && d_debugger->now_suspended) {
						emu->process_waiting_in_debugger();
					}
					emu->finish_waiting_in_debugger();
					d_debugger->now_waiting = false;
				}
				if(d_debugger->now_debugging) {
					d_mem = d_debugger;
				} else {
					now_debugging = false;
				}
				
				d_debugger->add_cpu_trace(PC);
				uint8_t ireg = M_RDOP(PCD);
				prevpc = PC;
				PC++;
				insn(ireg);
				increment_counter(cycles[ireg]);
				
				if(now_debugging) {
					if(!d_debugger->now_going) {
						d_debugger->now_suspended = true;
					}
					d_mem = d_mem_stored;
				}
			} else {
				d_debugger->add_cpu_trace(PC);
#endif
				uint8_t ireg = M_RDOP(PCD);
				prevpc = PC;
				PC++;
				insn(ireg);
				increment_counter(cycles[ireg]);
#ifdef USE_DEBUGGER
			}
#endif
		} while(one_more_insn);
	}
	
	// check interrupt
	if(int_state & NMI_REQ_BIT) {
		wai_state &= ~HD6301_SLP;
		int_state &= ~NMI_REQ_BIT;
		enter_interrupt(0xfffc);
	} else if(int_state & INT_REQ_BIT) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			int_state &= ~INT_REQ_BIT;
			enter_interrupt(0xfff8);
		}
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	} else if((tcsr & (TCSR_EICI | TCSR_ICF)) == (TCSR_EICI | TCSR_ICF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_ICI;
		}
	} else if((tcsr & (TCSR_EOCI | TCSR_OCF)) == (TCSR_EOCI | TCSR_OCF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_OCI;
		}
	} else if((tcsr & (TCSR_ETOI | TCSR_TOF)) == (TCSR_ETOI | TCSR_TOF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_TOI;
		}
	} else if(((trcsr & (TRCSR_RIE | TRCSR_RDRF)) == (TRCSR_RIE | TRCSR_RDRF)) ||
	          ((trcsr & (TRCSR_RIE | TRCSR_ORFE)) == (TRCSR_RIE | TRCSR_ORFE)) ||
	          ((trcsr & (TRCSR_TIE | TRCSR_TDRE)) == (TRCSR_TIE | TRCSR_TDRE))) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_SCI;
		}
#endif
	}
}

#ifdef USE_DEBUGGER
void MC6800::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t MC6800::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void MC6800::write_debug_data16(uint32_t addr, uint32_t data)
{
	write_debug_data8(addr, (data >> 8) & 0xff);
	write_debug_data8(addr + 1, data & 0xff);
}

uint32_t MC6800::read_debug_data16(uint32_t addr)
{
	uint32_t val = read_debug_data8(addr) << 8;
	val |= read_debug_data8(addr + 1);
	return val;
}

void MC6800::write_debug_data32(uint32_t addr, uint32_t data)
{
	write_debug_data16(addr, (data >> 16) & 0xffff);
	write_debug_data16(addr + 2, data & 0xffff);
}

uint32_t MC6800::read_debug_data32(uint32_t addr)
{
	uint32_t val = read_debug_data16(addr) << 16;
	val |= read_debug_data16(addr + 2);
	return val;
}

bool MC6800::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		B = data;
	} else if(_tcsicmp(reg, _T("X")) == 0 || _tcsicmp(reg, _T("IX")) == 0) {
		X = data;
	} else if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		S = data;
	} else {
		return false;
	}
	return true;
}

bool MC6800::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
	_T("CCR = [%c%c%c%c%c%c]  A = %02X  B = %02X  IX = %04X  PC = %04X  SP = %04X\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	(CC & 0x01) ? _T('C') : _T('-'), (CC & 0x02) ? _T('V') : _T('-'), (CC & 0x04) ? _T('Z') : _T('-'), (CC & 0x08) ? _T('N') : _T('-'),
	(CC & 0x10) ? _T('I') : _T('-'), (CC & 0x20) ? _T('X') : _T('-'), A, B, X, PC, S,
	total_icount, total_icount - prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_icount = total_icount;
	return true;
}

/*
 *   A quick-hack 6803/6808 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 * History:
 * 990314 HJB
 * The disassembler knows about valid opcodes for M6800/1/2/3/8 and HD63701.
 * 990302 HJB
 * Changed the string array into a table of opcode names (or tokens) and
 * argument types. This second try should give somewhat better results.
 * Named the undocumented HD63701YO opcodes $12 and $13 'asx1' and 'asx2',
 * since 'add contents of stack to x register' is what they do.
 *
 */

enum addr_mode {
	inh,    /* inherent */
	rel,    /* relative */
	imb,    /* immediate (byte) */
	imw,    /* immediate (word) */
	dir,    /* direct address */
	imd,    /* HD63701YO: immediate, direct address */
	ext,    /* extended address */
	idx,    /* x + byte offset */
	imx,    /* HD63701YO: immediate, x + byte offset */
	sx1     /* HD63701YO: undocumented opcodes: byte from (s+1) */
};

enum op_names {
	aba=0,  abx,    adca,   adcb,   adda,   addb,   addd,   aim,
	anda,   andb,   asl,    asla,   aslb,   asld,   asr,    asra,
	asrb,   bcc,    bcs,    beq,    bge,    bgt,    bhi,    bita,
	bitb,   ble,    bls,    blt,    bmi,    bne,    bpl,    bra,
	brn,    bsr,    bvc,    bvs,    cba,    clc,    cli,    clr,
	clra,   clrb,   clv,    cmpa,   cmpb,   cmpx,   com,    coma,
	comb,   daa,    dec,    deca,   decb,   des,    dex,    eim,
	eora,   eorb,   ill,    inc,    inca,   incb,   ins,    inx,
	jmp,    jsr,    lda,    ldb,    ldd,    lds,    ldx,    lsr,
	lsra,   lsrb,   lsrd,   mul,    neg,    nega,   negb,   nop,
	oim,    ora,    orb,    psha,   pshb,   pshx,   pula,   pulb,
	pulx,   rol,    rola,   rolb,   ror,    rora,   rorb,   rti,
	rts,    sba,    sbca,   sbcb,   sec,    sev,    sta,    stb,
	_std,   sei,    sts,    stx,    suba,   subb,   subd,   swi,
	wai,    tab,    tap,    tba,    tim,    tpa,    tst,    tsta,
	tstb,   tsx,    txs,    asx1,   asx2,   xgdx,   addx,   adcx
};

static const char *const op_name_str[] = {
	"aba",   "abx",   "adca",  "adcb",  "adda",  "addb",  "addd",  "aim",
	"anda",  "andb",  "asl",   "asla",  "aslb",  "asld",  "asr",   "asra",
	"asrb",  "bcc",   "bcs",   "beq",   "bge",   "bgt",   "bhi",   "bita",
	"bitb",  "ble",   "bls",   "blt",   "bmi",   "bne",   "bpl",   "bra",
	"brn",   "bsr",   "bvc",   "bvs",   "cba",   "clc",   "cli",   "clr",
	"clra",  "clrb",  "clv",   "cmpa",  "cmpb",  "cmpx",  "com",   "coma",
	"comb",  "daa",   "dec",   "deca",  "decb",  "des",   "dex",   "eim",
	"eora",  "eorb",  "illegal","inc",   "inca",  "incb",  "ins",   "inx",
	"jmp",   "jsr",   "lda",   "ldb",   "ldd",   "lds",   "ldx",   "lsr",
	"lsra",  "lsrb",  "lsrd",  "mul",   "neg",   "nega",  "negb",  "nop",
	"oim",   "ora",   "orb",   "psha",  "pshb",  "pshx",  "pula",  "pulb",
	"pulx",  "rol",   "rola",  "rolb",  "ror",   "rora",  "rorb",  "rti",
	"rts",   "sba",   "sbca",  "sbcb",  "sec",   "sev",   "sta",   "stb",
	"std",   "sei",   "sts",   "stx",   "suba",  "subb",  "subd",  "swi",
	"wai",   "tab",   "tap",   "tba",   "tim",   "tpa",   "tst",   "tsta",
	"tstb",  "tsx",   "txs",   "asx1",  "asx2",  "xgdx",  "addx",  "adcx"
};

/*
 * This table defines the opcodes:
 * byte meaning
 * 0    token (menmonic)
 * 1    addressing mode
 * 2    invalid opcode for 1:6800/6802/6808, 2:6801/6803, 4:HD63701
 */

static const UINT8 table[0x102][3] = {
	{ill, inh,7},{nop, inh,0},{ill, inh,7},{ill, inh,7},/* 00 */
	{lsrd,inh,1},{asld,inh,1},{tap, inh,0},{tpa, inh,0},
	{inx, inh,0},{dex, inh,0},{clv, inh,0},{sev, inh,0},
	{clc, inh,0},{sec, inh,0},{cli, inh,0},{sei, inh,0},
	{sba, inh,0},{cba, inh,0},{asx1,sx1,0},{asx2,sx1,0},/* 10 */
	{ill, inh,7},{ill, inh,7},{tab, inh,0},{tba, inh,0},
	{xgdx,inh,3},{daa, inh,0},{ill, inh,7},{aba, inh,0},
	{ill, inh,7},{ill, inh,7},{ill, inh,7},{ill, inh,7},
	{bra, rel,0},{brn, rel,0},{bhi, rel,0},{bls, rel,0},/* 20 */
	{bcc, rel,0},{bcs, rel,0},{bne, rel,0},{beq, rel,0},
	{bvc, rel,0},{bvs, rel,0},{bpl, rel,0},{bmi, rel,0},
	{bge, rel,0},{blt, rel,0},{bgt, rel,0},{ble, rel,0},
	{tsx, inh,0},{ins, inh,0},{pula,inh,0},{pulb,inh,0},/* 30 */
	{des, inh,0},{txs, inh,0},{psha,inh,0},{pshb,inh,0},
	{pulx,inh,1},{rts, inh,0},{abx, inh,1},{rti, inh,0},
	{pshx,inh,1},{mul, inh,1},{wai, inh,0},{swi, inh,0},
	{nega,inh,0},{ill, inh,7},{ill, inh,7},{coma,inh,0},/* 40 */
	{lsra,inh,0},{ill, inh,7},{rora,inh,0},{asra,inh,0},
	{asla,inh,0},{rola,inh,0},{deca,inh,0},{ill, inh,7},
	{inca,inh,0},{tsta,inh,0},{ill, inh,7},{clra,inh,0},
	{negb,inh,0},{ill, inh,7},{ill, inh,7},{comb,inh,0},/* 50 */
	{lsrb,inh,0},{ill, inh,7},{rorb,inh,0},{asrb,inh,0},
	{aslb,inh,0},{rolb,inh,0},{decb,inh,0},{ill, inh,7},
	{incb,inh,0},{tstb,inh,0},{ill, inh,7},{clrb,inh,0},
	{neg, idx,0},{aim, imx,3},{oim, imx,3},{com, idx,0},/* 60 */
	{lsr, idx,0},{eim, imx,3},{ror, idx,0},{asr, idx,0},
	{asl, idx,0},{rol, idx,0},{dec, idx,0},{tim, imx,3},
	{inc, idx,0},{tst, idx,0},{jmp, idx,0},{clr, idx,0},
	{neg, ext,0},{aim, imd,3},{oim, imd,3},{com, ext,0},/* 70 */
	{lsr, ext,0},{eim, imd,3},{ror, ext,0},{asr, ext,0},
	{asl, ext,0},{rol, ext,0},{dec, ext,0},{tim, imd,3},
	{inc, ext,0},{tst, ext,0},{jmp, ext,0},{clr, ext,0},
	{suba,imb,0},{cmpa,imb,0},{sbca,imb,0},{subd,imw,1},/* 80 */
	{anda,imb,0},{bita,imb,0},{lda, imb,0},{sta, imb,0},
	{eora,imb,0},{adca,imb,0},{ora, imb,0},{adda,imb,0},
	{cmpx,imw,0},{bsr, rel,0},{lds, imw,0},{sts, imw,0},
	{suba,dir,0},{cmpa,dir,0},{sbca,dir,0},{subd,dir,1},/* 90 */
	{anda,dir,0},{bita,dir,0},{lda, dir,0},{sta, dir,0},
	{eora,dir,0},{adca,dir,0},{ora, dir,0},{adda,dir,0},
	{cmpx,dir,0},{jsr, dir,0},{lds, dir,0},{sts, dir,0},
	{suba,idx,0},{cmpa,idx,0},{sbca,idx,0},{subd,idx,1},/* a0 */
	{anda,idx,0},{bita,idx,0},{lda, idx,0},{sta, idx,0},
	{eora,idx,0},{adca,idx,0},{ora, idx,0},{adda,idx,0},
	{cmpx,idx,0},{jsr, idx,0},{lds, idx,0},{sts, idx,0},
	{suba,ext,0},{cmpa,ext,0},{sbca,ext,0},{subd,ext,1},/* b0 */
	{anda,ext,0},{bita,ext,0},{lda, ext,0},{sta, ext,0},
	{eora,ext,0},{adca,ext,0},{ora, ext,0},{adda,ext,0},
	{cmpx,ext,0},{jsr, ext,0},{lds, ext,0},{sts, ext,0},
	{subb,imb,0},{cmpb,imb,0},{sbcb,imb,0},{addd,imw,1},/* c0 */
	{andb,imb,0},{bitb,imb,0},{ldb, imb,0},{stb, imb,0},
	{eorb,imb,0},{adcb,imb,0},{orb, imb,0},{addb,imb,0},
	{ldd, imw,1},{_std,imw,1},{ldx, imw,0},{stx, imw,0},
	{subb,dir,0},{cmpb,dir,0},{sbcb,dir,0},{addd,dir,1},/* d0 */
	{andb,dir,0},{bitb,dir,0},{ldb, dir,0},{stb, dir,0},
	{eorb,dir,0},{adcb,dir,0},{orb, dir,0},{addb,dir,0},
	{ldd, dir,1},{_std,dir,1},{ldx, dir,0},{stx, dir,0},
	{subb,idx,0},{cmpb,idx,0},{sbcb,idx,0},{addd,idx,1},/* e0 */
	{andb,idx,0},{bitb,idx,0},{ldb, idx,0},{stb, idx,0},
	{eorb,idx,0},{adcb,idx,0},{orb, idx,0},{addb,idx,0},
	{ldd, idx,1},{_std,idx,1},{ldx, idx,0},{stx, idx,0},
	{subb,ext,0},{cmpb,ext,0},{sbcb,ext,0},{addd,ext,1},/* f0 */
	{andb,ext,0},{bitb,ext,0},{ldb, ext,0},{stb, ext,0},
	{eorb,ext,0},{adcb,ext,0},{orb, ext,0},{addb,ext,0},
	{ldd, ext,1},{_std,ext,1},{ldx, ext,0},{stx, ext,0},

	/* extra instruction $fc for NSC-8105 */
	{addx,ext,0},
	/* extra instruction $ec for NSC-8105 */
	{adcx,imb,0}
};

/* some macros to keep things short */
#define OP      oprom[0]
#define ARG1    opram[1]
#define ARG2    opram[2]
#define ARGW    (opram[1]<<8) + opram[2]

static unsigned Dasm680x (int subtype, _TCHAR *buf, unsigned pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol)
{
//	UINT32 flags = 0;
	int invalid_mask;
	int code = OP;
	UINT8 opcode, args, invalid;

	switch( subtype )
	{
		case 6800: case 6802: case 6808: case 8105:
			invalid_mask = 1;
			break;
		case 6801: case 6803:
			invalid_mask = 2;
			break;
		default:
			invalid_mask = 4;
	}

	/* NSC-8105 is a special case */
	if (subtype == 8105)
	{
		/* swap bits */
		code = (code & 0x3c) | ((code & 0x41) << 1) | ((code & 0x82) >> 1);

		/* and check for extra instruction */
		if (code == 0xfc)  code = 0x0100;
		if (code == 0xec)  code = 0x0101;
	}

	opcode = table[code][0];
	args = table[code][1];
	invalid = table[code][2];

//	if (opcode == bsr || opcode == jsr) {
//		flags = DASMFLAG_STEP_OVER;
//	} else if (opcode == rti || opcode == rts) {
//		flags = DASMFLAG_STEP_OUT;
//	}

	if ( invalid & invalid_mask )   /* invalid for this cpu type ? */
	{
		_tcscpy(buf, _T("illegal"));
		return 1;// | flags | DASMFLAG_SUPPORTED;
	}

	buf += _stprintf(buf, _T("%-5s"), op_name_str[opcode]);

	switch( args )
	{
		case rel:  /* relative */
			_stprintf (buf, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), pc + (INT8)ARG1 + 2));
			return 2;// | flags | DASMFLAG_SUPPORTED;
		case imb:  /* immediate (byte) */
			_stprintf (buf, _T("#$%02X"), ARG1);
			return 2;// | flags | DASMFLAG_SUPPORTED;
		case imw:  /* immediate (word) */
			_stprintf (buf, _T("#%s"), get_value_or_symbol(first_symbol, _T("$%04X"), ARGW));
			return 3;// | flags | DASMFLAG_SUPPORTED;
		case idx:  /* indexed + byte offset */
			_stprintf (buf, _T("(x+$%02X)"), ARG1 );
			return 2;// | flags | DASMFLAG_SUPPORTED;
		case imx:  /* immediate, indexed + byte offset */
			_stprintf (buf, _T("#$%02X,(x+$%02x)"), ARG1, ARG2 );
			return 3;// | flags | DASMFLAG_SUPPORTED;
		case dir:  /* direct address */
			_stprintf (buf, _T("$%02X"), ARG1 );
			return 2;// | flags | DASMFLAG_SUPPORTED;
		case imd:  /* immediate, direct address */
			_stprintf (buf, _T("#$%02X,$%02X"), ARG1, ARG2);
			return 3;// | flags | DASMFLAG_SUPPORTED;
		case ext:  /* extended address */
			_stprintf (buf, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), ARGW));
			return 3;// | flags | DASMFLAG_SUPPORTED;
		case sx1:  /* byte from address (s + 1) */
			_stprintf (buf, _T("(s+1)"));
			return 1;// | flags | DASMFLAG_SUPPORTED;
		default:
			return 1;// | flags | DASMFLAG_SUPPORTED;
	}
}

int MC6800::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t ops[4];
	for(int i = 0; i < 4; i++) {
		int wait;
		ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
#if defined(HAS_MC6800)
	return Dasm680x(6800, buffer, pc, ops, ops, d_debugger->first_symbol);
#elif defined(HAS_MC6801)
	return Dasm680x(6801, buffer, pc, ops, ops, d_debugger->first_symbol);
#elif defined(HAS_HD6301)
	return Dasm680x(6301, buffer, pc, ops, ops, d_debugger->first_symbol);
#elif defined(HAS_MB8861)
	return Dasm680x(6800, buffer, pc, ops, ops, d_debugger->first_symbol);	// FIXME
#endif
	return 0;
}
#endif

void MC6800::enter_interrupt(uint16_t irq_vector)
{
	if(wai_state & MC6800_WAI) {
#ifdef USE_DEBUGGER
		total_icount += 4;
#endif
		icount -= 4;
		wai_state &= ~MC6800_WAI;
	} else {
		PUSHWORD(pPC);
		PUSHWORD(pX);
		PUSHBYTE(A);
		PUSHBYTE(B);
		PUSHBYTE(CC);
#ifdef USE_DEBUGGER
		total_icount += 12;
#endif
		icount -= 12;
	}
	SEI;
	PCD = RM16(irq_vector);
}

// opcodes

void MC6800::insn(uint8_t code)
{
	switch(code) {
	case 0x00: illegal(); break;
	case 0x01: nop(); break;
	case 0x02: illegal(); break;
	case 0x03: illegal(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x04: lsrd(); break;
	case 0x05: asld(); break;
#else
	case 0x04: illegal(); break;
	case 0x05: illegal(); break;
#endif
	case 0x06: tap(); break;
	case 0x07: tpa(); break;
	case 0x08: inx(); break;
	case 0x09: dex(); break;
	case 0x0a: clv(); break;
	case 0x0b: sev(); break;
	case 0x0c: clc(); break;
	case 0x0d: sec(); break;
	case 0x0e: cli(); break;
	case 0x0f: sei(); break;
	case 0x10: sba(); break;
	case 0x11: cba(); break;
#if defined(HAS_HD6301)
	case 0x12: undoc1(); break;
	case 0x13: undoc2(); break;
#else
	case 0x12: illegal(); break;
	case 0x13: illegal(); break;
#endif
	case 0x14: illegal(); break;
	case 0x15: illegal(); break;
	case 0x16: tab(); break;
	case 0x17: tba(); break;
#if defined(HAS_HD6301)
	case 0x18: xgdx(); break;
#else
	case 0x18: illegal(); break;
#endif
	case 0x19: daa(); break;
#if defined(HAS_HD6301)
	case 0x1a: slp(); break;
#else
	case 0x1a: illegal(); break;
#endif
	case 0x1b: aba(); break;
	case 0x1c: illegal(); break;
	case 0x1d: illegal(); break;
	case 0x1e: illegal(); break;
	case 0x1f: illegal(); break;
	case 0x20: bra(); break;
	case 0x21: brn(); break;
	case 0x22: bhi(); break;
	case 0x23: bls(); break;
	case 0x24: bcc(); break;
	case 0x25: bcs(); break;
	case 0x26: bne(); break;
	case 0x27: beq(); break;
	case 0x28: bvc(); break;
	case 0x29: bvs(); break;
	case 0x2a: bpl(); break;
	case 0x2b: bmi(); break;
	case 0x2c: bge(); break;
	case 0x2d: blt(); break;
	case 0x2e: bgt(); break;
	case 0x2f: ble(); break;
	case 0x30: tsx(); break;
	case 0x31: ins(); break;
	case 0x32: pula(); break;
	case 0x33: pulb(); break;
	case 0x34: des(); break;
	case 0x35: txs(); break;
	case 0x36: psha(); break;
	case 0x37: pshb(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x38: pulx(); break;
#else
	case 0x38: illegal(); break;
#endif
	case 0x39: rts(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x3a: abx(); break;
#else
	case 0x3a: illegal(); break;
#endif
	case 0x3b: rti(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x3c: pshx(); break;
	case 0x3d: mul(); break;
#else
	case 0x3c: illegal(); break;
	case 0x3d: illegal(); break;
#endif
	case 0x3e: wai(); break;
	case 0x3f: swi(); break;
	case 0x40: nega(); break;
	case 0x41: illegal(); break;
	case 0x42: illegal(); break;
	case 0x43: coma(); break;
	case 0x44: lsra(); break;
	case 0x45: illegal(); break;
	case 0x46: rora(); break;
	case 0x47: asra(); break;
	case 0x48: asla(); break;
	case 0x49: rola(); break;
	case 0x4a: deca(); break;
	case 0x4b: illegal(); break;
	case 0x4c: inca(); break;
	case 0x4d: tsta(); break;
	case 0x4e: illegal(); break;
	case 0x4f: clra(); break;
	case 0x50: negb(); break;
	case 0x51: illegal(); break;
	case 0x52: illegal(); break;
	case 0x53: comb(); break;
	case 0x54: lsrb(); break;
	case 0x55: illegal(); break;
	case 0x56: rorb(); break;
	case 0x57: asrb(); break;
	case 0x58: aslb(); break;
	case 0x59: rolb(); break;
	case 0x5a: decb(); break;
	case 0x5b: illegal(); break;
	case 0x5c: incb(); break;
	case 0x5d: tstb(); break;
	case 0x5e: illegal(); break;
	case 0x5f: clrb(); break;
	case 0x60: neg_ix(); break;
#if defined(HAS_HD6301)
	case 0x61: aim_ix(); break;
	case 0x62: oim_ix(); break;
#else
	case 0x61: illegal(); break;
	case 0x62: illegal(); break;
#endif
	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;
#if defined(HAS_HD6301)
	case 0x65: eim_ix(); break;
#else
	case 0x65: illegal(); break;
#endif
	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;
#if defined(HAS_HD6301)
	case 0x6b: tim_ix(); break;
#else
	case 0x6b: illegal(); break;
#endif
	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;
#if defined(HAS_HD6301)
	case 0x71: aim_di(); break;
	case 0x72: oim_di(); break;
#elif defined(HAS_MB8861)
	case 0x71: nim_ix(); break;
	case 0x72: oim_ix_mb8861(); break;
#else
	case 0x71: illegal(); break;
	case 0x72: illegal(); break;
#endif
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;
#if defined(HAS_HD6301)
	case 0x75: eim_di(); break;
#elif defined(HAS_MB8861)
	case 0x75: xim_ix(); break;
#else
	case 0x75: illegal(); break;
#endif
	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;
#if defined(HAS_HD6301)
	case 0x7b: tim_di(); break;
#elif defined(HAS_MB8861)
	case 0x7b: tmm_ix(); break;
#else
	case 0x7b: illegal(); break;
#endif
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x83: subd_im(); break;
#else
	case 0x83: illegal(); break;
#endif
	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x8c: cpx_im (); break;
#else
	case 0x8c: cmpx_im(); break;
#endif
	case 0x8d: bsr(); break;
	case 0x8e: lds_im(); break;
	case 0x8f: sts_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x93: subd_di(); break;
#else
	case 0x93: illegal(); break;
#endif
	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x9c: cpx_di (); break;
#else
	case 0x9c: cmpx_di(); break;
#endif
	case 0x9d: jsr_di(); break;
	case 0x9e: lds_di(); break;
	case 0x9f: sts_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xa3: subd_ix(); break;
#else
	case 0xa3: illegal(); break;
#endif
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xac: cpx_ix (); break;
#else
	case 0xac: cmpx_ix(); break;
#endif
	case 0xad: jsr_ix(); break;
	case 0xae: lds_ix(); break;
	case 0xaf: sts_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xb3: subd_ex(); break;
#else
	case 0xb3: illegal(); break;
#endif
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xbc: cpx_ex (); break;
#else
	case 0xbc: cmpx_ex(); break;
#endif
	case 0xbd: jsr_ex(); break;
	case 0xbe: lds_ex(); break;
	case 0xbf: sts_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xc3: addd_im(); break;
#else
	case 0xc3: illegal(); break;
#endif
	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xcc: ldd_im(); break;
	case 0xcd: std_im(); break;
#else
	case 0xcc: illegal(); break;
	case 0xcd: illegal(); break;
#endif
	case 0xce: ldx_im(); break;
	case 0xcf: stx_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xd3: addd_di(); break;
#else
	case 0xd3: illegal(); break;
#endif
	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xdc: ldd_di(); break;
	case 0xdd: std_di(); break;
#else
	case 0xdc: illegal(); break;
	case 0xdd: illegal(); break;
#endif
	case 0xde: ldx_di(); break;
	case 0xdf: stx_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xe3: addd_ix(); break;
#else
	case 0xe3: illegal(); break;
#endif
	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xec: ldd_ix(); break;
	case 0xed: std_ix(); break;
#elif defined(HAS_MB8861)
	case 0xec: adx_im(); break;
	case 0xed: illegal(); break;
#else
	case 0xec: illegal(); break;
	case 0xed: illegal(); break;
#endif
	case 0xee: ldx_ix(); break;
	case 0xef: stx_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xf3: addd_ex(); break;
#else
	case 0xf3: illegal(); break;
#endif
	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xfc: ldd_ex(); break;
	case 0xfd: std_ex(); break;
#elif defined(HAS_MB8861)
	case 0xfc: adx_ex(); break;
	case 0xfd: illegal(); break;
#else
	case 0xfc: illegal(); break;
	case 0xfd: illegal(); break;
#endif
	case 0xfe: ldx_ex(); break;
	case 0xff: stx_ex(); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

/* operate one instruction for */
#define ONE_MORE_INSN() { \
	uint8_t ireg = M_RDOP(PCD); \
	prevpc = PC; \
	PC++; \
	insn(ireg); \
	increment_counter(cycles[ireg]); \
}

/* $00 ILLEGAL */
void MC6800::illegal()
{
#ifdef HAS_HD6301
	TAKE_TRAP;
#endif
}

/* $01 NOP */
void MC6800::nop()
{
	
}

/* $02 ILLEGAL */

/* $03 ILLEGAL */

/* $04 LSRD inherent -0*-* */
void MC6800::lsrd()
{
	uint16_t t;
	CLR_NZC;
	t = D;
	CC |= (t & 0x0001);
	t >>= 1;
	SET_Z16(t);
	D = t;
}

/* $05 ASLD inherent ?**** */
void MC6800::asld()
{
	int r;
	uint16_t t;
	t = D;
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS16(t, t, r);
	D = r;
}

/* $06 TAP inherent ##### */
void MC6800::tap()
{
	CC = A;
//	ONE_MORE_INSN();
	one_more_insn = true;
}

/* $07 TPA inherent ----- */
void MC6800::tpa()
{
	A = CC;
}

/* $08 INX inherent --*-- */
void MC6800::inx()
{
	++X;
	CLR_Z;
	SET_Z16(X);
}

/* $09 DEX inherent --*-- */
void MC6800::dex()
{
	--X;
	CLR_Z;
	SET_Z16(X);
}

/* $0a CLV */
void MC6800::clv()
{
	CLV;
}

/* $0b SEV */
void MC6800::sev()
{
	SEV;
}

/* $0c CLC */
void MC6800::clc()
{
	CLC;
}

/* $0d SEC */
void MC6800::sec()
{
	SEC;
}

/* $0e CLI */
void MC6800::cli()
{
	CLI;
//	ONE_MORE_INSN();
	one_more_insn = true;
}

/* $0f SEI */
void MC6800::sei()
{
	SEI;
//	ONE_MORE_INSN();
	one_more_insn = true;
}

/* $10 SBA inherent -**** */
void MC6800::sba()
{
	uint16_t t;
	t = A - B;
	CLR_NZVC;
	SET_FLAGS8(A, B, t);
	A = (uint8_t)t;
}

/* $11 CBA inherent -**** */
void MC6800::cba()
{
	uint16_t t;
	t = A - B;
	CLR_NZVC;
	SET_FLAGS8(A, B, t);
}

/* $12 ILLEGAL */
void MC6800::undoc1()
{
	X += RM(S + 1);
}

/* $13 ILLEGAL */
void MC6800::undoc2()
{
	X += RM(S + 1);
}

/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 TAB inherent -**0- */
void MC6800::tab()
{
	B=A;
	CLR_NZV;
	SET_NZ8(B);
}

/* $17 TBA inherent -**0- */
void MC6800::tba()
{
	A = B;
	CLR_NZV;
	SET_NZ8(A);
}

/* $18 XGDX inherent ----- */ /* HD6301 only */
void MC6800::xgdx()
{
	uint16_t t = X;
	X = D;
	D = t;
}

/* $19 DAA inherent (A) -**0* */
void MC6800::daa()
{
	uint8_t msn, lsn;
	uint16_t t, cf = 0;
	msn = A & 0xf0;
	lsn = A & 0x0f;
	if(lsn > 0x09 || CC & 0x20) {
		cf |= 0x06;
	}
	if(msn > 0x80 && lsn > 0x09) {
		cf |= 0x60;
	}
	if(msn > 0x90 || (CC & 0x01)) {
		cf |= 0x60;
	}
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((uint8_t)t);
	SET_C8(t);
	A = (uint8_t)t;
}

/* $1a ILLEGAL */

/* $1a SLP */ /* HD6301 only */
void MC6800::slp()
{
	/* wait for next IRQ (same as waiting of wai) */
	wai_state |= HD6301_SLP;
}

/* $1b ABA inherent ***** */
void MC6800::aba()
{
	uint16_t t;
	t = A + B;
	CLR_HNZVC;
	SET_FLAGS8(A, B, t);
	SET_H(A, B, t);
	A = (uint8_t)t;
}

/* $1c ILLEGAL */

/* $1d ILLEGAL */

/* $1e ILLEGAL */

/* $1f ILLEGAL */

/* $20 BRA relative ----- */
void MC6800::bra()
{
	uint8_t t;
	IMMBYTE(t);
	PC += SIGNED(t);
}

/* $21 BRN relative ----- */
void MC6800::brn()
{
	uint8_t t;
	IMMBYTE(t);
}

/* $22 BHI relative ----- */
void MC6800::bhi()
{
	uint8_t t;
	BRANCH(!(CC & 0x05));
}

/* $23 BLS relative ----- */
void MC6800::bls()
{
	uint8_t t;
	BRANCH(CC & 0x05);
}

/* $24 BCC relative ----- */
void MC6800::bcc()
{
	uint8_t t;
	BRANCH(!(CC & 0x01));
}

/* $25 BCS relative ----- */
void MC6800::bcs()
{
	uint8_t t;
	BRANCH(CC & 0x01);
}

/* $26 BNE relative ----- */
void MC6800::bne()
{
	uint8_t t;
	BRANCH(!(CC & 0x04));
}

/* $27 BEQ relative ----- */
void MC6800::beq()
{
	uint8_t t;
	BRANCH(CC & 0x04);
}

/* $28 BVC relative ----- */
void MC6800::bvc()
{
	uint8_t t;
	BRANCH(!(CC & 0x02));
}

/* $29 BVS relative ----- */
void MC6800::bvs()
{
	uint8_t t;
	BRANCH(CC & 0x02);
}

/* $2a BPL relative ----- */
void MC6800::bpl()
{
	uint8_t t;
	BRANCH(!(CC & 0x08));
}

/* $2b BMI relative ----- */
void MC6800::bmi()
{
	uint8_t t;
	BRANCH(CC & 0x08);
}

/* $2c BGE relative ----- */
void MC6800::bge()
{
	uint8_t t;
	BRANCH(!NXORV);
}

/* $2d BLT relative ----- */
void MC6800::blt()
{
	uint8_t t;
	BRANCH(NXORV);
}

/* $2e BGT relative ----- */
void MC6800::bgt()
{
	uint8_t t;
	BRANCH(!(NXORV||CC & 0x04));
}

/* $2f BLE relative ----- */
void MC6800::ble()
{
	uint8_t t;
	BRANCH(NXORV||CC & 0x04);
}

/* $30 TSX inherent ----- */
void MC6800::tsx()
{
	X = (S + 1);
}

/* $31 INS inherent ----- */
void MC6800::ins()
{
	++S;
}

/* $32 PULA inherent ----- */
void MC6800::pula()
{
	PULLBYTE(A);
}

/* $33 PULB inherent ----- */
void MC6800::pulb()
{
	PULLBYTE(B);
}

/* $34 DES inherent ----- */
void MC6800::des()
{
	--S;
}

/* $35 TXS inherent ----- */
void MC6800::txs()
{
	S = (X - 1);
}

/* $36 PSHA inherent ----- */
void MC6800::psha()
{
	PUSHBYTE(A);
}

/* $37 PSHB inherent ----- */
void MC6800::pshb()
{
	PUSHBYTE(B);
}

/* $38 PULX inherent ----- */
void MC6800::pulx()
{
	PULLWORD(pX);
}

/* $39 RTS inherent ----- */
void MC6800::rts()
{
	PULLWORD(pPC);
}

/* $3a ABX inherent ----- */
void MC6800::abx()
{
	X += B;
}

/* $3b RTI inherent ##### */
void MC6800::rti()
{
	PULLBYTE(CC);
	PULLBYTE(B);
	PULLBYTE(A);
	PULLWORD(pX);
	PULLWORD(pPC);
}

/* $3c PSHX inherent ----- */
void MC6800::pshx()
{
	PUSHWORD(pX);
}

/* $3d MUL inherent --*-@ */
void MC6800::mul()
{
	uint16_t t;
	t = A*B;
	CLR_C;
	if(t & 0x80) SEC;
	D = t;
}

/* $3e WAI inherent ----- */
void MC6800::wai()
{
	/*
	 * WAI stacks the entire machine state on the
	 * hardware stack, then waits for an interrupt.
	 */
	wai_state |= MC6800_WAI;
	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
	PUSHBYTE(CC);
}

/* $3f SWI absolute indirect ----- */
void MC6800::swi()
{
	PUSHWORD(pPC);
	PUSHWORD(pX);
	PUSHBYTE(A);
	PUSHBYTE(B);
	PUSHBYTE(CC);
	SEI;
	PCD = RM16(0xfffa);
}

/* $40 NEGA inherent ?**** */
void MC6800::nega()
{
	uint16_t r;
	r = -A;
	CLR_NZVC;
	SET_FLAGS8(0, A, r);
	A = (uint8_t)r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
void MC6800::coma()
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0*-* */
void MC6800::lsra()
{
	CLR_NZC;
	CC |= (A & 0x01);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
void MC6800::rora()
{
	uint8_t r;
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (A & 0x01);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?**-* */
void MC6800::asra()
{
	CLR_NZC;
	CC |= (A & 0x01);
	A >>= 1;
	A |= ((A & 0x40) << 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
void MC6800::asla()
{
	uint16_t r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A, A, r);
	A = (uint8_t)r;
}

/* $49 ROLA inherent -**** */
void MC6800::rola()
{
	uint16_t t, r;
	t = A;
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	A = (uint8_t)r;
}

/* $4a DECA inherent -***- */
void MC6800::deca()
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}

/* $4b ILLEGAL */

/* $4c INCA inherent -***- */
void MC6800::inca()
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4d TSTA inherent -**0- */
void MC6800::tsta()
{
	CLR_NZVC;
	SET_NZ8(A);
}

/* $4e ILLEGAL */

/* $4f CLRA inherent -0100 */
void MC6800::clra()
{
	A = 0;
	CLR_NZVC;
	SEZ;
}

/* $50 NEGB inherent ?**** */
void MC6800::negb()
{
	uint16_t r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0, B, r);
	B = (uint8_t)r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
void MC6800::comb()
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $54 LSRB inherent -0*-* */
void MC6800::lsrb()
{
	CLR_NZC;
	CC |= (B & 0x01);
	B >>= 1;
	SET_Z8(B);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
void MC6800::rorb()
{
	uint8_t r;
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (B & 0x01);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $57 ASRB inherent ?**-* */
void MC6800::asrb()
{
	CLR_NZC;
	CC |= (B & 0x01);
	B >>= 1;
	B |= ((B & 0x40) << 1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
void MC6800::aslb()
{
	uint16_t r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B, B, r);
	B = (uint8_t)r;
}

/* $59 ROLB inherent -**** */
void MC6800::rolb()
{
	uint16_t t, r;
	t = B;
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	B = (uint8_t)r;
}

/* $5a DECB inherent -***- */
void MC6800::decb()
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
}

/* $5b ILLEGAL */

/* $5c INCB inherent -***- */
void MC6800::incb()
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $5d TSTB inherent -**0- */
void MC6800::tstb()
{
	CLR_NZVC;
	SET_NZ8(B);
}

/* $5e ILLEGAL */

/* $5f CLRB inherent -0100 */
void MC6800::clrb()
{
	B=0;
	CLR_NZVC;
	SEZ;
}

/* $60 NEG indexed ?**** */
void MC6800::neg_ix()
{
	uint16_t r, t;
	IDXBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	WM(EAD, r);
}

/* $61 AIM --**0- */ /* HD6301 only */
void MC6800::aim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $62 OIM --**0- */ /* HD6301 only */
void MC6800::oim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $63 COM indexed -**01 */
void MC6800::com_ix()
{
	uint8_t t;
	IDXBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD, t);
}

/* $64 LSR indexed -0*-* */
void MC6800::lsr_ix()
{
	uint8_t t;
	IDXBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	SET_Z8(t);
	WM(EAD, t);
}

/* $65 EIM --**0- */ /* HD6301 only */
void MC6800::eim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $66 ROR indexed -**-* */
void MC6800::ror_ix()
{
	uint8_t t, r;
	IDXBYTE(t);
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (t & 0x01);
	r |= t >> 1;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $67 ASR indexed ?**-* */
void MC6800::asr_ix()
{
	uint8_t t;
	IDXBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	t |= ((t & 0x40) << 1);
	SET_NZ8(t);
	WM(EAD, t);
}

/* $68 ASL indexed ?**** */
void MC6800::asl_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $69 ROL indexed -**** */
void MC6800::rol_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $6a DEC indexed -***- */
void MC6800::dec_ix()
{
	uint8_t t;
	IDXBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD, t);
}

/* $6b TIM --**0- */ /* HD6301 only */
void MC6800::tim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $6c INC indexed -***- */
void MC6800::inc_ix()
{
	uint8_t t;
	IDXBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD, t);
}

/* $6d TST indexed -**0- */
void MC6800::tst_ix()
{
	uint8_t t;
	IDXBYTE(t);
	CLR_NZVC;
	SET_NZ8(t);
}

/* $6e JMP indexed ----- */
void MC6800::jmp_ix()
{
	INDEXED;
	PC = EA;
}

/* $6f CLR indexed -0100 */
void MC6800::clr_ix()
{
	INDEXED;
	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

/* $70 NEG extended ?**** */
void MC6800::neg_ex()
{
	uint16_t r, t;
	EXTBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	WM(EAD, r);
}

/* $71 AIM --**0- */ /* HD6301 only */
void MC6800::aim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $71 NIM --**0- */ /* MB8861 only */
void MC6800::nim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}

/* $72 OIM --**0- */ /* HD6301 only */
void MC6800::oim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r |= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $72 OIM --**0- */ /* MB8861 only */
void MC6800::oim_ix_mb8861()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}

/* $73 COM extended -**01 */
void MC6800::com_ex()
{
	uint8_t t;
	EXTBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD, t);
}

/* $74 LSR extended -0*-* */
void MC6800::lsr_ex()
{
	uint8_t t;
	EXTBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	SET_Z8(t);
	WM(EAD, t);
}

/* $75 EIM --**0- */ /* HD6301 only */
void MC6800::eim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r ^= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $75 XIM --**-- */ /* MB8861 only */
void MC6800::xim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZ;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}

/* $76 ROR extended -**-* */
void MC6800::ror_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	r = (CC & 0x01) << 7;
	CLR_NZC;
	CC |= (t & 0x01);
	r |= t >> 1;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $77 ASR extended ?**-* */
void MC6800::asr_ex()
{
	uint8_t t;
	EXTBYTE(t);
	CLR_NZC;
	CC |= (t & 0x01);
	t >>= 1;
	t |= ((t & 0x40) << 1);
	SET_NZ8(t);
	WM(EAD, t);
}

/* $78 ASL extended ?**** */
void MC6800::asl_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $79 ROL extended -**** */
void MC6800::rol_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = CC & 0x01;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $7a DEC extended -***- */
void MC6800::dec_ex()
{
	uint8_t t;
	EXTBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD, t);
}

/* $7b TIM --**0- */ /* HD6301 only */
void MC6800::tim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $7b TMM --***- */ /* MB8861 only */
void MC6800::tmm_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	if(!t || !r) {
		SEZ;
	} else if(r == t) {
		SEV;
	} else {
		SEN;
	}
}

/* $7c INC extended -***- */
void MC6800::inc_ex()
{
	uint8_t t;
	EXTBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD, t);
}

/* $7d TST extended -**0- */
void MC6800::tst_ex()
{
	uint8_t t;
	EXTBYTE(t);
	CLR_NZVC;
	SET_NZ8(t);
}

/* $7e JMP extended ----- */
void MC6800::jmp_ex()
{
	EXTENDED;
	PC = EA;
}

/* $7f CLR extended -0100 */
void MC6800::clr_ex()
{
	EXTENDED;
	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

/* $80 SUBA immediate ?**** */
void MC6800::suba_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $81 CMPA immediate ?**** */
void MC6800::cmpa_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $82 SBCA immediate ?**** */
void MC6800::sbca_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $83 SUBD immediate -**** */
void MC6800::subd_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $84 ANDA immediate -**0- */
void MC6800::anda_im()
{
	uint8_t t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
void MC6800::bita_im()
{
	uint8_t t, r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
void MC6800::lda_im()
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
void MC6800::sta_im()
{
	CLR_NZV;
	SET_NZ8(A);
	IMM8;
	WM(EAD, A);
}

/* $88 EORA immediate -**0- */
void MC6800::eora_im()
{
	uint8_t t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
void MC6800::adca_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $8a ORA immediate -**0- */
void MC6800::ora_im()
{
	uint8_t t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8b ADDA immediate ***** */
void MC6800::adda_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $8c CMPX immediate -***- */
void MC6800::cmpx_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r);
	SET_V16(d, b.d, r);
}

/* $8c CPX immediate -**** (6801) */
void MC6800::cpx_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}


/* $8d BSR ----- */
void MC6800::bsr()
{
	uint8_t t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
}

/* $8e LDS immediate -**0- */
void MC6800::lds_im()
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
}

/* $8f STS immediate -**0- */
void MC6800::sts_im()
{
	CLR_NZV;
	SET_NZ16(S);
	IMM16;
	WM16(EAD, &pS);
}

/* $90 SUBA direct ?**** */
void MC6800::suba_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $91 CMPA direct ?**** */
void MC6800::cmpa_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $92 SBCA direct ?**** */
void MC6800::sbca_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $93 SUBD direct -**** */
void MC6800::subd_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $94 ANDA direct -**0- */
void MC6800::anda_di()
{
	uint8_t t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
void MC6800::bita_di()
{
	uint8_t t, r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
void MC6800::lda_di()
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
void MC6800::sta_di()
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD, A);
}

/* $98 EORA direct -**0- */
void MC6800::eora_di()
{
	uint8_t t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
void MC6800::adca_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $9a ORA direct -**0- */
void MC6800::ora_di()
{
	uint8_t t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9b ADDA direct ***** */
void MC6800::adda_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $9c CMPX direct -***- */
void MC6800::cmpx_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r);
	SET_V16(d, b.d, r);
}

/* $9c CPX direct -**** (6801) */
void MC6800::cpx_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $9d JSR direct ----- */
void MC6800::jsr_di()
{
	DIRECT;
	PUSHWORD(pPC);
	PC = EA;
}

/* $9e LDS direct -**0- */
void MC6800::lds_di()
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
}

/* $9f STS direct -**0- */
void MC6800::sts_di()
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD, &pS);
}

/* $a0 SUBA indexed ?**** */
void MC6800::suba_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $a1 CMPA indexed ?**** */
void MC6800::cmpa_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $a2 SBCA indexed ?**** */
void MC6800::sbca_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $a3 SUBD indexed -**** */
void MC6800::subd_ix()
{
	uint32_t r, d;
	pair32_t b;
	IDXWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $a4 ANDA indexed -**0- */
void MC6800::anda_ix()
{
	uint8_t t;
	IDXBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
void MC6800::bita_ix()
{
	uint8_t t, r;
	IDXBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
void MC6800::lda_ix()
{
	IDXBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
void MC6800::sta_ix()
{
	CLR_NZV;
	SET_NZ8(A);
	INDEXED;
	WM(EAD, A);
}

/* $a8 EORA indexed -**0- */
void MC6800::eora_ix()
{
	uint8_t t;
	IDXBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
void MC6800::adca_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $aa ORA indexed -**0- */
void MC6800::ora_ix()
{
	uint8_t t;
	IDXBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $ab ADDA indexed ***** */
void MC6800::adda_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $ac CMPX indexed -***- */
void MC6800::cmpx_ix()
{
	uint32_t r, d;
	pair32_t b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r);
	SET_V16(d, b.d, r);
}

/* $ac CPX indexed -**** (6801)*/
void MC6800::cpx_ix()
{
	uint32_t r, d;
	pair32_t b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $ad JSR indexed ----- */
void MC6800::jsr_ix()
{
	INDEXED;
	PUSHWORD(pPC);
	PC = EA;
}

/* $ae LDS indexed -**0- */
void MC6800::lds_ix()
{
	IDXWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
}

/* $af STS indexed -**0- */
void MC6800::sts_ix()
{
	CLR_NZV;
	SET_NZ16(S);
	INDEXED;
	WM16(EAD, &pS);
}

/* $b0 SUBA extended ?**** */
void MC6800::suba_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $b1 CMPA extended ?**** */
void MC6800::cmpa_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $b2 SBCA extended ?**** */
void MC6800::sbca_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8_t)r;
}

/* $b3 SUBD extended -**** */
void MC6800::subd_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $b4 ANDA extended -**0- */
void MC6800::anda_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
void MC6800::bita_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
void MC6800::lda_ex()
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
void MC6800::sta_ex()
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD, A);
}

/* $b8 EORA extended -**0- */
void MC6800::eora_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
void MC6800::adca_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $ba ORA extended -**0- */
void MC6800::ora_ex()
{
	uint8_t t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bb ADDA extended ***** */
void MC6800::adda_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8_t)r;
}

/* $bc CMPX extended -***- */
void MC6800::cmpx_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZV;
	SET_NZ16(r);
	SET_V16(d, b.d, r);
}

/* $bc CPX extended -**** (6801) */
void MC6800::cpx_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $bd JSR extended ----- */
void MC6800::jsr_ex()
{
	EXTENDED;
	PUSHWORD(pPC);
	PC = EA;
}

/* $be LDS extended -**0- */
void MC6800::lds_ex()
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
}

/* $bf STS extended -**0- */
void MC6800::sts_ex()
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD, &pS);
}

/* $c0 SUBB immediate ?**** */
void MC6800::subb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $c1 CMPB immediate ?**** */
void MC6800::cmpb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $c2 SBCB immediate ?**** */
void MC6800::sbcb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $c3 ADDD immediate -**** */
void MC6800::addd_im()
{
	uint32_t r, d;
	pair32_t b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
void MC6800::andb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
void MC6800::bitb_im()
{
	uint8_t t, r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
void MC6800::ldb_im()
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
void MC6800::stb_im()
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD, B);
}

/* $c8 EORB immediate -**0- */
void MC6800::eorb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
void MC6800::adcb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $ca ORB immediate -**0- */
void MC6800::orb_im()
{
	uint8_t t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cb ADDB immediate ***** */
void MC6800::addb_im()
{
	uint16_t t, r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $CC LDD immediate -**0- */
void MC6800::ldd_im()
{
	IMMWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cd STD immediate -**0- */
void MC6800::std_im()
{
	IMM16;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $ce LDX immediate -**0- */
void MC6800::ldx_im()
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $cf STX immediate -**0- */
void MC6800::stx_im()
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD, &pX);
}

/* $d0 SUBB direct ?**** */
void MC6800::subb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $d1 CMPB direct ?**** */
void MC6800::cmpb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $d2 SBCB direct ?**** */
void MC6800::sbcb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $d3 ADDD direct -**** */
void MC6800::addd_di()
{
	uint32_t r, d;
	pair32_t b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $d4 ANDB direct -**0- */
void MC6800::andb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
void MC6800::bitb_di()
{
	uint8_t t, r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
void MC6800::ldb_di()
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
void MC6800::stb_di()
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD, B);
}

/* $d8 EORB direct -**0- */
void MC6800::eorb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
void MC6800::adcb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $da ORB direct -**0- */
void MC6800::orb_di()
{
	uint8_t t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $db ADDB direct ***** */
void MC6800::addb_di()
{
	uint16_t t, r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $dc LDD direct -**0- */
void MC6800::ldd_di()
{
	DIRWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dd STD direct -**0- */
void MC6800::std_di()
{
	DIRECT;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $de LDX direct -**0- */
void MC6800::ldx_di()
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $dF STX direct -**0- */
void MC6800::stx_di()
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD, &pX);
}

/* $e0 SUBB indexed ?**** */
void MC6800::subb_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $e1 CMPB indexed ?**** */
void MC6800::cmpb_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $e2 SBCB indexed ?**** */
void MC6800::sbcb_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = B - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $e3 ADDD indexed -**** */
void MC6800::addd_ix()
{
	uint32_t r, d;
	pair32_t b;
	IDXWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
void MC6800::andb_ix()
{
	uint8_t t;
	IDXBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
void MC6800::bitb_ix()
{
	uint8_t t, r;
	IDXBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
void MC6800::ldb_ix()
{
	IDXBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
void MC6800::stb_ix()
{
	CLR_NZV;
	SET_NZ8(B);
	INDEXED;
	WM(EAD, B);
}

/* $e8 EORB indexed -**0- */
void MC6800::eorb_ix()
{
	uint8_t t;
	IDXBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
void MC6800::adcb_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = B + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $ea ORB indexed -**0- */
void MC6800::orb_ix()
{
	uint8_t t;
	IDXBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
void MC6800::addb_ix()
{
	uint16_t t, r;
	IDXBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $ec LDD indexed -**0- */
void MC6800::ldd_ix()
{
	IDXWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $ec ADX immediate -**** */ /* MB8861 only */
void MC6800::adx_im()
{
	uint32_t r, d, t;
	IMMBYTE(t);
	d = X;
	r = d + t;
	CLR_NZVC;
	SET_FLAGS16(d, t, r);
	X = r;
}

/* $ed STD indexed -**0- */
void MC6800::std_ix()
{
	INDEXED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $ee LDX indexed -**0- */
void MC6800::ldx_ix()
{
	IDXWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ef STX indexed -**0- */
void MC6800::stx_ix()
{
	CLR_NZV;
	SET_NZ16(X);
	INDEXED;
	WM16(EAD, &pX);
}

/* $f0 SUBB extended ?**** */
void MC6800::subb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $f1 CMPB extended ?**** */
void MC6800::cmpb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $f2 SBCB extended ?**** */
void MC6800::sbcb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B - t - (CC & 0x01);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8_t)r;
}

/* $f3 ADDD extended -**** */
void MC6800::addd_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $f4 ANDB extended -**0- */
void MC6800::andb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
void MC6800::bitb_ex()
{
	uint8_t t, r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
void MC6800::ldb_ex()
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
void MC6800::stb_ex()
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD, B);
}

/* $f8 EORB extended -**0- */
void MC6800::eorb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
void MC6800::adcb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B + t + (CC & 0x01);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $fa ORB extended -**0- */
void MC6800::orb_ex()
{
	uint8_t t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fb ADDB extended ***** */
void MC6800::addb_ex()
{
	uint16_t t, r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8_t)r;
}

/* $fc LDD extended -**0- */
void MC6800::ldd_ex()
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fc ADX immediate -**** */ /* MB8861 only */
void MC6800::adx_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = X;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	X = r;
}

/* $fd STD extended -**0- */
void MC6800::std_ex()
{
	EXTENDED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $fe LDX extended -**0- */
void MC6800::ldx_ex()
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $ff STX extended -**0- */
void MC6800::stx_ex()
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD, &pX);
}

#define STATE_VERSION	2

bool MC6800::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(pc.d);
	state_fio->StateValue(prevpc);
	state_fio->StateValue(sp.d);
	state_fio->StateValue(ix.d);
	state_fio->StateValue(acc_d.d);
	state_fio->StateValue(ea.d);
	state_fio->StateValue(cc);
	state_fio->StateValue(wai_state);
	state_fio->StateValue(int_state);
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_icount);
#endif
	state_fio->StateValue(icount);
#if defined(HAS_MC6801) || defined(HAS_HD6301)
	for(int i = 0; i < 4; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].ddr);
		state_fio->StateValue(port[i].latched_data);
		state_fio->StateValue(port[i].latched);
		state_fio->StateValue(port[i].first_write);
	}
	state_fio->StateValue(p3csr);
	state_fio->StateValue(p3csr_is3_flag_read);
	state_fio->StateValue(sc1_state);
	state_fio->StateValue(sc2_state);
	state_fio->StateValue(counter.d);
	state_fio->StateValue(output_compare.d);
	state_fio->StateValue(timer_over.d);
	state_fio->StateValue(tcsr);
	state_fio->StateValue(pending_tcsr);
	state_fio->StateValue(input_capture);
#ifdef HAS_HD6301
	state_fio->StateValue(latch09);
#endif
	state_fio->StateValue(timer_next);
	if(!recv_buffer->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(trcsr);
	state_fio->StateValue(rdr);
	state_fio->StateValue(tdr);
	state_fio->StateValue(trcsr_read_tdre);
	state_fio->StateValue(trcsr_read_orfe);
	state_fio->StateValue(trcsr_read_rdrf);
	state_fio->StateValue(rmcr);
	state_fio->StateValue(sio_counter);
	state_fio->StateValue(ram_ctrl);
	state_fio->StateArray(ram, sizeof(ram), 1);
#endif
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		prev_total_icount = total_icount;
	}
#endif
	return true;
}

