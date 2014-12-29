/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#include "upd7801.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define PRESCALER	16

#define VA	regs[0].w.l
#define BC	regs[1].w.l
#define DE 	regs[2].w.l
#define HL	regs[3].w.l

#define _V	regs[0].b.h
#define _A	regs[0].b.l
#define _B	regs[1].b.h
#define _C	regs[1].b.l
#define _D	regs[2].b.h
#define _E	regs[2].b.l
#define _H	regs[3].b.h
#define _L	regs[3].b.l

#define altVA	regs[4].w.l
#define altBC	regs[5].w.l
#define altDE 	regs[6].w.l
#define altHL	regs[7].w.l

#define altV	regs[4].b.h
#define altA	regs[4].b.l
#define altB	regs[5].b.h
#define altC	regs[5].b.l
#define altD	regs[6].b.h
#define altE	regs[6].b.l
#define altH	regs[7].b.h
#define altL	regs[7].b.l

#define F_CY	0x01
#define F_L0	0x04
#define F_L1	0x08
#define F_HC	0x10
#define F_SK	0x20
#define F_Z	0x40

#define INTF0	0x01
#define INTFT	0x02
#define INTF1	0x04
#define INTF2	0x08
#define INTFS	0x10

static const uint8 irq_bits[5] = {
	INTF0, INTFT, INTF1, INTF2, INTFS
};

static const uint16 irq_addr[5] = {
	0x0004, 0x0008, 0x0010, 0x0020, 0x0040
};

typedef struct {
	int oplen;	// bytes of opecode
	int clock;	// clock
} op_t;

static const op_t op[256] = {
	{1, 4}, {1, 6}, {1, 7}, {1, 7}, {3,10}, {3,16}, {1, 4}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{1, 4}, {1, 4}, {1, 7}, {1, 7}, {3,10}, {3,16}, {2, 7}, {2, 7}, {1,11}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4}, {1, 4},
	{2,13}, {1,19}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{2,13}, {1,13}, {1, 7}, {1, 7}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7}, {1, 7},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,16}, {3,13}, {2, 7}, {2, 7}, {0, 0}, {2,10}, {2,10}, {2,10}, {0, 0}, {0, 0}, {2,17}, {2,17},
	{1, 4}, {1, 4}, {1, 4}, {1, 4}, {3,10}, {3,13}, {2, 7}, {2, 7}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{0, 0}, {1, 4}, {1,15}, {1,13}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7}, {2, 7},
	{0, 0}, {3,13}, {1,19}, {1, 4}, {0, 0}, {3,13}, {2, 7}, {2, 7}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16}, {2,16},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19}, {1,19},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13},
	{1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}, {1,13}

};
static const op_t op48[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,17}, {2,17}, {2, 8}, {2, 8}, {2,11}, {2, 8}, {2,17}, {2,15},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4c[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op4d[256] = {
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10},
	{2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2,10}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op60[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op64[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11}, {3,11},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,17}, {3,17}, {3,17}, {3,17}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {3,14}, {3,14}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};
static const op_t op70[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,20}, {4,20},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17}, {4,17},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11},
	{2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2, 8}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}, {2,11}
};
static const op_t op74[256] = {
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8},
	{3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {3,14}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}, {2, 8}
};

// flag control

#define ZHC_ADD(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} else if(a < b) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) < (b & 15)) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define ZHC_SUB(a, b, c) { \
	if(a) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if(a == b) { \
		PSW = (PSW & ~F_CY) | (c); \
	} else if(a > b) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if((a & 15) > (b & 15)) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define SET_Z(n) { \
	if(n) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define SKIP_CY { \
	if(PSW & F_CY) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NC { \
	if(!(PSW & F_CY)) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_Z { \
	if(PSW & F_Z) { \
		PSW |= F_SK; \
	} \
}
#define SKIP_NZ { \
	if(!(PSW & F_Z)) { \
		PSW |= F_SK; \
	} \
}

// memory

inline uint8 UPD7801::RM8(uint16 addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8 val = d_mem->read_data8w(addr, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(addr);
#endif
}

inline void UPD7801::WM8(uint16 addr, uint8 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(addr, val, &wait);
	period += wait;
#else
	d_mem->write_data8(addr, val);
#endif
}

inline uint16 UPD7801::RM16(uint16 addr)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16 val = d_mem->read_data16w(addr, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data16(addr);
#endif
}

inline void UPD7801::WM16(uint16 addr, uint16 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(addr, val, &wait);
	period += wait;
#else
	d_mem->write_data16(addr, val);
#endif
}

inline uint8 UPD7801::FETCH8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8 val = d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(PC++);
#endif
}

inline uint16 UPD7801::FETCH16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16 val = d_mem->read_data16w(PC, &wait);
	period += wait;
#else
	uint16 val = d_mem->read_data16(PC);
#endif
	PC += 2;
	return val;
}

inline uint16 UPD7801::FETCHWA()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16 val = (_V << 8) | d_mem->read_data8w(PC++, &wait);
	period += wait;
	return val;
#else
	return (_V << 8) | d_mem->read_data8(PC++);
#endif
}

inline uint8 UPD7801::POP8()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint8 val = d_mem->read_data8w(SP++, &wait);
	period += wait;
	return val;
#else
	return d_mem->read_data8(SP++);
#endif
}

inline void UPD7801::PUSH8(uint8 val)
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data8w(--SP, val, &wait);
	period += wait;
#else
	d_mem->write_data8(--SP, val);
#endif
}

inline uint16 UPD7801::POP16()
{
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	uint16 val = d_mem->read_data16w(SP, &wait);
	period += wait;
#else
	uint16 val = d_mem->read_data16(SP);
#endif
	SP += 2;
	return val;
}

inline void UPD7801::PUSH16(uint16 val)
{
	SP -= 2;
#ifdef UPD7801_MEMORY_WAIT
	int wait;
	d_mem->write_data16w(SP, val, &wait);
	period += wait;
#else
	d_mem->write_data16(SP, val);
#endif
}

// io

inline uint8 UPD7801::IN8(int port)
{
	if(port == P_C) {
		return (d_io->read_io8(P_C) & 0x87) | (PORTC & 0x78);
	}
	return d_io->read_io8(port);
}

inline void UPD7801::OUT8(int port, uint8 val)
{
	if(port == P_C) {
		PORTC = val;
	}
	d_io->write_io8(port, val);
}

// IOM : 0x20 = I/O, 0 = MEMORY
inline void UPD7801::UPDATE_PORTC(uint8 IOM)
{
	d_io->write_io8(P_C, (PORTC & MC) | ((SAK | TO | IOM) & ~MC));
}

// opecode

#define ACI(r) { \
	uint8 tmp = r + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ACI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old + FETCH8() + (PSW & F_CY); \
	ZHC_ADD(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define ADC(r, n) { \
	uint8 tmp = r + n + (PSW & F_CY); \
	ZHC_ADD(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define ADCW() { \
	uint8 tmp = _A + RM8(FETCHWA()) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADCX(r) { \
	uint8 tmp = _A + RM8(r) + (PSW & F_CY); \
	ZHC_ADD(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define ADD(r, n) { \
	uint8 tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADDNC(r, n) { \
	uint8 tmp = r + n; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADDNCW() { \
	uint8 tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDNCX(r) { \
	uint8 tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define ADDW() { \
	uint8 tmp = _A + RM8(FETCHWA()); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADDX(r) { \
	uint8 tmp = _A + RM8(r); \
	ZHC_ADD(tmp, _A, 0); \
	_A = tmp; \
}
#define ADI(r) { \
	uint8 tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
}
#define ADI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define ADINC(r) { \
	uint8 tmp = r + FETCH8(); \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define ADINC_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old + FETCH8(); \
	ZHC_ADD(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define ANA(r, n) { \
	r &= n; \
	SET_Z(r); \
}
#define ANAW() { \
	_A &= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ANAX(r) { \
	_A &= RM8(r); \
	SET_Z(_A); \
}
#define ANI(r) { \
	r &= FETCH8(); \
	SET_Z(r); \
}
#define ANI_IO(p) { \
	uint8 tmp = IN8(p) & FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ANIW() { \
	uint16 dst = FETCHWA(); \
	uint8 tmp = RM8(dst) & FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define BIT(b) { \
	if(RM8(FETCHWA()) & (1 << b)) { \
		PSW |= F_SK; \
	} \
}
#define BLOCK() { \
	WM8(DE++, RM8(HL++)); \
	if(_C--) { \
		PSW &= ~F_CY; \
		PC--; \
	} else { \
		PSW |= F_CY; \
	} \
}
#define CALF(o) { \
	uint16 dst = 0x800 + ((o & 7) << 8) + FETCH8(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALL() { \
	uint16 dst = FETCH16(); \
	PUSH16(PC); \
	PC = dst; \
}
#define CALT(o) { \
	uint16 dst = RM16(0x80 + ((o & 0x3f) << 1)); \
	PUSH16(PC); \
	PC = dst; \
}
#define DAA() { \
	uint8 lo = _A & 0xf, hi = _A >> 4, diff = 0; \
	if(lo <= 9 && !(PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x60 : 0x00; \
	} else if(lo >= 10 && !(PSW & F_HC)) { \
		diff = (hi >= 9 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} else if(lo <= 2 && (PSW & F_HC)) { \
		diff = (hi >= 10 || (PSW & F_CY)) ? 0x66 : 0x06; \
	} \
	_A += diff; \
	if(_A) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z; \
	} \
	if((PSW & F_CY) || (lo <= 9 ? hi >= 10 : hi >= 9)) { \
		PSW |= F_CY; \
	} else { \
		PSW &= ~F_CY; \
	} \
	if(lo >= 10) { \
		PSW |= F_HC; \
	} else { \
		PSW &= ~F_HC; \
	} \
}
#define DCR(r) { \
	uint8 carry = PSW & F_CY; \
	uint8 tmp = r - 1; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define DCRW() { \
	uint8 carry = PSW & F_CY; \
	uint16 dst = FETCHWA(); \
	uint8 old = RM8(dst); \
	uint8 tmp = old - 1; \
	ZHC_SUB(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define EQA(r, n) { \
	uint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQAW() { \
	uint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQAX(r) { \
	uint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_Z; \
}
#define EQI(r) { \
	uint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_Z; \
}
#define EQI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EQIW() { \
	uint8 old = RM8(FETCHWA()); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_Z; \
}
#define EX() { \
	uint16 tmp; \
	tmp = VA; VA = altVA; altVA = tmp; \
}
#define EXX() { \
	uint16 tmp; \
	tmp = BC; BC = altBC; altBC = tmp; \
	tmp = DE; DE = altDE; altDE = tmp; \
	tmp = HL; HL = altHL; altHL = tmp; \
}
#define GTA(r, n) { \
	uint8 tmp = r - n - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTAW() { \
	uint8 tmp = _A - RM8(FETCHWA()) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTAX(r) { \
	uint8 tmp = _A - RM8(r) - 1; \
	ZHC_SUB(tmp, _A, 1); \
	SKIP_NC; \
}
#define GTI(r) { \
	uint8 tmp = r - FETCH8() - 1; \
	ZHC_SUB(tmp, r, 1); \
	SKIP_NC; \
}
#define GTI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define GTIW() { \
	uint8 old = RM8(FETCHWA()); \
	uint8 tmp = old - FETCH8() - 1; \
	ZHC_SUB(tmp, old, 1); \
	SKIP_NC; \
}
#define INR(r) { \
	uint8 carry = PSW & F_CY; \
	uint8 tmp = r + 1; \
	ZHC_ADD(tmp, r, 0); \
	r = tmp; \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define INRW() { \
	uint8 carry = PSW & F_CY; \
	uint16 dst = FETCHWA(); \
	uint8 old = RM8(dst); \
	uint8 tmp = old + 1; \
	ZHC_ADD(tmp, old, 0); \
	WM8(dst, tmp); \
	SKIP_CY; \
	PSW = (PSW & ~F_CY) | carry; \
}
#define JRE(o) { \
	uint8 tmp = FETCH8(); \
	if(o & 1) { \
		PC -= 256 - tmp; \
	} else { \
		PC += tmp; \
	} \
}
#define LTA(r, n) { \
	uint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTAW() { \
	uint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTAX(r) { \
	uint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_CY; \
}
#define LTI(r) { \
	uint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_CY; \
}
#define LTI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define LTIW() { \
	uint8 old = RM8(FETCHWA()); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_CY; \
}
#define MVIW() { \
	uint16 dst = FETCHWA(); \
	WM8(dst, FETCH8()); \
}
#define NEA(r, n) { \
	uint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEAW() { \
	uint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEAX(r) { \
	uint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	SKIP_NZ; \
}
#define NEI(r) { \
	uint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	SKIP_NZ; \
}
#define NEI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define NEIW() { \
	uint8 old = RM8(FETCHWA()); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	SKIP_NZ; \
}
#define OFFA(r, n) { \
	if(r & n) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFAX(r) { \
	if(_A & RM8(r)) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI(r) { \
	if(r & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define OFFIW() { \
	uint8 tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW &= ~F_Z; \
	} else { \
		PSW |= F_Z | F_SK; \
	} \
}
#define ONA(r, n) { \
	if(r & n) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONAW() { \
	if(_A & RM8(FETCHWA())) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONAX(r) { \
	if(_A & RM8(r)) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONI(r) { \
	if(r & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONI_IO(p) { \
	if(IN8(p) & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ONIW() { \
	uint8 tmp = RM8(FETCHWA()); \
	if(tmp & FETCH8()) { \
		PSW = (PSW & ~F_Z) | F_SK; \
	} else { \
		PSW |= F_Z; \
	} \
}
#define ORA(r, n) { \
	r |= n; \
	SET_Z(r); \
}
#define ORAW() { \
	_A |= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define ORAX(r) { \
	_A |= RM8(r); \
	SET_Z(_A); \
}
#define ORI(r) { \
	r |= FETCH8(); \
	SET_Z(r); \
}
#define ORI_IO(p) { \
	uint8 tmp = IN8(p) | FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}
#define ORIW() { \
	uint16 dst = FETCHWA(); \
	uint8 tmp = RM8(dst) | FETCH8(); \
	WM8(dst, tmp); \
	SET_Z(tmp); \
}
#define PEN() { \
}
#define PER() { \
}
#define PEX() { \
}
#define RLD() { \
	uint8 old = RM8(HL); \
	uint8 tmp = (old << 4) | (_A & 0x0f); \
	_A = (_A & 0xf0) | (old >> 4); \
	WM8(HL, tmp); \
}
#define RLL(r) { \
	uint8 carry = PSW & F_CY; \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r = (r << 1) | carry; \
}
#define RLR(r) { \
	uint8 carry = (PSW & F_CY) << 7; \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r = (r >> 1) | carry; \
}
#define RRD() { \
	uint8 old = RM8(HL); \
	uint8 tmp = (_A << 4) | (old >> 4); \
	_A = (_A & 0xf0) | (old & 0x0f); \
	WM8(HL, tmp); \
}
#define SBB(r, n) { \
	uint8 tmp = r - n - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBBW() { \
	uint8 tmp = _A - RM8(FETCHWA()) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBBX(r) { \
	uint8 tmp = _A - RM8(r) - (PSW & F_CY); \
	ZHC_SUB(tmp, _A, (PSW & F_CY)); \
	_A = tmp; \
}
#define SBI(r) { \
	uint8 tmp = r - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, r, (PSW & F_CY)); \
	r = tmp; \
}
#define SBI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8() - (PSW & F_CY); \
	ZHC_SUB(tmp, old, (PSW & F_CY)); \
	OUT8(p, tmp); \
}
#define SIO() { \
	scount = 32 + 4; \
}
#define SK(f) { \
	if(PSW & f) { \
		PSW |= F_SK; \
	} \
}
#define SKIT(f) { \
	if(IRR & f) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SKN(f) { \
	if(!(PSW & f)) { \
		PSW |= F_SK; \
	} \
}
#define SKNIT(f) { \
	if(!(IRR & f)) { \
		PSW |= F_SK; \
	} \
	IRR &= ~f; \
}
#define SLL(r) { \
	PSW = (PSW & ~F_CY) | ((r >> 7) & F_CY); \
	r <<= 1; \
}
#define SLR(r) { \
	PSW = (PSW & ~F_CY) | (r & F_CY); \
	r >>= 1; \
}
#define STM() { \
	tcount = (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER; \
}
#define SUB(r, n) { \
	uint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUBNB(r, n) { \
	uint8 tmp = r - n; \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUBNBW() { \
	uint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBNBX(r) { \
	uint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
	SKIP_NC; \
}
#define SUBW() { \
	uint8 tmp = _A - RM8(FETCHWA()); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUBX(r) { \
	uint8 tmp = _A - RM8(r); \
	ZHC_SUB(tmp, _A, 0); \
	_A = tmp; \
}
#define SUI(r) { \
	uint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
}
#define SUI_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
}
#define SUINB(r) { \
	uint8 tmp = r - FETCH8(); \
	ZHC_SUB(tmp, r, 0); \
	r = tmp; \
	SKIP_NC; \
}
#define SUINB_IO(p) { \
	uint8 old = IN8(p); \
	uint8 tmp = old - FETCH8(); \
	ZHC_SUB(tmp, old, 0); \
	OUT8(p, tmp); \
	SKIP_NC; \
}
#define XRA(r, n) { \
	r ^= n; \
	SET_Z(r); \
}
#define XRAW() { \
	_A ^= RM8(FETCHWA()); \
	SET_Z(_A); \
}
#define XRAX(r) { \
	_A ^= RM8(r); \
	SET_Z(_A); \
}
#define XRI(r) { \
	r ^= FETCH8(); \
	SET_Z(r); \
}
#define XRI_IO(p) { \
	uint8 tmp = IN8(p) ^ FETCH8(); \
	OUT8(p, tmp); \
	SET_Z(tmp); \
}

void UPD7801::initialize()
{
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void UPD7801::reset()
{
	PC = SP = 0;
//	VA = BC = DE = HL = altVA = altBC = altDE = altHL = 0;
	PSW = IRR = IFF = SIRQ = HALT = 0;
	_V = MB = MC = TM0 = TM1 = SR = 0xff;
	altVA = VA;
	MK = 0x1f;
	PORTC = TO = SAK = 0;
	count = 0;
	scount = tcount = 0;
	wait = false;
}

int UPD7801::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		count = 0;
#ifdef USE_DEBUGGER
		run_one_opecode_debugger();
#else
		run_one_opecode();
#endif
		return -count;
	} else {
		// run cpu while given clocks
		count += clock;
		int first_count = count;
		
		while(count > 0) {
#ifdef USE_DEBUGGER
			run_one_opecode_debugger();
#else
			run_one_opecode();
#endif
		}
		return first_count - count;
	}
}

void UPD7801::run_one_opecode()
{
	if(wait) {
		period = 1;
	} else {
		// interrupt is enabled after next opecode of ei
		if(IFF & 2) {
			IFF--;
		}
		
		// run 1 opecode
		period = 0;
		prevPC = PC;
		OP();
	}
	count -= period;
	
	// update serial count
	if(scount && (scount -= period) <= 0) {
		scount = 0;
		IRR |= INTFS;
		OUT8(P_SO, SR);
		SR = IN8(P_SI);
		if(SAK) {
			SAK = 0;
			UPDATE_PORTC(0);
		}
	}
	// update timer
	if(tcount && (tcount -= period) <= 0) {
		tcount += (((TM0 | (TM1 << 8)) & 0xfff) + 1) * PRESCALER;
		IRR |= INTFT;
		if(TO) {
			TO = 0;
			UPDATE_PORTC(0);
		}
	}
	
	// check interrupt
	if(IFF == 1 && !SIRQ) {
		for(int i = 0; i < 5; i++) {
			uint8 bit = irq_bits[i];
			if((IRR & bit) && !(MK & bit)) {
				if(HALT) {
					HALT = 0;
					PC++;
				}
				PUSH8(PSW);
				PUSH16(PC);
				
				PC = irq_addr[i];
				PSW &= ~(F_SK | F_L0 | F_L1);
				IFF = 0;
				IRR &= ~bit;
				break;
			}
		}
	}
}

#ifdef USE_DEBUGGER
void UPD7801::run_one_opecode_debugger()
{
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended) {
			emu->mute_sound();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				Sleep(10);
			}
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		run_one_opecode();
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
		run_one_opecode();
	}
}

void UPD7801::debug_write_data8(uint32 addr, uint32 data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32 UPD7801::debug_read_data8(uint32 addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void UPD7801::debug_write_io8(uint32 addr, uint32 data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32 UPD7801::debug_read_io8(uint32 addr) {
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

bool UPD7801::debug_write_reg(_TCHAR *reg, uint32 data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("VA")) == 0) {
		VA = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("V")) == 0) {
		_V = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		_A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		_B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		_C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		_D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		_E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		_H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		_L = data;
	} else if(_tcsicmp(reg, _T("VA'")) == 0) {
		altVA = data;
	} else if(_tcsicmp(reg, _T("BC'")) == 0) {
		altBC = data;
	} else if(_tcsicmp(reg, _T("DE'")) == 0) {
		altDE = data;
	} else if(_tcsicmp(reg, _T("HL'")) == 0) {
		altHL = data;
	} else if(_tcsicmp(reg, _T("V'")) == 0) {
		altV = data;
	} else if(_tcsicmp(reg, _T("A'")) == 0) {
		altA = data;
	} else if(_tcsicmp(reg, _T("B'")) == 0) {
		altB = data;
	} else if(_tcsicmp(reg, _T("C'")) == 0) {
		altC = data;
	} else if(_tcsicmp(reg, _T("D'")) == 0) {
		altD = data;
	} else if(_tcsicmp(reg, _T("E'")) == 0) {
		altE = data;
	} else if(_tcsicmp(reg, _T("H'")) == 0) {
		altH = data;
	} else if(_tcsicmp(reg, _T("L'")) == 0) {
		altL = data;
	} else {
		return false;
	}
	return true;
}

void UPD7801::debug_regs_info(_TCHAR *buffer)
{
/*
VA = 0000  BC = 0000  DE = 0000 HL = 0000  PSW= 00 [Z SK HC L1 L0 CY]
VA'= 0000  BC'= 0000  DE'= 0000 HL'= 0000  SP = 0000  PC = 0000
          (BC)= 0000 (DE)=0000 (HL)= 0000 (SP)= 0000 <DI>
*/
	int wait;
	_stprintf(buffer, _T("VA = %04X  BC = %04X  DE = %04X HL = %04X  PSW= %02x [%s %s %s %s %s %s]\nVA'= %04X  BC'= %04X  DE'= %04X HL'= %04X  SP = %04X  PC = %04X\n          (BC)= %04X (DE)=%04X (HL)= %04X (SP)= %04X <%s>"),
	VA, BC, DE, HL, PSW,
	(PSW & F_Z) ? _T("Z") : _T("-"), (PSW & F_SK) ? _T("SK") : _T("--"), (PSW & F_HC) ? _T("HC") : _T("--"), (PSW & F_L1) ? _T("L1") : _T("--"), (PSW & F_L0) ? _T("L0") : _T("--"), (PSW & F_CY) ? _T("CY") : _T("--"),
	altVA, altBC, altDE, altHL, SP, PC,
	d_mem_stored->read_data16w(BC, &wait), d_mem_stored->read_data16w(DE, &wait), d_mem_stored->read_data16w(HL, &wait), d_mem_stored->read_data16w(SP, &wait),
	IFF ? _T("EI") : _T("DI"));
}

// disassembler

uint8 upd7801_dasm_ops[4];
int upd7801_dasm_ptr;

uint8 getb()
{
	return upd7801_dasm_ops[upd7801_dasm_ptr++];
}

uint16 getw()
{
	uint16 l = getb();
	return l | (getb() << 8);
}

uint8 getwa()
{
	return getb();
}

int UPD7801::debug_dasm(uint32 pc, _TCHAR *buffer)
{
	for(int i = 0; i < 4; i++) {
		int wait;
		upd7801_dasm_ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	upd7801_dasm_ptr = 0;
	
	uint8 b;
	uint16 wa;
	
	switch(b = getb()) {
	case 0x00: _stprintf(buffer, _T("nop")); break;
	case 0x01: _stprintf(buffer, _T("hlt")); break;
	case 0x02: _stprintf(buffer, _T("inx sp")); break;
	case 0x03: _stprintf(buffer, _T("dcx sp")); break;
	case 0x04: _stprintf(buffer, _T("lxi sp,%4xh"), getw()); break;
	case 0x05: wa = getwa(); _stprintf(buffer, _T("aniw v.%xh,%2xh"), wa, getb()); break;
//	case 0x06:
	case 0x07: _stprintf(buffer, _T("ani a,%2xh"), getb()); break;
	case 0x08: _stprintf(buffer, _T("ret")); break;
	case 0x09: _stprintf(buffer, _T("sio")); break;
	case 0x0a: _stprintf(buffer, _T("mov a,b")); break;
	case 0x0b: _stprintf(buffer, _T("mov a,c")); break;
	case 0x0c: _stprintf(buffer, _T("mov a,d")); break;
	case 0x0d: _stprintf(buffer, _T("mov a,e")); break;
	case 0x0e: _stprintf(buffer, _T("mov a,h")); break;
	case 0x0f: _stprintf(buffer, _T("mov a,l")); break;
	
	case 0x10: _stprintf(buffer, _T("ex")); break;
	case 0x11: _stprintf(buffer, _T("exx")); break;
	case 0x12: _stprintf(buffer, _T("inx b")); break;
	case 0x13: _stprintf(buffer, _T("dcx b")); break;
	case 0x14: _stprintf(buffer, _T("lxi b,%4xh"), getw()); break;
	case 0x15: wa = getwa(); _stprintf(buffer, _T("oriw v.%xh,%2xh"), wa, getb()); break;
	case 0x16: _stprintf(buffer, _T("xri a,%2xh"), getb()); break;
	case 0x17: _stprintf(buffer, _T("ori a,%2xh"), getb()); break;
	case 0x18: _stprintf(buffer, _T("rets")); break;
	case 0x19: _stprintf(buffer, _T("stm")); break;
	case 0x1a: _stprintf(buffer, _T("mov b,a")); break;
	case 0x1b: _stprintf(buffer, _T("mov c,a")); break;
	case 0x1c: _stprintf(buffer, _T("mov d,a")); break;
	case 0x1d: _stprintf(buffer, _T("mov e,a")); break;
	case 0x1e: _stprintf(buffer, _T("mov h,a")); break;
	case 0x1f: _stprintf(buffer, _T("mov l,a")); break;
	
	case 0x20: _stprintf(buffer, _T("inrw v.%xh"), getwa()); break;
	case 0x21: _stprintf(buffer, _T("table")); break;
	case 0x22: _stprintf(buffer, _T("inx d")); break;
	case 0x23: _stprintf(buffer, _T("dcx d")); break;
	case 0x24: _stprintf(buffer, _T("lxi d,%4xh"), getw()); break;
	case 0x25: wa = getwa(); _stprintf(buffer, _T("gtiw v.%xh,%2xh"), wa, getb()); break;
	case 0x26: _stprintf(buffer, _T("adinc a,%2xh"), getb()); break;
	case 0x27: _stprintf(buffer, _T("gti a,%2xh"), getb()); break;
	case 0x28: _stprintf(buffer, _T("ldaw v.%xh"), getwa()); break;
	case 0x29: _stprintf(buffer, _T("ldax b")); break;
	case 0x2a: _stprintf(buffer, _T("ldax d")); break;
	case 0x2b: _stprintf(buffer, _T("ldax h")); break;
	case 0x2c: _stprintf(buffer, _T("ldax d+")); break;
	case 0x2d: _stprintf(buffer, _T("ldax h+")); break;
	case 0x2e: _stprintf(buffer, _T("ldax d-")); break;
	case 0x2f: _stprintf(buffer, _T("ldax h-")); break;
	
	case 0x30: _stprintf(buffer, _T("dcrw v.%xh"), getwa()); break;
	case 0x31: _stprintf(buffer, _T("block")); break;
	case 0x32: _stprintf(buffer, _T("inx h")); break;
	case 0x33: _stprintf(buffer, _T("dcx h")); break;
	case 0x34: _stprintf(buffer, _T("lxi h,%4xh"), getw()); break;
	case 0x35: wa = getwa(); _stprintf(buffer, _T("ltiw v.%xh,%2xh"), wa, getb()); break;
	case 0x36: _stprintf(buffer, _T("suinb a,%2xh"), getb()); break;
	case 0x37: _stprintf(buffer, _T("lti a,%2xh"), getb()); break;
	case 0x38: _stprintf(buffer, _T("staw v.%xh"), getwa()); break;
	case 0x39: _stprintf(buffer, _T("stax b")); break;
	case 0x3a: _stprintf(buffer, _T("stax d")); break;
	case 0x3b: _stprintf(buffer, _T("stax h")); break;
	case 0x3c: _stprintf(buffer, _T("stax d+")); break;
	case 0x3d: _stprintf(buffer, _T("stax h+")); break;
	case 0x3e: _stprintf(buffer, _T("stax d-")); break;
	case 0x3f: _stprintf(buffer, _T("stax h-")); break;
	
//	case 0x40:
	case 0x41: _stprintf(buffer, _T("inr a")); break;
	case 0x42: _stprintf(buffer, _T("inr b")); break;
	case 0x43: _stprintf(buffer, _T("inr c")); break;
	case 0x44: _stprintf(buffer, _T("call %4xh"), getw()); break;
	case 0x45: wa = getwa(); _stprintf(buffer, _T("oniw v.%xh,%2xh"), wa, getb()); break;
	case 0x46: _stprintf(buffer, _T("adi a,%2xh"), getb()); break;
	case 0x47: _stprintf(buffer, _T("oni a,%2xh"), getb()); break;
	case 0x48:
		switch(b = getb()) {
		case 0x00: _stprintf(buffer, _T("skit intf0")); break;
		case 0x01: _stprintf(buffer, _T("skit intft")); break;
		case 0x02: _stprintf(buffer, _T("skit intf1")); break;
		case 0x03: _stprintf(buffer, _T("skit intf2")); break;
		case 0x04: _stprintf(buffer, _T("skit intfs")); break;
		case 0x0a: _stprintf(buffer, _T("sk cy")); break;
		case 0x0c: _stprintf(buffer, _T("sk z")); break;
		case 0x0e: _stprintf(buffer, _T("push v")); break;
		case 0x0f: _stprintf(buffer, _T("pop v")); break;
		case 0x10: _stprintf(buffer, _T("sknit f0")); break;
		case 0x11: _stprintf(buffer, _T("sknit ft")); break;
		case 0x12: _stprintf(buffer, _T("sknit f1")); break;
		case 0x13: _stprintf(buffer, _T("sknit f2")); break;
		case 0x14: _stprintf(buffer, _T("sknit fs")); break;
		case 0x1a: _stprintf(buffer, _T("skn cy")); break;
		case 0x1c: _stprintf(buffer, _T("skn z")); break;
		case 0x1e: _stprintf(buffer, _T("push b")); break;
		case 0x1f: _stprintf(buffer, _T("pop b")); break;
		case 0x20: _stprintf(buffer, _T("ei")); break;
		case 0x24: _stprintf(buffer, _T("di")); break;
		case 0x2a: _stprintf(buffer, _T("clc")); break;
		case 0x2b: _stprintf(buffer, _T("stc")); break;
		case 0x2c: _stprintf(buffer, _T("pen")); break;
		case 0x2d: _stprintf(buffer, _T("pex")); break;
		case 0x2e: _stprintf(buffer, _T("push d")); break;
		case 0x2f: _stprintf(buffer, _T("pop d")); break;
		case 0x30: _stprintf(buffer, _T("rll a")); break;
		case 0x31: _stprintf(buffer, _T("rlr a")); break;
		case 0x32: _stprintf(buffer, _T("rll c")); break;
		case 0x33: _stprintf(buffer, _T("rlr c")); break;
		case 0x34: _stprintf(buffer, _T("sll a")); break;
		case 0x35: _stprintf(buffer, _T("slr a")); break;
		case 0x36: _stprintf(buffer, _T("sll c")); break;
		case 0x37: _stprintf(buffer, _T("sll c")); break;
		case 0x38: _stprintf(buffer, _T("rld")); break;
		case 0x39: _stprintf(buffer, _T("rrd")); break;
		case 0x3c: _stprintf(buffer, _T("per")); break;
		case 0x3e: _stprintf(buffer, _T("push h")); break;
		case 0x3f: _stprintf(buffer, _T("pop h")); break;
		default: _stprintf(buffer, _T("db 48h,%2xh"), b);
		}
		break;
	case 0x49: _stprintf(buffer, _T("mvix b,%2xh"), getb()); break;
	case 0x4a: _stprintf(buffer, _T("mvix d,%2xh"), getb()); break;
	case 0x4b: _stprintf(buffer, _T("mvix h,%2xh"), getb()); break;
	case 0x4c:
		switch(b = getb()) {
		case 0xc0: _stprintf(buffer, _T("mov a,pa")); break;
		case 0xc1: _stprintf(buffer, _T("mov a,pb")); break;
		case 0xc2: _stprintf(buffer, _T("mov a,pc")); break;
		case 0xc3: _stprintf(buffer, _T("mov a,mk")); break;
		case 0xc4: _stprintf(buffer, _T("mov a,mb")); break;	// 未定義?
		case 0xc5: _stprintf(buffer, _T("mov a,mc")); break;	// 未定義?
		case 0xc6: _stprintf(buffer, _T("mov a,tm0")); break;	// 未定義?
		case 0xc7: _stprintf(buffer, _T("mov a,tm1")); break;	// 未定義?
		case 0xc8: _stprintf(buffer, _T("mov a,s")); break;
		default:
			if(b < 0xc0) {
				_stprintf(buffer, _T("in %2xh"), getb()); break;
			}
			_stprintf(buffer, _T("db 4ch,%2xh"), b);
		}
		break;
	case 0x4d:
		switch(b = getb()) {
		case 0xc0: _stprintf(buffer, _T("mov pa,a")); break;
		case 0xc1: _stprintf(buffer, _T("mov pb,a")); break;
		case 0xc2: _stprintf(buffer, _T("mov pc,a")); break;
		case 0xc3: _stprintf(buffer, _T("mov mk,a")); break;
		case 0xc4: _stprintf(buffer, _T("mov mb,a")); break;
		case 0xc5: _stprintf(buffer, _T("mov mc,a")); break;
		case 0xc6: _stprintf(buffer, _T("mov tm0,a")); break;
		case 0xc7: _stprintf(buffer, _T("mov tm1,a")); break;
		case 0xc8: _stprintf(buffer, _T("mov s,a")); break;
		default:
			if(b < 0xc0) {
				_stprintf(buffer, _T("out %2xh"), getb()); break;
			}
			_stprintf(buffer, _T("db 4dh,%2xh"), b);
		}
		break;
	case 0x4e: b = getb(); _stprintf(buffer, _T("jre %4xh"), pc + b); break;
	case 0x4f: b = getb(); _stprintf(buffer, _T("jre %4xh"), (pc + b - 256) & 0xffff); break;
	
//	case 0x50:
	case 0x51: _stprintf(buffer, _T("dcr a")); break;
	case 0x52: _stprintf(buffer, _T("dcr b")); break;
	case 0x53: _stprintf(buffer, _T("dcr c")); break;
	case 0x54: _stprintf(buffer, _T("jmp %4xh"), getw()); break;
	case 0x55: wa = getwa(); _stprintf(buffer, _T("offiw v.%xh,%2xh"), wa, getb()); break;
	case 0x56: _stprintf(buffer, _T("aci a,%2xh"), getb()); break;
	case 0x57: _stprintf(buffer, _T("offi a,%2xh"), getb()); break;
	case 0x58: _stprintf(buffer, _T("bit 0,v.%xh"), getwa()); break;
	case 0x59: _stprintf(buffer, _T("bit 1,v.%xh"), getwa()); break;
	case 0x5a: _stprintf(buffer, _T("bit 2,v.%xh"), getwa()); break;
	case 0x5b: _stprintf(buffer, _T("bit 3,v.%xh"), getwa()); break;
	case 0x5c: _stprintf(buffer, _T("bit 4,v.%xh"), getwa()); break;
	case 0x5d: _stprintf(buffer, _T("bit 5,v.%xh"), getwa()); break;
	case 0x5e: _stprintf(buffer, _T("bit 6,v.%xh"), getwa()); break;
	case 0x5f: _stprintf(buffer, _T("bit 7,v.%xh"), getwa()); break;
	
	case 0x60:
		switch(b = getb()) {
		case 0x08: _stprintf(buffer, _T("ana v,a")); break;
		case 0x09: _stprintf(buffer, _T("ana a,a")); break;
		case 0x0a: _stprintf(buffer, _T("ana b,a")); break;
		case 0x0b: _stprintf(buffer, _T("ana c,a")); break;
		case 0x0c: _stprintf(buffer, _T("ana d,a")); break;
		case 0x0d: _stprintf(buffer, _T("ana e,a")); break;
		case 0x0e: _stprintf(buffer, _T("ana h,a")); break;
		case 0x0f: _stprintf(buffer, _T("ana l,a")); break;
		case 0x10: _stprintf(buffer, _T("xra v,a")); break;
		case 0x11: _stprintf(buffer, _T("xra a,a")); break;
		case 0x12: _stprintf(buffer, _T("xra b,a")); break;
		case 0x13: _stprintf(buffer, _T("xra c,a")); break;
		case 0x14: _stprintf(buffer, _T("xra d,a")); break;
		case 0x15: _stprintf(buffer, _T("xra e,a")); break;
		case 0x16: _stprintf(buffer, _T("xra h,a")); break;
		case 0x17: _stprintf(buffer, _T("xra l,a")); break;
		case 0x18: _stprintf(buffer, _T("ora v,a")); break;
		case 0x19: _stprintf(buffer, _T("ora a,a")); break;
		case 0x1a: _stprintf(buffer, _T("ora b,a")); break;
		case 0x1b: _stprintf(buffer, _T("ora c,a")); break;
		case 0x1c: _stprintf(buffer, _T("ora d,a")); break;
		case 0x1d: _stprintf(buffer, _T("ora e,a")); break;
		case 0x1e: _stprintf(buffer, _T("ora h,a")); break;
		case 0x1f: _stprintf(buffer, _T("ora l,a")); break;
		case 0x20: _stprintf(buffer, _T("addnc v,a")); break;
		case 0x21: _stprintf(buffer, _T("addnc a,a")); break;
		case 0x22: _stprintf(buffer, _T("addnc b,a")); break;
		case 0x23: _stprintf(buffer, _T("addnc c,a")); break;
		case 0x24: _stprintf(buffer, _T("addnc d,a")); break;
		case 0x25: _stprintf(buffer, _T("addnc e,a")); break;
		case 0x26: _stprintf(buffer, _T("addnc h,a")); break;
		case 0x27: _stprintf(buffer, _T("addnc l,a")); break;
		case 0x28: _stprintf(buffer, _T("gta v,a")); break;
		case 0x29: _stprintf(buffer, _T("gta a,a")); break;
		case 0x2a: _stprintf(buffer, _T("gta b,a")); break;
		case 0x2b: _stprintf(buffer, _T("gta c,a")); break;
		case 0x2c: _stprintf(buffer, _T("gta d,a")); break;
		case 0x2d: _stprintf(buffer, _T("gta e,a")); break;
		case 0x2e: _stprintf(buffer, _T("gta h,a")); break;
		case 0x2f: _stprintf(buffer, _T("gta l,a")); break;
		case 0x30: _stprintf(buffer, _T("subnb v,a")); break;
		case 0x31: _stprintf(buffer, _T("subnb a,a")); break;
		case 0x32: _stprintf(buffer, _T("subnb b,a")); break;
		case 0x33: _stprintf(buffer, _T("subnb c,a")); break;
		case 0x34: _stprintf(buffer, _T("subnb d,a")); break;
		case 0x35: _stprintf(buffer, _T("subnb e,a")); break;
		case 0x36: _stprintf(buffer, _T("subnb h,a")); break;
		case 0x37: _stprintf(buffer, _T("subnb l,a")); break;
		case 0x38: _stprintf(buffer, _T("lta v,a")); break;
		case 0x39: _stprintf(buffer, _T("lta a,a")); break;
		case 0x3a: _stprintf(buffer, _T("lta b,a")); break;
		case 0x3b: _stprintf(buffer, _T("lta c,a")); break;
		case 0x3c: _stprintf(buffer, _T("lta d,a")); break;
		case 0x3d: _stprintf(buffer, _T("lta e,a")); break;
		case 0x3e: _stprintf(buffer, _T("lta h,a")); break;
		case 0x3f: _stprintf(buffer, _T("lta l,a")); break;
		case 0x40: _stprintf(buffer, _T("add v,a")); break;
		case 0x41: _stprintf(buffer, _T("add a,a")); break;
		case 0x42: _stprintf(buffer, _T("add b,a")); break;
		case 0x43: _stprintf(buffer, _T("add c,a")); break;
		case 0x44: _stprintf(buffer, _T("add d,a")); break;
		case 0x45: _stprintf(buffer, _T("add e,a")); break;
		case 0x46: _stprintf(buffer, _T("add h,a")); break;
		case 0x47: _stprintf(buffer, _T("add l,a")); break;
		case 0x50: _stprintf(buffer, _T("adc v,a")); break;
		case 0x51: _stprintf(buffer, _T("adc a,a")); break;
		case 0x52: _stprintf(buffer, _T("adc b,a")); break;
		case 0x53: _stprintf(buffer, _T("adc c,a")); break;
		case 0x54: _stprintf(buffer, _T("adc d,a")); break;
		case 0x55: _stprintf(buffer, _T("adc e,a")); break;
		case 0x56: _stprintf(buffer, _T("adc h,a")); break;
		case 0x57: _stprintf(buffer, _T("adc l,a")); break;
		case 0x60: _stprintf(buffer, _T("sub v,a")); break;
		case 0x61: _stprintf(buffer, _T("sub a,a")); break;
		case 0x62: _stprintf(buffer, _T("sub b,a")); break;
		case 0x63: _stprintf(buffer, _T("sub c,a")); break;
		case 0x64: _stprintf(buffer, _T("sub d,a")); break;
		case 0x65: _stprintf(buffer, _T("sub e,a")); break;
		case 0x66: _stprintf(buffer, _T("sub h,a")); break;
		case 0x67: _stprintf(buffer, _T("sub l,a")); break;
		case 0x68: _stprintf(buffer, _T("nea v,a")); break;
		case 0x69: _stprintf(buffer, _T("nea a,a")); break;
		case 0x6a: _stprintf(buffer, _T("nea b,a")); break;
		case 0x6b: _stprintf(buffer, _T("nea c,a")); break;
		case 0x6c: _stprintf(buffer, _T("nea d,a")); break;
		case 0x6d: _stprintf(buffer, _T("nea e,a")); break;
		case 0x6e: _stprintf(buffer, _T("nea h,a")); break;
		case 0x6f: _stprintf(buffer, _T("nea l,a")); break;
		case 0x70: _stprintf(buffer, _T("sbb v,a")); break;
		case 0x71: _stprintf(buffer, _T("sbb a,a")); break;
		case 0x72: _stprintf(buffer, _T("sbb b,a")); break;
		case 0x73: _stprintf(buffer, _T("sbb c,a")); break;
		case 0x74: _stprintf(buffer, _T("sbb d,a")); break;
		case 0x75: _stprintf(buffer, _T("sbb e,a")); break;
		case 0x76: _stprintf(buffer, _T("sbb h,a")); break;
		case 0x77: _stprintf(buffer, _T("sbb l,a")); break;
		case 0x78: _stprintf(buffer, _T("eqa v,a")); break;
		case 0x79: _stprintf(buffer, _T("eqa a,a")); break;
		case 0x7a: _stprintf(buffer, _T("eqa b,a")); break;
		case 0x7b: _stprintf(buffer, _T("eqa c,a")); break;
		case 0x7c: _stprintf(buffer, _T("eqa d,a")); break;
		case 0x7d: _stprintf(buffer, _T("eqa e,a")); break;
		case 0x7e: _stprintf(buffer, _T("eqa h,a")); break;
		case 0x7f: _stprintf(buffer, _T("eqa l,a")); break;
		case 0x88: _stprintf(buffer, _T("ana a,v")); break;
		case 0x89: _stprintf(buffer, _T("ana a,a")); break;
		case 0x8a: _stprintf(buffer, _T("ana a,b")); break;
		case 0x8b: _stprintf(buffer, _T("ana a,c")); break;
		case 0x8c: _stprintf(buffer, _T("ana a,d")); break;
		case 0x8d: _stprintf(buffer, _T("ana a,e")); break;
		case 0x8e: _stprintf(buffer, _T("ana a,h")); break;
		case 0x8f: _stprintf(buffer, _T("ana a,l")); break;
		case 0x90: _stprintf(buffer, _T("xra a,v")); break;
		case 0x91: _stprintf(buffer, _T("xra a,a")); break;
		case 0x92: _stprintf(buffer, _T("xra a,b")); break;
		case 0x93: _stprintf(buffer, _T("xra a,c")); break;
		case 0x94: _stprintf(buffer, _T("xra a,d")); break;
		case 0x95: _stprintf(buffer, _T("xra a,e")); break;
		case 0x96: _stprintf(buffer, _T("xra a,h")); break;
		case 0x97: _stprintf(buffer, _T("xra a,l")); break;
		case 0x98: _stprintf(buffer, _T("ora a,v")); break;
		case 0x99: _stprintf(buffer, _T("ora a,a")); break;
		case 0x9a: _stprintf(buffer, _T("ora a,b")); break;
		case 0x9b: _stprintf(buffer, _T("ora a,c")); break;
		case 0x9c: _stprintf(buffer, _T("ora a,d")); break;
		case 0x9d: _stprintf(buffer, _T("ora a,e")); break;
		case 0x9e: _stprintf(buffer, _T("ora a,h")); break;
		case 0x9f: _stprintf(buffer, _T("ora a,l")); break;
		case 0xa0: _stprintf(buffer, _T("addnc a,v")); break;
		case 0xa1: _stprintf(buffer, _T("addnc a,a")); break;
		case 0xa2: _stprintf(buffer, _T("addnc a,b")); break;
		case 0xa3: _stprintf(buffer, _T("addnc a,c")); break;
		case 0xa4: _stprintf(buffer, _T("addnc a,d")); break;
		case 0xa5: _stprintf(buffer, _T("addnc a,e")); break;
		case 0xa6: _stprintf(buffer, _T("addnc a,h")); break;
		case 0xa7: _stprintf(buffer, _T("addnc a,l")); break;
		case 0xa8: _stprintf(buffer, _T("gta a,v")); break;
		case 0xa9: _stprintf(buffer, _T("gta a,a")); break;
		case 0xaa: _stprintf(buffer, _T("gta a,b")); break;
		case 0xab: _stprintf(buffer, _T("gta a,c")); break;
		case 0xac: _stprintf(buffer, _T("gta a,d")); break;
		case 0xad: _stprintf(buffer, _T("gta a,e")); break;
		case 0xae: _stprintf(buffer, _T("gta a,h")); break;
		case 0xaf: _stprintf(buffer, _T("gta a,l")); break;
		case 0xb0: _stprintf(buffer, _T("subnb a,v")); break;
		case 0xb1: _stprintf(buffer, _T("subnb a,a")); break;
		case 0xb2: _stprintf(buffer, _T("subnb a,b")); break;
		case 0xb3: _stprintf(buffer, _T("subnb a,c")); break;
		case 0xb4: _stprintf(buffer, _T("subnb a,d")); break;
		case 0xb5: _stprintf(buffer, _T("subnb a,e")); break;
		case 0xb6: _stprintf(buffer, _T("subnb a,h")); break;
		case 0xb7: _stprintf(buffer, _T("subnb a,l")); break;
		case 0xb8: _stprintf(buffer, _T("lta a,v")); break;
		case 0xb9: _stprintf(buffer, _T("lta a,a")); break;
		case 0xba: _stprintf(buffer, _T("lta a,b")); break;
		case 0xbb: _stprintf(buffer, _T("lta a,c")); break;
		case 0xbc: _stprintf(buffer, _T("lta a,d")); break;
		case 0xbd: _stprintf(buffer, _T("lta a,e")); break;
		case 0xbe: _stprintf(buffer, _T("lta a,h")); break;
		case 0xbf: _stprintf(buffer, _T("lta a,l")); break;
		case 0xc0: _stprintf(buffer, _T("add a,v")); break;
		case 0xc1: _stprintf(buffer, _T("add a,a")); break;
		case 0xc2: _stprintf(buffer, _T("add a,b")); break;
		case 0xc3: _stprintf(buffer, _T("add a,c")); break;
		case 0xc4: _stprintf(buffer, _T("add a,d")); break;
		case 0xc5: _stprintf(buffer, _T("add a,e")); break;
		case 0xc6: _stprintf(buffer, _T("add a,h")); break;
		case 0xc7: _stprintf(buffer, _T("add a,l")); break;
		case 0xc8: _stprintf(buffer, _T("ona a,v")); break;
		case 0xc9: _stprintf(buffer, _T("ona a,a")); break;
		case 0xca: _stprintf(buffer, _T("ona a,b")); break;
		case 0xcb: _stprintf(buffer, _T("ona a,c")); break;
		case 0xcc: _stprintf(buffer, _T("ona a,d")); break;
		case 0xcd: _stprintf(buffer, _T("ona a,e")); break;
		case 0xce: _stprintf(buffer, _T("ona a,h")); break;
		case 0xcf: _stprintf(buffer, _T("ona a,l")); break;
		case 0xd0: _stprintf(buffer, _T("adc a,v")); break;
		case 0xd1: _stprintf(buffer, _T("adc a,a")); break;
		case 0xd2: _stprintf(buffer, _T("adc a,b")); break;
		case 0xd3: _stprintf(buffer, _T("adc a,c")); break;
		case 0xd4: _stprintf(buffer, _T("adc a,d")); break;
		case 0xd5: _stprintf(buffer, _T("adc a,e")); break;
		case 0xd6: _stprintf(buffer, _T("adc a,h")); break;
		case 0xd7: _stprintf(buffer, _T("adc a,l")); break;
		case 0xd8: _stprintf(buffer, _T("offa a,v")); break;
		case 0xd9: _stprintf(buffer, _T("offa a,a")); break;
		case 0xda: _stprintf(buffer, _T("offa a,b")); break;
		case 0xdb: _stprintf(buffer, _T("offa a,c")); break;
		case 0xdc: _stprintf(buffer, _T("offa a,d")); break;
		case 0xdd: _stprintf(buffer, _T("offa a,e")); break;
		case 0xde: _stprintf(buffer, _T("offa a,h")); break;
		case 0xdf: _stprintf(buffer, _T("offa a,l")); break;
		case 0xe0: _stprintf(buffer, _T("sub a,v")); break;
		case 0xe1: _stprintf(buffer, _T("sub a,a")); break;
		case 0xe2: _stprintf(buffer, _T("sub a,b")); break;
		case 0xe3: _stprintf(buffer, _T("sub a,c")); break;
		case 0xe4: _stprintf(buffer, _T("sub a,d")); break;
		case 0xe5: _stprintf(buffer, _T("sub a,e")); break;
		case 0xe6: _stprintf(buffer, _T("sub a,h")); break;
		case 0xe7: _stprintf(buffer, _T("sub a,l")); break;
		case 0xe8: _stprintf(buffer, _T("nea a,v")); break;
		case 0xe9: _stprintf(buffer, _T("nea a,a")); break;
		case 0xea: _stprintf(buffer, _T("nea a,b")); break;
		case 0xeb: _stprintf(buffer, _T("nea a,c")); break;
		case 0xec: _stprintf(buffer, _T("nea a,d")); break;
		case 0xed: _stprintf(buffer, _T("nea a,e")); break;
		case 0xee: _stprintf(buffer, _T("nea a,h")); break;
		case 0xef: _stprintf(buffer, _T("nea a,l")); break;
		case 0xf0: _stprintf(buffer, _T("sbb a,v")); break;
		case 0xf1: _stprintf(buffer, _T("sbb a,a")); break;
		case 0xf2: _stprintf(buffer, _T("sbb a,b")); break;
		case 0xf3: _stprintf(buffer, _T("sbb a,c")); break;
		case 0xf4: _stprintf(buffer, _T("sbb a,d")); break;
		case 0xf5: _stprintf(buffer, _T("sbb a,e")); break;
		case 0xf6: _stprintf(buffer, _T("sbb a,h")); break;
		case 0xf7: _stprintf(buffer, _T("sbb a,l")); break;
		case 0xf8: _stprintf(buffer, _T("eqa a,v")); break;
		case 0xf9: _stprintf(buffer, _T("eqa a,a")); break;
		case 0xfa: _stprintf(buffer, _T("eqa a,b")); break;
		case 0xfb: _stprintf(buffer, _T("eqa a,c")); break;
		case 0xfc: _stprintf(buffer, _T("eqa a,d")); break;
		case 0xfd: _stprintf(buffer, _T("eqa a,e")); break;
		case 0xfe: _stprintf(buffer, _T("eqa a,h")); break;
		case 0xff: _stprintf(buffer, _T("eqa a,l")); break;
		default: _stprintf(buffer, _T("db 60h,%2xh"), b);
		}
		break;
	case 0x61: _stprintf(buffer, _T("daa")); break;
	case 0x62: _stprintf(buffer, _T("reti")); break;
	case 0x63: _stprintf(buffer, _T("calb")); break;
	case 0x64:
		switch(b = getb()) {
		case 0x08: _stprintf(buffer, _T("ani v,%2xh"), getb()); break;
		case 0x09: _stprintf(buffer, _T("ani a,%2xh"), getb()); break;
		case 0x0a: _stprintf(buffer, _T("ani b,%2xh"), getb()); break;
		case 0x0b: _stprintf(buffer, _T("ani c,%2xh"), getb()); break;
		case 0x0c: _stprintf(buffer, _T("ani d,%2xh"), getb()); break;
		case 0x0d: _stprintf(buffer, _T("ani e,%2xh"), getb()); break;
		case 0x0e: _stprintf(buffer, _T("ani h,%2xh"), getb()); break;
		case 0x0f: _stprintf(buffer, _T("ani l,%2xh"), getb()); break;
		case 0x10: _stprintf(buffer, _T("xri v,%2xh"), getb()); break;
		case 0x11: _stprintf(buffer, _T("xri a,%2xh"), getb()); break;
		case 0x12: _stprintf(buffer, _T("xri b,%2xh"), getb()); break;
		case 0x13: _stprintf(buffer, _T("xri c,%2xh"), getb()); break;
		case 0x14: _stprintf(buffer, _T("xri d,%2xh"), getb()); break;
		case 0x15: _stprintf(buffer, _T("xri e,%2xh"), getb()); break;
		case 0x16: _stprintf(buffer, _T("xri h,%2xh"), getb()); break;
		case 0x17: _stprintf(buffer, _T("xri l,%2xh"), getb()); break;
		case 0x18: _stprintf(buffer, _T("ori v,%2xh"), getb()); break;
		case 0x19: _stprintf(buffer, _T("ori a,%2xh"), getb()); break;
		case 0x1a: _stprintf(buffer, _T("ori b,%2xh"), getb()); break;
		case 0x1b: _stprintf(buffer, _T("ori c,%2xh"), getb()); break;
		case 0x1c: _stprintf(buffer, _T("ori d,%2xh"), getb()); break;
		case 0x1d: _stprintf(buffer, _T("ori e,%2xh"), getb()); break;
		case 0x1e: _stprintf(buffer, _T("ori h,%2xh"), getb()); break;
		case 0x1f: _stprintf(buffer, _T("ori l,%2xh"), getb()); break;
		case 0x20: _stprintf(buffer, _T("adinc v,%2xh"), getb()); break;
		case 0x21: _stprintf(buffer, _T("adinc a,%2xh"), getb()); break;
		case 0x22: _stprintf(buffer, _T("adinc b,%2xh"), getb()); break;
		case 0x23: _stprintf(buffer, _T("adinc c,%2xh"), getb()); break;
		case 0x24: _stprintf(buffer, _T("adinc d,%2xh"), getb()); break;
		case 0x25: _stprintf(buffer, _T("adinc e,%2xh"), getb()); break;
		case 0x26: _stprintf(buffer, _T("adinc h,%2xh"), getb()); break;
		case 0x27: _stprintf(buffer, _T("adinc l,%2xh"), getb()); break;
		case 0x28: _stprintf(buffer, _T("gti v,%2xh"), getb()); break;
		case 0x29: _stprintf(buffer, _T("gti a,%2xh"), getb()); break;
		case 0x2a: _stprintf(buffer, _T("gti b,%2xh"), getb()); break;
		case 0x2b: _stprintf(buffer, _T("gti c,%2xh"), getb()); break;
		case 0x2c: _stprintf(buffer, _T("gti d,%2xh"), getb()); break;
		case 0x2d: _stprintf(buffer, _T("gti e,%2xh"), getb()); break;
		case 0x2e: _stprintf(buffer, _T("gti h,%2xh"), getb()); break;
		case 0x2f: _stprintf(buffer, _T("gti l,%2xh"), getb()); break;
		case 0x30: _stprintf(buffer, _T("suinb v,%2xh"), getb()); break;
		case 0x31: _stprintf(buffer, _T("suinb a,%2xh"), getb()); break;
		case 0x32: _stprintf(buffer, _T("suinb b,%2xh"), getb()); break;
		case 0x33: _stprintf(buffer, _T("suinb c,%2xh"), getb()); break;
		case 0x34: _stprintf(buffer, _T("suinb d,%2xh"), getb()); break;
		case 0x35: _stprintf(buffer, _T("suinb e,%2xh"), getb()); break;
		case 0x36: _stprintf(buffer, _T("suinb h,%2xh"), getb()); break;
		case 0x37: _stprintf(buffer, _T("suinb l,%2xh"), getb()); break;
		case 0x38: _stprintf(buffer, _T("lti v,%2xh"), getb()); break;
		case 0x39: _stprintf(buffer, _T("lti a,%2xh"), getb()); break;
		case 0x3a: _stprintf(buffer, _T("lti b,%2xh"), getb()); break;
		case 0x3b: _stprintf(buffer, _T("lti c,%2xh"), getb()); break;
		case 0x3c: _stprintf(buffer, _T("lti d,%2xh"), getb()); break;
		case 0x3d: _stprintf(buffer, _T("lti e,%2xh"), getb()); break;
		case 0x3e: _stprintf(buffer, _T("lti h,%2xh"), getb()); break;
		case 0x3f: _stprintf(buffer, _T("lti l,%2xh"), getb()); break;
		case 0x40: _stprintf(buffer, _T("adi v,%2xh"), getb()); break;
		case 0x41: _stprintf(buffer, _T("adi a,%2xh"), getb()); break;
		case 0x42: _stprintf(buffer, _T("adi b,%2xh"), getb()); break;
		case 0x43: _stprintf(buffer, _T("adi c,%2xh"), getb()); break;
		case 0x44: _stprintf(buffer, _T("adi d,%2xh"), getb()); break;
		case 0x45: _stprintf(buffer, _T("adi e,%2xh"), getb()); break;
		case 0x46: _stprintf(buffer, _T("adi h,%2xh"), getb()); break;
		case 0x47: _stprintf(buffer, _T("adi l,%2xh"), getb()); break;
		case 0x48: _stprintf(buffer, _T("oni v,%2xh"), getb()); break;
		case 0x49: _stprintf(buffer, _T("oni a,%2xh"), getb()); break;
		case 0x4a: _stprintf(buffer, _T("oni b,%2xh"), getb()); break;
		case 0x4b: _stprintf(buffer, _T("oni c,%2xh"), getb()); break;
		case 0x4c: _stprintf(buffer, _T("oni d,%2xh"), getb()); break;
		case 0x4d: _stprintf(buffer, _T("oni e,%2xh"), getb()); break;
		case 0x4e: _stprintf(buffer, _T("oni h,%2xh"), getb()); break;
		case 0x4f: _stprintf(buffer, _T("oni l,%2xh"), getb()); break;
		case 0x50: _stprintf(buffer, _T("aci v,%2xh"), getb()); break;
		case 0x51: _stprintf(buffer, _T("aci a,%2xh"), getb()); break;
		case 0x52: _stprintf(buffer, _T("aci b,%2xh"), getb()); break;
		case 0x53: _stprintf(buffer, _T("aci c,%2xh"), getb()); break;
		case 0x54: _stprintf(buffer, _T("aci d,%2xh"), getb()); break;
		case 0x55: _stprintf(buffer, _T("aci e,%2xh"), getb()); break;
		case 0x56: _stprintf(buffer, _T("aci h,%2xh"), getb()); break;
		case 0x57: _stprintf(buffer, _T("aci l,%2xh"), getb()); break;
		case 0x58: _stprintf(buffer, _T("offi v,%2xh"), getb()); break;
		case 0x59: _stprintf(buffer, _T("offi a,%2xh"), getb()); break;
		case 0x5a: _stprintf(buffer, _T("offi b,%2xh"), getb()); break;
		case 0x5b: _stprintf(buffer, _T("offi c,%2xh"), getb()); break;
		case 0x5c: _stprintf(buffer, _T("offi d,%2xh"), getb()); break;
		case 0x5d: _stprintf(buffer, _T("offi e,%2xh"), getb()); break;
		case 0x5e: _stprintf(buffer, _T("offi h,%2xh"), getb()); break;
		case 0x5f: _stprintf(buffer, _T("offi l,%2xh"), getb()); break;
		case 0x60: _stprintf(buffer, _T("sui v,%2xh"), getb()); break;
		case 0x61: _stprintf(buffer, _T("sui a,%2xh"), getb()); break;
		case 0x62: _stprintf(buffer, _T("sui b,%2xh"), getb()); break;
		case 0x63: _stprintf(buffer, _T("sui c,%2xh"), getb()); break;
		case 0x64: _stprintf(buffer, _T("sui d,%2xh"), getb()); break;
		case 0x65: _stprintf(buffer, _T("sui e,%2xh"), getb()); break;
		case 0x66: _stprintf(buffer, _T("sui h,%2xh"), getb()); break;
		case 0x67: _stprintf(buffer, _T("sui l,%2xh"), getb()); break;
		case 0x68: _stprintf(buffer, _T("nei v,%2xh"), getb()); break;
		case 0x69: _stprintf(buffer, _T("nei a,%2xh"), getb()); break;
		case 0x6a: _stprintf(buffer, _T("nei b,%2xh"), getb()); break;
		case 0x6b: _stprintf(buffer, _T("nei c,%2xh"), getb()); break;
		case 0x6c: _stprintf(buffer, _T("nei d,%2xh"), getb()); break;
		case 0x6d: _stprintf(buffer, _T("nei e,%2xh"), getb()); break;
		case 0x6e: _stprintf(buffer, _T("nei h,%2xh"), getb()); break;
		case 0x6f: _stprintf(buffer, _T("nei l,%2xh"), getb()); break;
		case 0x70: _stprintf(buffer, _T("sbi v,%2xh"), getb()); break;
		case 0x71: _stprintf(buffer, _T("sbi a,%2xh"), getb()); break;
		case 0x72: _stprintf(buffer, _T("sbi b,%2xh"), getb()); break;
		case 0x73: _stprintf(buffer, _T("sbi c,%2xh"), getb()); break;
		case 0x74: _stprintf(buffer, _T("sbi d,%2xh"), getb()); break;
		case 0x75: _stprintf(buffer, _T("sbi e,%2xh"), getb()); break;
		case 0x76: _stprintf(buffer, _T("sbi h,%2xh"), getb()); break;
		case 0x77: _stprintf(buffer, _T("sbi l,%2xh"), getb()); break;
		case 0x78: _stprintf(buffer, _T("eqi v,%2xh"), getb()); break;
		case 0x79: _stprintf(buffer, _T("eqi a,%2xh"), getb()); break;
		case 0x7a: _stprintf(buffer, _T("eqi b,%2xh"), getb()); break;
		case 0x7b: _stprintf(buffer, _T("eqi c,%2xh"), getb()); break;
		case 0x7c: _stprintf(buffer, _T("eqi d,%2xh"), getb()); break;
		case 0x7d: _stprintf(buffer, _T("eqi e,%2xh"), getb()); break;
		case 0x7e: _stprintf(buffer, _T("eqi h,%2xh"), getb()); break;
		case 0x7f: _stprintf(buffer, _T("eqi l,%2xh"), getb()); break;
		case 0x88: _stprintf(buffer, _T("ani pa,%2xh"), getb()); break;
		case 0x89: _stprintf(buffer, _T("ani pb,%2xh"), getb()); break;
		case 0x8a: _stprintf(buffer, _T("ani pc,%2xh"), getb()); break;
		case 0x8b: _stprintf(buffer, _T("ani mk,%2xh"), getb()); break;
		case 0x90: _stprintf(buffer, _T("xri pa,%2xh"), getb()); break;
		case 0x91: _stprintf(buffer, _T("xri pb,%2xh"), getb()); break;
		case 0x92: _stprintf(buffer, _T("xri pc,%2xh"), getb()); break;
		case 0x93: _stprintf(buffer, _T("xri mk,%2xh"), getb()); break;
		case 0x98: _stprintf(buffer, _T("ori pa,%2xh"), getb()); break;
		case 0x99: _stprintf(buffer, _T("ori pb,%2xh"), getb()); break;
		case 0x9a: _stprintf(buffer, _T("ori pc,%2xh"), getb()); break;
		case 0x9b: _stprintf(buffer, _T("ori mk,%2xh"), getb()); break;
		case 0xa0: _stprintf(buffer, _T("adinc pa,%2xh"), getb()); break;
		case 0xa1: _stprintf(buffer, _T("adinc pb,%2xh"), getb()); break;
		case 0xa2: _stprintf(buffer, _T("adinc pc,%2xh"), getb()); break;
		case 0xa3: _stprintf(buffer, _T("adinc mk,%2xh"), getb()); break;
		case 0xa8: _stprintf(buffer, _T("gti pa,%2xh"), getb()); break;
		case 0xa9: _stprintf(buffer, _T("gti pb,%2xh"), getb()); break;
		case 0xaa: _stprintf(buffer, _T("gti pc,%2xh"), getb()); break;
		case 0xab: _stprintf(buffer, _T("gti mk,%2xh"), getb()); break;
		case 0xb0: _stprintf(buffer, _T("suinb pa,%2xh"), getb()); break;
		case 0xb1: _stprintf(buffer, _T("suinb pb,%2xh"), getb()); break;
		case 0xb2: _stprintf(buffer, _T("suinb pc,%2xh"), getb()); break;
		case 0xb3: _stprintf(buffer, _T("suinb mk,%2xh"), getb()); break;
		case 0xb8: _stprintf(buffer, _T("lti pa,%2xh"), getb()); break;
		case 0xb9: _stprintf(buffer, _T("lti pb,%2xh"), getb()); break;
		case 0xba: _stprintf(buffer, _T("lti pc,%2xh"), getb()); break;
		case 0xbb: _stprintf(buffer, _T("lti mk,%2xh"), getb()); break;
		case 0xc0: _stprintf(buffer, _T("adi pa,%2xh"), getb()); break;
		case 0xc1: _stprintf(buffer, _T("adi pb,%2xh"), getb()); break;
		case 0xc2: _stprintf(buffer, _T("adi pc,%2xh"), getb()); break;
		case 0xc3: _stprintf(buffer, _T("adi mk,%2xh"), getb()); break;
		case 0xc8: _stprintf(buffer, _T("oni pa,%2xh"), getb()); break;
		case 0xc9: _stprintf(buffer, _T("oni pb,%2xh"), getb()); break;
		case 0xca: _stprintf(buffer, _T("oni pc,%2xh"), getb()); break;
		case 0xcb: _stprintf(buffer, _T("oni mk,%2xh"), getb()); break;
		case 0xd0: _stprintf(buffer, _T("aci pa,%2xh"), getb()); break;
		case 0xd1: _stprintf(buffer, _T("aci pb,%2xh"), getb()); break;
		case 0xd2: _stprintf(buffer, _T("aci pc,%2xh"), getb()); break;
		case 0xd3: _stprintf(buffer, _T("aci mk,%2xh"), getb()); break;
		case 0xd8: _stprintf(buffer, _T("offi pa,%2xh"), getb()); break;
		case 0xd9: _stprintf(buffer, _T("offi pb,%2xh"), getb()); break;
		case 0xda: _stprintf(buffer, _T("offi pc,%2xh"), getb()); break;
		case 0xdb: _stprintf(buffer, _T("offi mk,%2xh"), getb()); break;
		case 0xe0: _stprintf(buffer, _T("sui pa,%2xh"), getb()); break;
		case 0xe1: _stprintf(buffer, _T("sui pb,%2xh"), getb()); break;
		case 0xe2: _stprintf(buffer, _T("sui pc,%2xh"), getb()); break;
		case 0xe3: _stprintf(buffer, _T("sui mk,%2xh"), getb()); break;
		case 0xe8: _stprintf(buffer, _T("nei pa,%2xh"), getb()); break;
		case 0xe9: _stprintf(buffer, _T("nei pb,%2xh"), getb()); break;
		case 0xea: _stprintf(buffer, _T("nei pc,%2xh"), getb()); break;
		case 0xeb: _stprintf(buffer, _T("nei mk,%2xh"), getb()); break;
		case 0xf0: _stprintf(buffer, _T("sbi pa,%2xh"), getb()); break;
		case 0xf1: _stprintf(buffer, _T("sbi pb,%2xh"), getb()); break;
		case 0xf2: _stprintf(buffer, _T("sbi pc,%2xh"), getb()); break;
		case 0xf3: _stprintf(buffer, _T("sbi mk,%2xh"), getb()); break;
		case 0xf8: _stprintf(buffer, _T("eqi pa,%2xh"), getb()); break;
		case 0xf9: _stprintf(buffer, _T("eqi pb,%2xh"), getb()); break;
		case 0xfa: _stprintf(buffer, _T("eqi pc,%2xh"), getb()); break;
		case 0xfb: _stprintf(buffer, _T("eqi mk,%2xh"), getb()); break;
		default: _stprintf(buffer, _T("db 64h,%2xh"), b);
		}
		break;
	case 0x65: wa = getwa(); _stprintf(buffer, _T("neiw v.%xh,%2xh"), wa, getb()); break;
	case 0x66: _stprintf(buffer, _T("sui a,%2xh"), getb()); break;
	case 0x67: _stprintf(buffer, _T("nei a,%2xh"), getb()); break;
	case 0x68: _stprintf(buffer, _T("mvi v,%2xh"), getb()); break;
	case 0x69: _stprintf(buffer, _T("mvi a,%2xh"), getb()); break;
	case 0x6a: _stprintf(buffer, _T("mvi b,%2xh"), getb()); break;
	case 0x6b: _stprintf(buffer, _T("mvi c,%2xh"), getb()); break;
	case 0x6c: _stprintf(buffer, _T("mvi d,%2xh"), getb()); break;
	case 0x6d: _stprintf(buffer, _T("mvi e,%2xh"), getb()); break;
	case 0x6e: _stprintf(buffer, _T("mvi h,%2xh"), getb()); break;
	case 0x6f: _stprintf(buffer, _T("mvi l,%2xh"), getb()); break;
	
	case 0x70:
		switch(b = getb()) {
		case 0x0e: _stprintf(buffer, _T("sspd %4xh"), getw()); break;
		case 0x0f: _stprintf(buffer, _T("lspd %4xh"), getw()); break;
		case 0x1e: _stprintf(buffer, _T("sbcd %4xh"), getw()); break;
		case 0x1f: _stprintf(buffer, _T("lbcd %4xh"), getw()); break;
		case 0x2e: _stprintf(buffer, _T("sded %4xh"), getw()); break;
		case 0x2f: _stprintf(buffer, _T("lded %4xh"), getw()); break;
		case 0x3e: _stprintf(buffer, _T("shld %4xh"), getw()); break;
		case 0x3f: _stprintf(buffer, _T("lhld %4xh"), getw()); break;
		case 0x68: _stprintf(buffer, _T("mov v,%4xh"), getw()); break;
		case 0x69: _stprintf(buffer, _T("mov a,%4xh"), getw()); break;
		case 0x6a: _stprintf(buffer, _T("mov b,%4xh"), getw()); break;
		case 0x6b: _stprintf(buffer, _T("mov c,%4xh"), getw()); break;
		case 0x6c: _stprintf(buffer, _T("mov d,%4xh"), getw()); break;
		case 0x6d: _stprintf(buffer, _T("mov e,%4xh"), getw()); break;
		case 0x6e: _stprintf(buffer, _T("mov h,%4xh"), getw()); break;
		case 0x6f: _stprintf(buffer, _T("mov l,%4xh"), getw()); break;
		case 0x78: _stprintf(buffer, _T("mov %4xh,v"), getw()); break;
		case 0x79: _stprintf(buffer, _T("mov %4xh,a"), getw()); break;
		case 0x7a: _stprintf(buffer, _T("mov %4xh,b"), getw()); break;
		case 0x7b: _stprintf(buffer, _T("mov %4xh,c"), getw()); break;
		case 0x7c: _stprintf(buffer, _T("mov %4xh,d"), getw()); break;
		case 0x7d: _stprintf(buffer, _T("mov %4xh,e"), getw()); break;
		case 0x7e: _stprintf(buffer, _T("mov %4xh,h"), getw()); break;
		case 0x7f: _stprintf(buffer, _T("mov %4xh,l"), getw()); break;
		case 0x89: _stprintf(buffer, _T("anax b")); break;
		case 0x8a: _stprintf(buffer, _T("anax d")); break;
		case 0x8b: _stprintf(buffer, _T("anax h")); break;
		case 0x8c: _stprintf(buffer, _T("anax d+")); break;
		case 0x8d: _stprintf(buffer, _T("anax h+")); break;
		case 0x8e: _stprintf(buffer, _T("anax d-")); break;
		case 0x8f: _stprintf(buffer, _T("anax h-")); break;
		case 0x91: _stprintf(buffer, _T("xrax b")); break;
		case 0x92: _stprintf(buffer, _T("xrax d")); break;
		case 0x93: _stprintf(buffer, _T("xrax h")); break;
		case 0x94: _stprintf(buffer, _T("xrax d+")); break;
		case 0x95: _stprintf(buffer, _T("xrax h+")); break;
		case 0x96: _stprintf(buffer, _T("xrax d-")); break;
		case 0x97: _stprintf(buffer, _T("xrax h-")); break;
		case 0x99: _stprintf(buffer, _T("orax b")); break;
		case 0x9a: _stprintf(buffer, _T("orax d")); break;
		case 0x9b: _stprintf(buffer, _T("orax h")); break;
		case 0x9c: _stprintf(buffer, _T("orax d+")); break;
		case 0x9d: _stprintf(buffer, _T("orax h+")); break;
		case 0x9e: _stprintf(buffer, _T("orax d-")); break;
		case 0x9f: _stprintf(buffer, _T("orax h-")); break;
		case 0xa1: _stprintf(buffer, _T("addncx b")); break;
		case 0xa2: _stprintf(buffer, _T("addncx d")); break;
		case 0xa3: _stprintf(buffer, _T("addncx h")); break;
		case 0xa4: _stprintf(buffer, _T("addncx d+")); break;
		case 0xa5: _stprintf(buffer, _T("addncx h+")); break;
		case 0xa6: _stprintf(buffer, _T("addncx d-")); break;
		case 0xa7: _stprintf(buffer, _T("addncx h-")); break;
		case 0xa9: _stprintf(buffer, _T("gtax b")); break;
		case 0xaa: _stprintf(buffer, _T("gtax d")); break;
		case 0xab: _stprintf(buffer, _T("gtax h")); break;
		case 0xac: _stprintf(buffer, _T("gtax d+")); break;
		case 0xad: _stprintf(buffer, _T("gtax h+")); break;
		case 0xae: _stprintf(buffer, _T("gtax d-")); break;
		case 0xaf: _stprintf(buffer, _T("gtax h-")); break;
		case 0xb1: _stprintf(buffer, _T("subnbx b")); break;
		case 0xb2: _stprintf(buffer, _T("subnbx d")); break;
		case 0xb3: _stprintf(buffer, _T("subnbx h")); break;
		case 0xb4: _stprintf(buffer, _T("subnbx d+")); break;
		case 0xb5: _stprintf(buffer, _T("subnbx h+")); break;
		case 0xb6: _stprintf(buffer, _T("subnbx d-")); break;
		case 0xb7: _stprintf(buffer, _T("subnbx h-")); break;
		case 0xb9: _stprintf(buffer, _T("ltax b")); break;
		case 0xba: _stprintf(buffer, _T("ltax d")); break;
		case 0xbb: _stprintf(buffer, _T("ltax h")); break;
		case 0xbc: _stprintf(buffer, _T("ltax d+")); break;
		case 0xbd: _stprintf(buffer, _T("ltax h+")); break;
		case 0xbe: _stprintf(buffer, _T("ltax d-")); break;
		case 0xbf: _stprintf(buffer, _T("ltax h-")); break;
		case 0xc1: _stprintf(buffer, _T("addx b")); break;
		case 0xc2: _stprintf(buffer, _T("addx d")); break;
		case 0xc3: _stprintf(buffer, _T("addx h")); break;
		case 0xc4: _stprintf(buffer, _T("addx d+")); break;
		case 0xc5: _stprintf(buffer, _T("addx h+")); break;
		case 0xc6: _stprintf(buffer, _T("addx d-")); break;
		case 0xc7: _stprintf(buffer, _T("addx h-")); break;
		case 0xc9: _stprintf(buffer, _T("onax b")); break;
		case 0xca: _stprintf(buffer, _T("onax d")); break;
		case 0xcb: _stprintf(buffer, _T("onax h")); break;
		case 0xcc: _stprintf(buffer, _T("onax d+")); break;
		case 0xcd: _stprintf(buffer, _T("onax h+")); break;
		case 0xce: _stprintf(buffer, _T("onax d-")); break;
		case 0xcf: _stprintf(buffer, _T("onax h-")); break;
		case 0xd1: _stprintf(buffer, _T("adcx b")); break;
		case 0xd2: _stprintf(buffer, _T("adcx d")); break;
		case 0xd3: _stprintf(buffer, _T("adcx h")); break;
		case 0xd4: _stprintf(buffer, _T("adcx d+")); break;
		case 0xd5: _stprintf(buffer, _T("adcx h+")); break;
		case 0xd6: _stprintf(buffer, _T("adcx d-")); break;
		case 0xd7: _stprintf(buffer, _T("adcx h-")); break;
		case 0xd9: _stprintf(buffer, _T("offax b")); break;
		case 0xda: _stprintf(buffer, _T("offax d")); break;
		case 0xdb: _stprintf(buffer, _T("offax h")); break;
		case 0xdc: _stprintf(buffer, _T("offax d+")); break;
		case 0xdd: _stprintf(buffer, _T("offax h+")); break;
		case 0xde: _stprintf(buffer, _T("offax d-")); break;
		case 0xdf: _stprintf(buffer, _T("offax h-")); break;
		case 0xe1: _stprintf(buffer, _T("subx b")); break;
		case 0xe2: _stprintf(buffer, _T("subx d")); break;
		case 0xe3: _stprintf(buffer, _T("subx h")); break;
		case 0xe4: _stprintf(buffer, _T("subx d+")); break;
		case 0xe5: _stprintf(buffer, _T("subx h+")); break;
		case 0xe6: _stprintf(buffer, _T("subx d-")); break;
		case 0xe7: _stprintf(buffer, _T("subx h-")); break;
		case 0xe9: _stprintf(buffer, _T("neax b")); break;
		case 0xea: _stprintf(buffer, _T("neax d")); break;
		case 0xeb: _stprintf(buffer, _T("neax h")); break;
		case 0xec: _stprintf(buffer, _T("neax d+")); break;
		case 0xed: _stprintf(buffer, _T("neax h+")); break;
		case 0xee: _stprintf(buffer, _T("neax d-")); break;
		case 0xef: _stprintf(buffer, _T("neax h-")); break;
		case 0xf1: _stprintf(buffer, _T("sbbx b")); break;
		case 0xf2: _stprintf(buffer, _T("sbbx d")); break;
		case 0xf3: _stprintf(buffer, _T("sbbx h")); break;
		case 0xf4: _stprintf(buffer, _T("sbbx d+")); break;
		case 0xf5: _stprintf(buffer, _T("sbbx h+")); break;
		case 0xf6: _stprintf(buffer, _T("sbbx d-")); break;
		case 0xf7: _stprintf(buffer, _T("sbbx h-")); break;
		case 0xf9: _stprintf(buffer, _T("eqax b")); break;
		case 0xfa: _stprintf(buffer, _T("eqax d")); break;
		case 0xfb: _stprintf(buffer, _T("eqax h")); break;
		case 0xfc: _stprintf(buffer, _T("eqax d+")); break;
		case 0xfd: _stprintf(buffer, _T("eqax h+")); break;
		case 0xfe: _stprintf(buffer, _T("eqax d-")); break;
		case 0xff: _stprintf(buffer, _T("eqax h-")); break;
		default: _stprintf(buffer, _T("db 70h,%2xh"), b);
		}
		break;
	case 0x71: wa = getwa(); _stprintf(buffer, _T("mviw v.%xh,%2xh"), wa, getb()); break;
	case 0x72: _stprintf(buffer, _T("softi")); break;
	case 0x73: _stprintf(buffer, _T("jb")); break;
	case 0x74:
		switch(b = getb()) {
		case 0x88: _stprintf(buffer, _T("anaw v.%x"), getwa()); break;
		case 0x90: _stprintf(buffer, _T("xraw v.%x"), getwa()); break;
		case 0x98: _stprintf(buffer, _T("oraw v.%x"), getwa()); break;
		case 0xa0: _stprintf(buffer, _T("addncw v.%x"), getwa()); break;
		case 0xa8: _stprintf(buffer, _T("gtaw v.%x"), getwa()); break;
		case 0xb0: _stprintf(buffer, _T("subnbw v.%x"), getwa()); break;
		case 0xb8: _stprintf(buffer, _T("ltaw v.%x"), getwa()); break;
		case 0xc0: _stprintf(buffer, _T("addw v.%x"), getwa()); break;
		case 0xc8: _stprintf(buffer, _T("onaw v.%x"), getwa()); break;
		case 0xd0: _stprintf(buffer, _T("adcw v.%x"), getwa()); break;
		case 0xd8: _stprintf(buffer, _T("offaw v.%x"), getwa()); break;
		case 0xe0: _stprintf(buffer, _T("subw v.%x"), getwa()); break;
		case 0xe8: _stprintf(buffer, _T("neaw v.%x"), getwa()); break;
		case 0xf0: _stprintf(buffer, _T("sbbw v.%x"), getwa()); break;
		case 0xf8: _stprintf(buffer, _T("eqaw v.%x"), getwa()); break;
		default: _stprintf(buffer, _T("db 74h,%2xh"), b);
		}
		break;
	case 0x75: wa = getwa(); _stprintf(buffer, _T("eqiw v.%xh,%2xh"), wa, getb()); break;
	case 0x76: _stprintf(buffer, _T("sbi a,%2xh"), getb()); break;
	case 0x77: _stprintf(buffer, _T("eqi a,%2xh"), getb()); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		_stprintf(buffer, _T("calf %3x"), 0x800 | ((b & 7) << 8) | getb()); break;
	
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		_stprintf(buffer, _T("calt %2xh"), 0x80 | ((b & 0x3f) << 1)); break;
		
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		_stprintf(buffer, _T("jr %4xh"), pc + (b & 0x1f)); break;
	
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		_stprintf(buffer, _T("jr %4xh"), pc + ((b & 0x1f) - 0x20)); break;
	
	default: _stprintf(buffer, _T("db %2xh"), b); break;
	}
	return upd7801_dasm_ptr;
}
#endif

void UPD7801::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_UPD7801_INTF0) {
		if(data & mask) {
			IRR |= INTF0;
		} else {
			IRR &= ~INTF0;
		}
	} else if(id == SIG_UPD7801_INTF1) {
		if(data & mask) {
			IRR |= INTF1;
		} else {
			IRR &= ~INTF1;
		}
	} else if(id == SIG_UPD7801_INTF2) {
		if((data & mask) && (MK & 0x20)) {
			IRR |= INTF2;
		} else if(!(data & mask) && !(MK & 0x20)) {
			IRR |= INTF2;
		}
	} else if(id == SIG_UPD7801_WAIT) {
		wait = ((data & mask) != 0);
	}
}

void UPD7801::OP()
{
	uint8 ope = FETCH8();
	
	if((PSW & F_SK) && ope != 0x72) {
		// skip this mnemonic
		switch(ope) {
		case 0x48: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op48[ope].oplen - 2; period += op48[ope].clock; break;
		case 0x4c: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4c[ope].oplen - 2; period += op4c[ope].clock; break;
		case 0x4d: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op4d[ope].oplen - 2; period += op4d[ope].clock; break;
		case 0x60: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op60[ope].oplen - 2; period += op60[ope].clock; break;
		case 0x64: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op64[ope].oplen - 2; period += op64[ope].clock; break;
		case 0x70: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op70[ope].oplen - 2; period += op70[ope].clock; break;
		case 0x74: PSW &= ~(F_SK | F_L0 | F_L1); ope = FETCH8(); PC += op74[ope].oplen - 2; period += op74[ope].clock; break;
		case 0x34: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x69: PSW &= ~(F_SK | F_L0       );                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		case 0x6f: PSW &= ~(F_SK        | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		default:   PSW &= ~(F_SK | F_L0 | F_L1);                 PC += op[ope].oplen - 1;   period += op[ope].clock;   break;
		}
		return;
	}
	period += op[ope].clock;
	
	switch(ope) {
	case 0x00:	// nop
		break;
	case 0x01:	// hlt
		HALT = 1; PC--; break;
	case 0x02:	// inx sp
		SP++; break;
	case 0x03:	// dcx sp
		SP--; break;
	case 0x04:	// lxi sp,word
		SP = FETCH16(); break;
	case 0x05:	// aniw wa,byte
		ANIW(); break;
	case 0x06:
		break;
	case 0x07:	// ani a,byte
		ANI(_A); break;
	case 0x08:	// ret
		PC = POP16(); break;
	case 0x09:	// sio
		SIO(); break;
	case 0x0a:	// mov a,b
		_A = _B; break;
	case 0x0b:	// mov a,c
		_A = _C; break;
	case 0x0c:	// mov a,d
		_A = _D; break;
	case 0x0d:	// mov a,e
		_A = _E; break;
	case 0x0e:	// mov a,h
		_A = _H; break;
	case 0x0f:	// mov a,l
		_A = _L; break;
	case 0x10:	// ex
		EX(); break;
	case 0x11:	// exx
		EXX(); break;
	case 0x12:	// inx b
		BC++; break;
	case 0x13:	// dcx b
		BC--; break;
	case 0x14:	// lxi b,word
		BC = FETCH16(); break;
	case 0x15:	// oriw wa,byte
		ORIW(); break;
	case 0x16:	// xri a,byte
		XRI(_A); break;
	case 0x17:	// ori a,byte
		ORI(_A); break;
	case 0x18:	// rets
		PC = POP16(); PSW |= F_SK; break;
	case 0x19:	// stm
		if(!TO) {
			TO = 0x10; UPDATE_PORTC(0);
		}
		STM(); break;
	case 0x1a:	// mov b, a
		_B = _A; break;
	case 0x1b:	// mov c, a
		_C = _A; break;
	case 0x1c:	// mov d, a
		_D = _A; break;
	case 0x1d:	// mov e, a
		_E = _A; break;
	case 0x1e:	// mov h, a
		_H = _A; break;
	case 0x1f:	// mov l, a
		_L = _A; break;
	case 0x20:	// inrw wa
		INRW(); break;
	case 0x21:	// table
		BC = RM16(PC + _A + 1); break;
	case 0x22:	// inx d
		DE++; break;
	case 0x23:	// dcx d
		DE--; break;
	case 0x24:	// lxi d,word
		DE = FETCH16(); break;
	case 0x25:	// gtiw wa,byte
		GTIW(); break;
	case 0x26:	// adinc a,byte
		ADINC(_A); break;
	case 0x27:	// gti a,byte
		GTI(_A); break;
	case 0x28:	// ldaw wa
		_A = RM8(FETCHWA()); break;
	case 0x29:	// ldax b
		_A = RM8(BC); break;
	case 0x2a:	// ldax d
		_A = RM8(DE); break;
	case 0x2b:	// ldax h
		_A = RM8(HL); break;
	case 0x2c:	// ldax d+
		_A = RM8(DE++);; break;
	case 0x2d:	// ldax h+
		_A = RM8(HL++); break;
	case 0x2e:	// ldax d-
		_A = RM8(DE--); break;
	case 0x2f:	// ldax h-
		_A = RM8(HL--); break;
	case 0x30:	// dcrw wa
		DCRW(); break;
	case 0x31:	// block
		BLOCK(); break;
	case 0x32:	// inx h
		HL++; break;
	case 0x33:	// dcx h
		HL--; break;
	case 0x34:	// lxi h,word
		if(PSW & F_L0) {
			PC += 2;
		} else {
			HL = FETCH16();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x35:	// ltiw wa,byte
		LTIW(); break;
	case 0x36:	// suinb a,byte
		SUINB(_A); break;
	case 0x37:	// lti a,byte
		LTI(_A); break;
	case 0x38:	// staw wa
		WM8(FETCHWA(), _A); break;
	case 0x39:	// stax b
		WM8(BC, _A); break;
	case 0x3a:	// stax d
		WM8(DE, _A); break;
	case 0x3b:	// stax h
		WM8(HL, _A); break;
	case 0x3c:	// stax d+
		WM8(DE++, _A); break;
	case 0x3d:	// stax h+
		WM8(HL++, _A); break;
	case 0x3e:	// stax d-
		WM8(DE--, _A); break;
	case 0x3f:	// stax h-
		WM8(HL--, _A); break;
	case 0x40:
		break;
	case 0x41:	// inr a
		INR(_A); break;
	case 0x42:	// inr b
		INR(_B); break;
	case 0x43:	// inr c
		INR(_C); break;
	case 0x44:	// call word
		CALL(); break;
	case 0x45:	// oniw wa,byte
		ONIW(); break;
	case 0x46:	// adi a,byte
		ADI(_A); break;
	case 0x47:	// oni a,byte
		ONI(_A); break;
	case 0x48:	// 48 xx
		OP48(); break;
	case 0x49:	// mvix b,byte
		WM8(BC, FETCH8()); break;
	case 0x4a:	// mvix d,byte
		WM8(DE, FETCH8()); break;
	case 0x4b:	// mvix h,byte
		WM8(HL, FETCH8()); break;
	case 0x4c:	// 4c xx
		OP4C(); break;
	case 0x4d:	// 4d xx
		OP4D(); break;
	case 0x4e:	// jre
	case 0x4f:	// jre
		JRE(ope); break;
	case 0x50:
		break;
	case 0x51:	// dcr a
		DCR(_A); break;
	case 0x52:	// dcr b
		DCR(_B); break;
	case 0x53:	// dcr c
		DCR(_C); break;
	case 0x54:	// jmp word
		PC = FETCH16(); break;
	case 0x55:	// offiw wa,byte
		OFFIW(); break;
	case 0x56:	// aci a,byte
		ACI(_A); break;
	case 0x57:	// offi a,byte
		OFFI(_A); break;
	case 0x58:	// bit 0,wa
		BIT(0); break;
	case 0x59:	// bit 1,wa
		BIT(1); break;
	case 0x5a:	// bit 2,wa
		BIT(2); break;
	case 0x5b:	// bit 3,wa
		BIT(3); break;
	case 0x5c:	// bit 4,wa
		BIT(4); break;
	case 0x5d:	// bit 5,wa
		BIT(5); break;
	case 0x5e:	// bit 6,wa
		BIT(6); break;
	case 0x5f:	// bit 7,wa
		BIT(7); break;
	case 0x60:	// 60 xx
		OP60(); break;
	case 0x61:	// daa
		DAA(); break;
	case 0x62:	// reti
		PC = POP16(); PSW = POP8(); SIRQ = 0; return;
	case 0x63:	// calb
		PUSH16(PC); PC = BC; break;
	case 0x64:	// 64 xx
		OP64(); break;
	case 0x65:	// neiw wa,byte
		NEIW(); break;
	case 0x66:	// sui a,byte
		SUI(_A); break;
	case 0x67:	// nei a,byte
		NEI(_A); break;
	case 0x68:	// mvi v,byte
		_V = FETCH8(); 
		break;
	case 0x69:	// mvi a,byte
		if(PSW & F_L1) {
			PC++;
		} else {
			_A = FETCH8();
		}
		PSW = (PSW & ~F_L0) | F_L1; return;
	case 0x6a:	// mvi b,byte
		_B = FETCH8(); break;
	case 0x6b:	// mvi c,byte
		_C = FETCH8(); break;
	case 0x6c:	// mvi d,byte
		_D = FETCH8(); break;
	case 0x6d:	// mvi e,byte
		_E = FETCH8(); break;
	case 0x6e:	// mvi h,byte
		_H = FETCH8(); break;
	case 0x6f:	// mvi l,byte
		if(PSW & F_L0) {
			PC++;
		} else {
			_L = FETCH8();
		}
		PSW = (PSW & ~F_L1) | F_L0; return;
	case 0x70:	// 70 xx
		OP70(); break;
	case 0x71:	// mviw wa,byte
		MVIW(); break;
	case 0x72:	// softi
		PUSH8(PSW); PUSH16(PC); PSW &= ~F_SK; SIRQ = 1; PC = 0x0060; break;
	case 0x73:	// jb
		PC = BC; break;
	case 0x74:	// 74xx
		OP74(); break;
	case 0x75:	// eqiw wa,byte
		EQIW(); break;
	case 0x76:	// sbi a,byte
		SBI(_A); break;
	case 0x77:	// eqi a,byte
		EQI(_A); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:	// calf
		CALF(ope); break;
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:	// calt
		CALT(ope); break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:	// jr
		PC += ope & 0x1f; break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:	// jr
		PC -= 0x20 - (ope & 0x1f); break;
	default:
		__assume(0);
	}
	PSW &= ~(F_L0 | F_L1);
}

void UPD7801::OP48()
{
	uint8 ope = FETCH8();
	period += op48[ope].clock;
	
	switch(ope) {
	case 0x00:	// skit intf0
		SKIT(INTF0); break;
	case 0x01:	// skit intft
		SKIT(INTFT); break;
	case 0x02:	// skit intf1
		SKIT(INTF1); break;
	case 0x03:	// skit intf2
		SKIT(INTF2); break;
	case 0x04:	// skit intfs
		SKIT(INTFS); break;
	case 0x0a:	// sk cy
		SK(F_CY); break;
	case 0x0c:	// sk z
		SK(F_Z); break;
	case 0x0e:	// push v
		PUSH16(VA); break;
	case 0x0f:	// pop v
		VA = POP16(); break;
	case 0x10:	// sknit intf0
		SKNIT(INTF0); break;
	case 0x11:	// sknit intft
		SKNIT(INTFT); break;
	case 0x12:	// sknit intf1
		SKNIT(INTF1); break;
	case 0x13:	// sknit intf2
		SKNIT(INTF2); break;
	case 0x14:	// sknit intfs
		SKNIT(INTFS); break;
	case 0x1a:	// skn cy
		SKN(F_CY); break;
	case 0x1c:	// skn z
		SKN(F_Z); break;
	case 0x1e:	// push b
		PUSH16(BC); break;
	case 0x1f:	// pop b
		BC = POP16(); break;
	case 0x20:	// ei
		IFF = 3; break;
	case 0x24:	// di
		IFF = 0; break;
	case 0x2a:	// clc
		PSW &= ~F_CY; break;
	case 0x2b:	// stc
		PSW |= F_CY; break;
	case 0x2c:	// pen
		PEN(); break;
	case 0x2d:	// pex
		PEX(); break;
	case 0x2e:	// push d
		PUSH16(DE); break;
	case 0x2f:	// pop d
		DE = POP16(); break;
	case 0x30:	// rll a
		RLL(_A); break;
	case 0x31:	// rlr a
		RLR(_A); break;
	case 0x32:	// rll c
		RLL(_C); break;
	case 0x33:	// rlr c
		RLR(_C); break;
	case 0x34:	// sll a
		SLL(_A); break;
	case 0x35:	// slr a
		SLR(_A); break;
	case 0x36:	// sll c
		SLL(_C); break;
	case 0x37:	// slr c
		SLR(_C); break;
	case 0x38:	// rld
		RLD(); break;
	case 0x39:	// rrd
		RRD(); break;
	case 0x3c:	// per
		PER(); break;
	case 0x3e:	// push h
		PUSH16(HL); break;
	case 0x3f:	// pop h
		HL = POP16(); break;
	default:
		emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 48 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP4C()
{
	uint8 ope = FETCH8();
	period += op4c[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov a,pa
		_A = IN8(P_A); break;
	case 0xc1:	// mov a,pb
		_A = IN8(P_B); break;
	case 0xc2:	// mov a,pc
		_A = IN8(P_C); break;
	case 0xc3:	// mov a,mk
		_A = MK; break;
	case 0xc4:	// mov a,mb ?
		_A = MB; break;
	case 0xc5:	// mov a,mc ?
		_A = MC; break;
	case 0xc6:	// mov a,tm0 ?
		_A = TM0; break;
	case 0xc7:	// mov a,tm1 ?
		_A = TM1; break;
	case 0xc8:	// mov a,s
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		_A = SR; break;
	default:
		if(ope < 0xc0) {
			// in byte
			UPDATE_PORTC(0x20);
			_A = RM8((_B << 8) | ope);
			UPDATE_PORTC(0);
		} else {
			emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 4c %2x\n"), prevPC, ope);
		}
	}
}

void UPD7801::OP4D()
{
	uint8 ope = FETCH8();
	period += op4d[ope].clock;
	
	switch(ope) {
	case 0xc0:	// mov pa,a
		OUT8(P_A, _A); break;
	case 0xc1:	// mov pb,a
		OUT8(P_B, _A); break;
	case 0xc2:	// mov pc,a
		OUT8(P_C, _A); break;
	case 0xc3:	// mov mk,a
		MK = _A; break;
	case 0xc4:	// mov mb,a
		MB = _A; break;
	case 0xc5:	// mov mc,a
		if(MC != _A) {
			MC = _A; UPDATE_PORTC(0); break;
		}
		MC = _A; break;
	case 0xc6:	// mov tm0,a
		TM0 = _A; break;
	case 0xc7:	// mov tm1,a
		TM1 = _A; break;
	case 0xc8:	// mov s,a
		if(!SAK) {
			SAK = 8; UPDATE_PORTC(0);
		}
		SR = _A; break;
	default:
		if(ope < 0xc0) {
			// out byte
			UPDATE_PORTC(0x20);
			WM8((_B << 8) | ope, _A);
			UPDATE_PORTC(0);
		} else {
			emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 4d %2x\n"), prevPC, ope);
		}
	}
}

void UPD7801::OP60()
{
	uint8 ope = FETCH8();
	period += op60[ope].clock;
	
	switch(ope) {
	case 0x08:	// ana v,a
		ANA(_V, _A); break;
	case 0x09:	// ana a,a
		ANA(_A, _A); break;
	case 0x0a:	// ana b,a
		ANA(_B, _A); break;
	case 0x0b:	// ana c,a
		ANA(_C, _A); break;
	case 0x0c:	// ana d,a
		ANA(_D, _A); break;
	case 0x0d:	// ana e,a
		ANA(_E, _A); break;
	case 0x0e:	// ana h,a
		ANA(_H, _A); break;
	case 0x0f:	// ana l,a
		ANA(_L, _A); break;
	case 0x10:	// xra v,a
		XRA(_V, _A); break;
	case 0x11:	// xra a,a
		XRA(_A, _A); break;
	case 0x12:	// xra b,a
		XRA(_B, _A); break;
	case 0x13:	// xra c,a
		XRA(_C, _A); break;
	case 0x14:	// xra d,a
		XRA(_D, _A); break;
	case 0x15:	// xra e,a
		XRA(_E, _A); break;
	case 0x16:	// xra h,a
		XRA(_H, _A); break;
	case 0x17:	// xra l,a
		XRA(_L, _A); break;
	case 0x18:	// ora v,a
		ORA(_V, _A); break;
	case 0x19:	// ora a,a
		ORA(_A, _A); break;
	case 0x1a:	// ora b,a
		ORA(_B, _A); break;
	case 0x1b:	// ora c,a
		ORA(_C, _A); break;
	case 0x1c:	// ora d,a
		ORA(_D, _A); break;
	case 0x1d:	// ora e,a
		ORA(_E, _A); break;
	case 0x1e:	// ora h,a
		ORA(_H, _A); break;
	case 0x1f:	// ora l,a
		ORA(_L, _A); break;
	case 0x20:	// addnc v,a
		ADDNC(_V, _A); break;
	case 0x21:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0x22:	// addnc b,a
		ADDNC(_B, _A); break;
	case 0x23:	// addnc c,a
		ADDNC(_C, _A); break;
	case 0x24:	// addnc d,a
		ADDNC(_D, _A); break;
	case 0x25:	// addnc e,a
		ADDNC(_E, _A); break;
	case 0x26:	// addnc h,a
		ADDNC(_H, _A); break;
	case 0x27:	// addnc l,a
		ADDNC(_L, _A); break;
	case 0x28:	// gta v,a
		GTA(_V, _A); break;
	case 0x29:	// gta a,a
		GTA(_A, _A); break;
	case 0x2a:	// gta b,a
		GTA(_B, _A); break;
	case 0x2b:	// gta c,a
		GTA(_C, _A); break;
	case 0x2c:	// gta d,a
		GTA(_D, _A); break;
	case 0x2d:	// gta e,a
		GTA(_E, _A); break;
	case 0x2e:	// gta h,a
		GTA(_H, _A); break;
	case 0x2f:	// gta l,a
		GTA(_L, _A); break;
	case 0x30:	// subnb v,a
		SUBNB(_V, _A); break;
	case 0x31:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0x32:	// subnb b,a
		SUBNB(_B, _A); break;
	case 0x33:	// subnb c,a
		SUBNB(_C, _A); break;
	case 0x34:	// subnb d,a
		SUBNB(_D, _A); break;
	case 0x35:	// subnb e,a
		SUBNB(_E, _A); break;
	case 0x36:	// subnb h,a
		SUBNB(_H, _A); break;
	case 0x37:	// subnb l,a
		SUBNB(_L, _A); break;
	case 0x38:	// lta v,a
		LTA(_V, _A); break;
	case 0x39:	// lta a,a
		LTA(_A, _A); break;
	case 0x3a:	// lta b,a
		LTA(_B, _A); break;
	case 0x3b:	// lta c,a
		LTA(_C, _A); break;
	case 0x3c:	// lta d,a
		LTA(_D, _A); break;
	case 0x3d:	// lta e,a
		LTA(_E, _A); break;
	case 0x3e:	// lta h,a
		LTA(_H, _A); break;
	case 0x3f:	// lta l,a
		LTA(_L, _A); break;
	case 0x40:	// add v,a
		ADD(_V, _A); break;
	case 0x41:	// add a,a
		ADD(_A, _A); break;
	case 0x42:	// add b,a
		ADD(_B, _A); break;
	case 0x43:	// add c,a
		ADD(_C, _A); break;
	case 0x44:	// add d,a
		ADD(_D, _A); break;
	case 0x45:	// add e,a
		ADD(_E, _A); break;
	case 0x46:	// add h,a
		ADD(_H, _A); break;
	case 0x47:	// add l,a
		ADD(_L, _A); break;
	case 0x50:	// adc v,a
		ADC(_V, _A); break;
	case 0x51:	// adc a,a
		ADC(_A, _A); break;
	case 0x52:	// adc b,a
		ADC(_B, _A); break;
	case 0x53:	// adc c,a
		ADC(_C, _A); break;
	case 0x54:	// adc d,a
		ADC(_D, _A); break;
	case 0x55:	// adc e,a
		ADC(_E, _A); break;
	case 0x56:	// adc h,a
		ADC(_H, _A); break;
	case 0x57:	// adc l,a
		ADC(_L, _A); break;
	case 0x60:	// sub v,a
		SUB(_V, _A); break;
	case 0x61:	// sub a,a
		SUB(_A, _A); break;
	case 0x62:	// sub b,a
		SUB(_B, _A); break;
	case 0x63:	// sub c,a
		SUB(_C, _A); break;
	case 0x64:	// sub d,a
		SUB(_D, _A); break;
	case 0x65:	// sub e,a
		SUB(_E, _A); break;
	case 0x66:	// sub h,a
		SUB(_H, _A); break;
	case 0x67:	// sub l,a
		SUB(_L, _A); break;
	case 0x68:	// nea v,a
		NEA(_V, _A); break;
	case 0x69:	// nea a,a
		NEA(_A, _A); break;
	case 0x6a:	// nea b,a
		NEA(_B, _A); break;
	case 0x6b:	// nea c,a
		NEA(_C, _A); break;
	case 0x6c:	// nea d,a
		NEA(_D, _A); break;
	case 0x6d:	// nea e,a
		NEA(_E, _A); break;
	case 0x6e:	// nea h,a
		NEA(_H, _A); break;
	case 0x6f:	// nea l,a
		NEA(_L, _A); break;
	case 0x70:	// sbb v,a
		SBB(_V, _A); break;
	case 0x71:	// sbb a,a
		SBB(_A, _A); break;
	case 0x72:	// sbb b,a
		SBB(_B, _A); break;
	case 0x73:	// sbb c,a
		SBB(_C, _A); break;
	case 0x74:	// sbb d,a
		SBB(_D, _A); break;
	case 0x75:	// sbb e,a
		SBB(_E, _A); break;
	case 0x76:	// sbb h,a
		SBB(_H, _A); break;
	case 0x77:	// sbb l,a
		SBB(_L, _A); break;
	case 0x78:	// eqa v,a
		EQA(_V, _A); break;
	case 0x79:	// eqa a,a
		EQA(_A, _A); break;
	case 0x7a:	// eqa b,a
		EQA(_B, _A); break;
	case 0x7b:	// eqa c,a
		EQA(_C, _A); break;
	case 0x7c:	// eqa d,a
		EQA(_D, _A); break;
	case 0x7d:	// eqa e,a
		EQA(_E, _A); break;
	case 0x7e:	// eqa h,a
		EQA(_H, _A); break;
	case 0x7f:	// eqa l,a
		EQA(_L, _A); break;
	case 0x88:	// ana a,v
		ANA(_A, _V); break;
	case 0x89:	// ana a,a
		ANA(_A, _A); break;
	case 0x8a:	// ana a,b
		ANA(_A, _B); break;
	case 0x8b:	// ana a,c
		ANA(_A, _C); break;
	case 0x8c:	// ana a,d
		ANA(_A, _D); break;
	case 0x8d:	// ana a,e
		ANA(_A, _E); break;
	case 0x8e:	// ana a,h
		ANA(_A, _H); break;
	case 0x8f:	// ana a,l
		ANA(_A, _L); break;
	case 0x90:	// xra a,v
		XRA(_A, _V); break;
	case 0x91:	// xra a,a
		XRA(_A, _A); break;
	case 0x92:	// xra a,b
		XRA(_A, _B); break;
	case 0x93:	// xra a,c
		XRA(_A, _C); break;
	case 0x94:	// xra a,d
		XRA(_A, _D); break;
	case 0x95:	// xra a,e
		XRA(_A, _E); break;
	case 0x96:	// xra a,h
		XRA(_A, _H); break;
	case 0x97:	// xra a,l
		XRA(_A, _L); break;
	case 0x98:	// ora a,v
		ORA(_A, _V); break;
	case 0x99:	// ora a,a
		ORA(_A, _A); break;
	case 0x9a:	// ora a,b
		ORA(_A, _B); break;
	case 0x9b:	// ora a,c
		ORA(_A, _C); break;
	case 0x9c:	// ora a,d
		ORA(_A, _D); break;
	case 0x9d:	// ora a,e
		ORA(_A, _E); break;
	case 0x9e:	// ora a,h
		ORA(_A, _H); break;
	case 0x9f:	// ora a,l
		ORA(_A, _L); break;
	case 0xa0:	// addnc a,v
		ADDNC(_A, _V); break;
	case 0xa1:	// addnc a,a
		ADDNC(_A, _A); break;
	case 0xa2:	// addnc a,b
		ADDNC(_A, _B); break;
	case 0xa3:	// addnc a,c
		ADDNC(_A, _C); break;
	case 0xa4:	// addnc a,d
		ADDNC(_A, _D); break;
	case 0xa5:	// addnc a,e
		ADDNC(_A, _E); break;
	case 0xa6:	// addnc a,h
		ADDNC(_A, _H); break;
	case 0xa7:	// addnc a,l
		ADDNC(_A, _L); break;
	case 0xa8:	// gta a,v
		GTA(_A, _V); break;
	case 0xa9:	// gta a,a
		GTA(_A, _A); break;
	case 0xaa:	// gta a,b
		GTA(_A, _B); break;
	case 0xab:	// gta a,c
		GTA(_A, _C); break;
	case 0xac:	// gta a,d
		GTA(_A, _D); break;
	case 0xad:	// gta a,e
		GTA(_A, _E); break;
	case 0xae:	// gta a,h
		GTA(_A, _H); break;
	case 0xaf:	// gta a,l
		GTA(_A, _L); break;
	case 0xb0:	// subnb a,v
		SUBNB(_A, _V); break;
	case 0xb1:	// subnb a,a
		SUBNB(_A, _A); break;
	case 0xb2:	// subnb a,b
		SUBNB(_A, _B); break;
	case 0xb3:	// subnb a,c
		SUBNB(_A, _C); break;
	case 0xb4:	// subnb a,d
		SUBNB(_A, _D); break;
	case 0xb5:	// subnb a,e
		SUBNB(_A, _E); break;
	case 0xb6:	// subnb a,h
		SUBNB(_A, _H); break;
	case 0xb7:	// subnb a,l
		SUBNB(_A, _L); break;
	case 0xb8:	// lta a,v
		LTA(_A, _V); break;
	case 0xb9:	// lta a,a
		LTA(_A, _A); break;
	case 0xba:	// lta a,b
		LTA(_A, _B); break;
	case 0xbb:	// lta a,c
		LTA(_A, _C); break;
	case 0xbc:	// lta a,d
		LTA(_A, _D); break;
	case 0xbd:	// lta a,e
		LTA(_A, _E); break;
	case 0xbe:	// lta a,h
		LTA(_A, _H); break;
	case 0xbf:	// lta a,l
		LTA(_A, _L); break;
	case 0xc0:	// add a,v
		ADD(_A, _V); break;
	case 0xc1:	// add a,a
		ADD(_A, _A); break;
	case 0xc2:	// add a,b
		ADD(_A, _B); break;
	case 0xc3:	// add a,c
		ADD(_A, _C); break;
	case 0xc4:	// add a,d
		ADD(_A, _D); break;
	case 0xc5:	// add a,e
		ADD(_A, _E); break;
	case 0xc6:	// add a,h
		ADD(_A, _H); break;
	case 0xc7:	// add a,l
		ADD(_A, _L); break;
	case 0xc8:	// ona a,v
		ONA(_A, _V); break;
	case 0xc9:	// ona a,a
		ONA(_A, _A); break;
	case 0xca:	// ona a,b
		ONA(_A, _B); break;
	case 0xcb:	// ona a,c
		ONA(_A, _C); break;
	case 0xcc:	// ona a,d
		ONA(_A, _D); break;
	case 0xcd:	// ona a,e
		ONA(_A, _E); break;
	case 0xce:	// ona a,h
		ONA(_A, _H); break;
	case 0xcf:	// ona a,l
		ONA(_A, _L); break;
	case 0xd0:	// adc a,v
		ADC(_A, _V); break;
	case 0xd1:	// adc a,a
		ADC(_A, _A); break;
	case 0xd2:	// adc a,b
		ADC(_A, _B); break;
	case 0xd3:	// adc a,c
		ADC(_A, _C); break;
	case 0xd4:	// adc a,d
		ADC(_A, _D); break;
	case 0xd5:	// adc a,e
		ADC(_A, _E); break;
	case 0xd6:	// adc a,h
		ADC(_A, _H); break;
	case 0xd7:	// adc a,l
		ADC(_A, _L); break;
	case 0xd8:	// offa a,v
		OFFA(_A, _V); break;
	case 0xd9:	// offa a,a
		OFFA(_A, _A); break;
	case 0xda:	// offa a,b
		OFFA(_A, _B); break;
	case 0xdb:	// offa a,c
		OFFA(_A, _C); break;
	case 0xdc:	// offa a,d
		OFFA(_A, _D); break;
	case 0xdd:	// offa a,e
		OFFA(_A, _E); break;
	case 0xde:	// offa a,h
		OFFA(_A, _H); break;
	case 0xdf:	// offa a,l
		OFFA(_A, _L); break;
	case 0xe0:	// sub a,v
		SUB(_A, _V); break;
	case 0xe1:	// sub a,a
		SUB(_A, _A); break;
	case 0xe2:	// sub a,b
		SUB(_A, _B); break;
	case 0xe3:	// sub a,c
		SUB(_A, _C); break;
	case 0xe4:	// sub a,d
		SUB(_A, _D); break;
	case 0xe5:	// sub a,e
		SUB(_A, _E); break;
	case 0xe6:	// sub a,h
		SUB(_A, _H); break;
	case 0xe7:	// sub a,l
		SUB(_A, _L); break;
	case 0xe8:	// nea a,v
		NEA(_A, _V); break;
	case 0xe9:	// nea a,a
		NEA(_A, _A); break;
	case 0xea:	// nea a,b
		NEA(_A, _B); break;
	case 0xeb:	// nea a,c
		NEA(_A, _C); break;
	case 0xec:	// nea a,d
		NEA(_A, _D); break;
	case 0xed:	// nea a,e
		NEA(_A, _E); break;
	case 0xee:	// nea a,h
		NEA(_A, _H); break;
	case 0xef:	// nea a,l
		NEA(_A, _L); break;
	case 0xf0:	// sbb a,v
		SBB(_A, _V); break;
	case 0xf1:	// sbb a,a
		SBB(_A, _A); break;
	case 0xf2:	// sbb a,b
		SBB(_A, _B); break;
	case 0xf3:	// sbb a,c
		SBB(_A, _C); break;
	case 0xf4:	// sbb a,d
		SBB(_A, _D); break;
	case 0xf5:	// sbb a,e
		SBB(_A, _E); break;
	case 0xf6:	// sbb a,h
		SBB(_A, _H); break;
	case 0xf7:	// sbb a,l
		SBB(_A, _L); break;
	case 0xf8:	// eqa a,v
		EQA(_A, _V); break;
	case 0xf9:	// eqa a,a
		EQA(_A, _A); break;
	case 0xfa:	// eqa a,b
		EQA(_A, _B); break;
	case 0xfb:	// eqa a,c
		EQA(_A, _C); break;
	case 0xfc:	// eqa a,d
		EQA(_A, _D); break;
	case 0xfd:	// eqa a,e
		EQA(_A, _E); break;
	case 0xfe:	// eqa a,h
		EQA(_A, _H); break;
	case 0xff:	// eqa a,l
		EQA(_A, _L); break;
	default:
		emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 60 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP64()
{
	uint8 ope = FETCH8();
	period += op64[ope].clock;
	
	switch(ope) {
	case 0x08:	// ani v,byte
		ANI(_V); break;
	case 0x09:	// ani a,byte
		ANI(_A); break;
	case 0x0a:	// ani b,byte
		ANI(_B); break;
	case 0x0b:	// ani c,byte
		ANI(_C); break;
	case 0x0c:	// ani d,byte
		ANI(_D); break;
	case 0x0d:	// ani e,byte
		ANI(_E); break;
	case 0x0e:	// ani h,byte
		ANI(_H); break;
	case 0x0f:	// ani l,byte
		ANI(_L); break;
	case 0x10:	// xri v,byte
		XRI(_V); break;
	case 0x11:	// xri a,byte
		XRI(_A); break;
	case 0x12:	// xri b,byte
		XRI(_B); break;
	case 0x13:	// xri c,byte
		XRI(_C); break;
	case 0x14:	// xri d,byte
		XRI(_D); break;
	case 0x15:	// xri e,byte
		XRI(_E); break;
	case 0x16:	// xri h,byte
		XRI(_H); break;
	case 0x17:	// xri l,byte
		XRI(_L); break;
	case 0x18:	// ori v,byte
		ORI(_V); break;
	case 0x19:	// ori a,byte
		ORI(_A); break;
	case 0x1a:	// ori b,byte
		ORI(_B); break;
	case 0x1b:	// ori c,byte
		ORI(_C); break;
	case 0x1c:	// ori d,byte
		ORI(_D); break;
	case 0x1d:	// ori e,byte
		ORI(_E); break;
	case 0x1e:	// ori h,byte
		ORI(_H); break;
	case 0x1f:	// ori l,byte
		ORI(_L); break;
	case 0x20:	// adinc v,byte
		ADINC(_V); break;
	case 0x21:	// adinc a,byte
		ADINC(_A); break;
	case 0x22:	// adinc b,byte
		ADINC(_B); break;
	case 0x23:	// adinc c,byte
		ADINC(_C); break;
	case 0x24:	// adinc d,byte
		ADINC(_D); break;
	case 0x25:	// adinc e,byte
		ADINC(_E); break;
	case 0x26:	// adinc h,byte
		ADINC(_H); break;
	case 0x27:	// adinc l,byte
		ADINC(_L); break;
	case 0x28:	// gti v,byte
		GTI(_V); break;
	case 0x29:	// gti a,byte
		GTI(_A); break;
	case 0x2a:	// gti b,byte
		GTI(_B); break;
	case 0x2b:	// gti c,byte
		GTI(_C); break;
	case 0x2c:	// gti d,byte
		GTI(_D); break;
	case 0x2d:	// gti e,byte
		GTI(_E); break;
	case 0x2e:	// gti h,byte
		GTI(_H); break;
	case 0x2f:	// gti l,byte
		GTI(_L); break;
	case 0x30:	// suinb v,byte
		SUINB(_V); break;
	case 0x31:	// suinb a,byte
		SUINB(_A); break;
	case 0x32:	// suinb b,byte
		SUINB(_B); break;
	case 0x33:	// suinb c,byte
		SUINB(_C); break;
	case 0x34:	// suinb d,byte
		SUINB(_D); break;
	case 0x35:	// suinb e,byte
		SUINB(_E); break;
	case 0x36:	// suinb h,byte
		SUINB(_H); break;
	case 0x37:	// suinb l,byte
		SUINB(_L); break;
	case 0x38:	// lti v,byte
		LTI(_V); break;
	case 0x39:	// lti a,byte
		LTI(_A); break;
	case 0x3a:	// lti b,byte
		LTI(_B); break;
	case 0x3b:	// lti c,byte
		LTI(_C); break;
	case 0x3c:	// lti d,byte
		LTI(_D); break;
	case 0x3d:	// lti e,byte
		LTI(_E); break;
	case 0x3e:	// lti h,byte
		LTI(_H); break;
	case 0x3f:	// lti l,byte
		LTI(_L); break;
	case 0x40:	// adi v,byte
		ADI(_V); break;
	case 0x41:	// adi a,byte
		ADI(_A); break;
	case 0x42:	// adi b,byte
		ADI(_B); break;
	case 0x43:	// adi c,byte
		ADI(_C); break;
	case 0x44:	// adi d,byte
		ADI(_D); break;
	case 0x45:	// adi e,byte
		ADI(_E); break;
	case 0x46:	// adi h,byte
		ADI(_H); break;
	case 0x47:	// adi l,byte
		ADI(_L); break;
	case 0x48:	// oni v,byte
		ONI(_V); break;
	case 0x49:	// oni a,byte
		ONI(_A); break;
	case 0x4a:	// oni b,byte
		ONI(_B); break;
	case 0x4b:	// oni c,byte
		ONI(_C); break;
	case 0x4c:	// oni d,byte
		ONI(_D); break;
	case 0x4d:	// oni e,byte
		ONI(_E); break;
	case 0x4e:	// oni h,byte
		ONI(_H); break;
	case 0x4f:	// oni l,byte
		ONI(_L); break;
	case 0x50:	// aci v,byte
		ACI(_V); break;
	case 0x51:	// aci a,byte
		ACI(_A); break;
	case 0x52:	// aci b,byte
		ACI(_B); break;
	case 0x53:	// aci c,byte
		ACI(_C); break;
	case 0x54:	// aci d,byte
		ACI(_D); break;
	case 0x55:	// aci e,byte
		ACI(_E); break;
	case 0x56:	// aci h,byte
		ACI(_H); break;
	case 0x57:	// aci l,byte
		ACI(_L); break;
	case 0x58:	// offi v,byte
		OFFI(_V); break;
	case 0x59:	// offi a,byte
		OFFI(_A); break;
	case 0x5a:	// offi b,byte
		OFFI(_B); break;
	case 0x5b:	// offi c,byte
		OFFI(_C); break;
	case 0x5c:	// offi d,byte
		OFFI(_D); break;
	case 0x5d:	// offi e,byte
		OFFI(_E); break;
	case 0x5e:	// offi h,byte
		OFFI(_H); break;
	case 0x5f:	// offi l,byte
		OFFI(_L); break;
	case 0x60:	// sui v,byte
		SUI(_V); break;
	case 0x61:	// sui a,byte
		SUI(_A); break;
	case 0x62:	// sui b,byte
		SUI(_B); break;
	case 0x63:	// sui c,byte
		SUI(_C); break;
	case 0x64:	// sui d,byte
		SUI(_D); break;
	case 0x65:	// sui e,byte
		SUI(_E); break;
	case 0x66:	// sui h,byte
		SUI(_H); break;
	case 0x67:	// sui l,byte
		SUI(_L); break;
	case 0x68:	// nei v,byte
		NEI(_V); break;
	case 0x69:	// nei a,byte
		NEI(_A); break;
	case 0x6a:	// nei b,byte
		NEI(_B); break;
	case 0x6b:	// nei c,byte
		NEI(_C); break;
	case 0x6c:	// nei d,byte
		NEI(_D); break;
	case 0x6d:	// nei e,byte
		NEI(_E); break;
	case 0x6e:	// nei h,byte
		NEI(_H); break;
	case 0x6f:	// nei l,byte
		NEI(_L); break;
	case 0x70:	// sbi v,byte
		SBI(_V); break;
	case 0x71:	// sbi a,byte
		SBI(_A); break;
	case 0x72:	// sbi b,byte
		SBI(_B); break;
	case 0x73:	// sbi c,byte
		SBI(_C); break;
	case 0x74:	// sbi d,byte
		SBI(_D); break;
	case 0x75:	// sbi e,byte
		SBI(_E); break;
	case 0x76:	// sbi h,byte
		SBI(_H); break;
	case 0x77:	// sbi l,byte
		SBI(_L); break;
	case 0x78:	// eqi v,byte
		EQI(_V); break;
	case 0x79:	// eqi a,byte
		EQI(_A); break;
	case 0x7a:	// eqi b,byte
		EQI(_B); break;
	case 0x7b:	// eqi c,byte
		EQI(_C); break;
	case 0x7c:	// eqi d,byte
		EQI(_D); break;
	case 0x7d:	// eqi e,byte
		EQI(_E); break;
	case 0x7e:	// eqi h,byte
		EQI(_H); break;
	case 0x7f:	// eqi l,byte
		EQI(_L); break;
	case 0x88:	// ani pa,byte
		ANI_IO(P_A); break;
	case 0x89:	// ani pb,byte
		ANI_IO(P_B); break;
	case 0x8a:	// ani pc,byte
		ANI_IO(P_C); break;
	case 0x8b:	// ani mk,byte
		ANI(MK); break;
	case 0x90:	// xri pa,byte
		XRI_IO(P_A); break;
	case 0x91:	// xri pb,byte
		XRI_IO(P_B); break;
	case 0x92:	// xri pc,byte
		XRI_IO(P_C); break;
	case 0x93:	// xri mk,byte
		XRI(MK); break;
	case 0x98:	// ori pa,byte
		ORI_IO(P_A); break;
	case 0x99:	// ori pb,byte
		ORI_IO(P_B); break;
	case 0x9a:	// ori pc,byte
		ORI_IO(P_C); break;
	case 0x9b:	// ori mk,byte
		ORI(MK); break;
	case 0xa0:	// adinc pa,byte
		ADINC_IO(P_A); break;
	case 0xa1:	// adinc pb,byte
		ADINC_IO(P_B); break;
	case 0xa2:	// adinc pc,byte
		ADINC_IO(P_C); break;
	case 0xa3:	// adinc mk,byte
		ADINC(MK); break;
	case 0xa8:	// gti pa,byte
		GTI_IO(P_A); break;
	case 0xa9:	// gti pb,byte
		GTI_IO(P_B); break;
	case 0xaa:	// gti pc,byte
		GTI_IO(P_C); break;
	case 0xab:	// gti mk,byte
		GTI(MK); break;
	case 0xb0:	// suinb pa,byte
		SUINB_IO(P_A); break;
	case 0xb1:	// suinb pb,byte
		SUINB_IO(P_B); break;
	case 0xb2:	// suinb pc,byte
		SUINB_IO(P_C); break;
	case 0xb3:	// suinb mk,byte
		SUINB(MK); break;
	case 0xb8:	// lti pa,byte
		LTI_IO(P_A); break;
	case 0xb9:	// lti pb,byte
		LTI_IO(P_B); break;
	case 0xba:	// lti pc,byte
		LTI_IO(P_C); break;
	case 0xbb:	// lti mk,byte
		LTI(MK); break;
	case 0xc0:	// adi pa,byte
		ADI_IO(P_A); break;
	case 0xc1:	// adi pb,byte
		ADI_IO(P_B); break;
	case 0xc2:	// adi pc,byte
		ADI_IO(P_C); break;
	case 0xc3:	// adi mk,byte
		ADI(MK); break;
	case 0xc8:	// oni pa,byte
		ONI_IO(P_A); break;
	case 0xc9:	// oni pb,byte
		ONI_IO(P_B); break;
	case 0xca:	// oni pc,byte
		ONI_IO(P_C); break;
	case 0xcb:	// oni mk,byte
		ONI(MK); break;
	case 0xd0:	// aci pa,byte
		ACI_IO(P_A); break;
	case 0xd1:	// aci pb,byte
		ACI_IO(P_B); break;
	case 0xd2:	// aci pc,byte
		ACI_IO(P_C); break;
	case 0xd3:	// aci mk,byte
		ACI(MK); break;
	case 0xd8:	// offi pa,byte
		OFFI_IO(P_A); break;
	case 0xd9:	// offi pb,byte
		OFFI_IO(P_B); break;
	case 0xda:	// offi pc,byte
		OFFI_IO(P_C); break;
	case 0xdb:	// offi mk,byte
		OFFI(MK); break;
	case 0xe0:	// sui pa,byte
		SUI_IO(P_A); break;
	case 0xe1:	// sui pb,byte
		SUI_IO(P_B); break;
	case 0xe2:	// sui pc,byte
		SUI_IO(P_C); break;
	case 0xe3:	// sui mk,byte
		SUI(MK); break;
	case 0xe8:	// nei pa,byte
		NEI_IO(P_A); break;
	case 0xe9:	// nei pb,byte
		NEI_IO(P_B); break;
	case 0xea:	// nei pc,byte
		NEI_IO(P_C); break;
	case 0xeb:	// nei mk,byte
		NEI(MK); break;
	case 0xf0:	// sbi pa,byte
		SBI_IO(P_A); break;
	case 0xf1:	// sbi pb,byte
		SBI_IO(P_B); break;
	case 0xf2:	// sbi pc,byte
		SBI_IO(P_C); break;
	case 0xf3:	// sbi mk,byte
		SBI(MK); break;
	case 0xf8:	// eqi pa,byte
		EQI_IO(P_A); break;
	case 0xf9:	// eqi pb,byte
		EQI_IO(P_B); break;
	case 0xfa:	// eqi pc,byte
		EQI_IO(P_C); break;
	case 0xfb:	// eqi mk,byte
		EQI(MK); break;
	default:
		emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 64 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP70()
{
	uint8 ope = FETCH8();
	period += op70[ope].clock;
	
	switch(ope) {
	case 0x0e:	// sspd word
		WM16(FETCH16(), SP); break;
	case 0x0f:	// lspd word
		SP = RM16(FETCH16()); break;
	case 0x1e:	// sbcd word
		WM16(FETCH16(), BC); break;
	case 0x1f:	// lbcd word
		BC = RM16(FETCH16()); break;
	case 0x2e:	// sded word
		WM16(FETCH16(), DE); break;
	case 0x2f:	// lded word
		DE = RM16(FETCH16()); break;
	case 0x3e:	// shld word
		WM16(FETCH16(), HL); break;
	case 0x3f:	// lhld word
		HL = RM16(FETCH16()); break;
	case 0x68:	// mov v,word
		_V = RM8(FETCH16()); 
		break;
	case 0x69:	// mov a,word
		_A = RM8(FETCH16()); break;
	case 0x6a:	// mov b,word
		_B = RM8(FETCH16()); break;
	case 0x6b:	// mov c,word
		_C = RM8(FETCH16()); break;
	case 0x6c:	// mov d,word
		_D = RM8(FETCH16()); break;
	case 0x6d:	// mov e,word
		_E = RM8(FETCH16()); break;
	case 0x6e:	// mov h,word
		_H = RM8(FETCH16()); break;
	case 0x6f:	// mov l,word
		_L = RM8(FETCH16()); break;
	case 0x78:	// mov word,v
		WM8(FETCH16(), _V); break;
	case 0x79:	// mov word,a
		WM8(FETCH16(), _A); break;
	case 0x7a:	// mov word,b
		WM8(FETCH16(), _B); break;
	case 0x7b:	// mov word,c
		WM8(FETCH16(), _C); break;
	case 0x7c:	// mov word,d
		WM8(FETCH16(), _D); break;
	case 0x7d:	// mov word,e
		WM8(FETCH16(), _E); break;
	case 0x7e:	// mov word,h
		WM8(FETCH16(), _H); break;
	case 0x7f:	// mov word,l
		WM8(FETCH16(), _L); break;
	case 0x89:	// anax b
		ANAX(BC); break;
	case 0x8a:	// anax d
		ANAX(DE); break;
	case 0x8b:	// anax h
		ANAX(HL); break;
	case 0x8c:	// anax d+
		ANAX(DE++); break;
	case 0x8d:	// anax h+
		ANAX(HL++); break;
	case 0x8e:	// anax d-
		ANAX(DE--); break;
	case 0x8f:	// anax h-
		ANAX(HL--); break;
	case 0x91:	// xrax b
		XRAX(BC); break;
	case 0x92:	// xrax d
		XRAX(DE); break;
	case 0x93:	// xrax h
		XRAX(HL); break;
	case 0x94:	// xrax d+
		XRAX(DE++); break;
	case 0x95:	// xrax h+
		XRAX(HL++); break;
	case 0x96:	// xrax d-
		XRAX(DE--); break;
	case 0x97:	// xrax h-
		XRAX(HL--); break;
	case 0x99:	// orax b
		ORAX(BC); break;
	case 0x9a:	// orax d
		ORAX(DE); break;
	case 0x9b:	// orax h
		ORAX(HL); break;
	case 0x9c:	// orax d+
		ORAX(DE++); break;
	case 0x9d:	// orax h+
		ORAX(HL++); break;
	case 0x9e:	// orax d-
		ORAX(DE--); break;
	case 0x9f:	// orax h-
		ORAX(HL--); break;
	case 0xa1:	// addncx b
		ADDNCX(BC); break;
	case 0xa2:	// addncx d
		ADDNCX(DE); break;
	case 0xa3:	// addncx h
		ADDNCX(HL); break;
	case 0xa4:	// addncx d+
		ADDNCX(DE++); break;
	case 0xa5:	// addncx h+
		ADDNCX(HL++); break;
	case 0xa6:	// addncx d-
		ADDNCX(DE--); break;
	case 0xa7:	// addncx h-
		ADDNCX(HL--); break;
	case 0xa9:	// gtax b
		GTAX(BC); break;
	case 0xaa:	// gtax d
		GTAX(DE); break;
	case 0xab:	// gtax h
		GTAX(HL); break;
	case 0xac:	// gtax d+
		GTAX(DE++); break;
	case 0xad:	// gtax h+
		GTAX(HL++); break;
	case 0xae:	// gtax d-
		GTAX(DE--); break;
	case 0xaf:	// gtax h-
		GTAX(HL--); break;
	case 0xb1:	// subnbx b
		SUBNBX(BC); break;
	case 0xb2:	// subnbx d
		SUBNBX(DE); break;
	case 0xb3:	// subnbx h
		SUBNBX(HL); break;
	case 0xb4:	// subnbx d+
		SUBNBX(DE++); break;
	case 0xb5:	// subnbx h+
		SUBNBX(HL++); break;
	case 0xb6:	// subnbx d-
		SUBNBX(DE--); break;
	case 0xb7:	// subnbx h-
		SUBNBX(HL--); break;
	case 0xb9:	// ltax b
		LTAX(BC); break;
	case 0xba:	// ltax d
		LTAX(DE); break;
	case 0xbb:	// ltax h
		LTAX(HL); break;
	case 0xbc:	// ltax d+
		LTAX(DE++); break;
	case 0xbd:	// ltax h+
		LTAX(HL++); break;
	case 0xbe:	// ltax d-
		LTAX(DE--); break;
	case 0xbf:	// ltax h-
		LTAX(HL--); break;
	case 0xc1:	// addx b
		ADDX(BC); break;
	case 0xc2:	// addx d
		ADDX(DE); break;
	case 0xc3:	// addx h
		ADDX(HL); break;
	case 0xc4:	// addx d+
		ADDX(DE++); break;
	case 0xc5:	// addx h+
		ADDX(HL++); break;
	case 0xc6:	// addx d-
		ADDX(DE--); break;
	case 0xc7:	// addx h-
		ADDX(HL--); break;
	case 0xc9:	// onax b
		ONAX(BC); break;
	case 0xca:	// onax d
		ONAX(DE); break;
	case 0xcb:	// onax h
		ONAX(HL); break;
	case 0xcc:	// onax d+
		ONAX(DE++); break;
	case 0xcd:	// onax h+
		ONAX(HL++); break;
	case 0xce:	// onax d-
		ONAX(DE--); break;
	case 0xcf:	// onax h-
		ONAX(HL--); break;
	case 0xd1:	// adcx b
		ADCX(BC); break;
	case 0xd2:	// adcx d
		ADCX(DE); break;
	case 0xd3:	// adcx h
		ADCX(HL); break;
	case 0xd4:	// adcx d+
		ADCX(DE++); break;
	case 0xd5:	// adcx h+
		ADCX(HL++); break;
	case 0xd6:	// adcx d-
		ADCX(DE--); break;
	case 0xd7:	// adcx h-
		ADCX(HL--); break;
	case 0xd9:	// offax b
		OFFAX(BC); break;
	case 0xda:	// offax d
		OFFAX(DE); break;
	case 0xdb:	// offax h
		OFFAX(HL); break;
	case 0xdc:	// offax d+
		OFFAX(DE++); break;
	case 0xdd:	// offax h+
		OFFAX(HL++); break;
	case 0xde:	// offax d-
		OFFAX(DE--); break;
	case 0xdf:	// offax h-
		OFFAX(HL--); break;
	case 0xe1:	// subx b
		SUBX(BC); break;
	case 0xe2:	// subx d
		SUBX(DE); break;
	case 0xe3:	// subx h
		SUBX(HL); break;
	case 0xe4:	// subx d+
		SUBX(DE++); break;
	case 0xe5:	// subx h+
		SUBX(HL++); break;
	case 0xe6:	// subx d-
		SUBX(DE--); break;
	case 0xe7:	// subx h-
		SUBX(HL--); break;
	case 0xe9:	// neax b
		NEAX(BC); break;
	case 0xea:	// neax d
		NEAX(DE); break;
	case 0xeb:	// neax h
		NEAX(HL); break;
	case 0xec:	// neax d+
		NEAX(DE++); break;
	case 0xed:	// neax h+
		NEAX(HL++); break;
	case 0xee:	// neax d-
		NEAX(DE--); break;
	case 0xef:	// neax h-
		NEAX(HL--); break;
	case 0xf1:	// sbbx b
		SBBX(BC); break;
	case 0xf2:	// sbbx d
		SBBX(DE); break;
	case 0xf3:	// sbbx h
		SBBX(HL); break;
	case 0xf4:	// sbbx d+
		SBBX(DE++); break;
	case 0xf5:	// sbbx h+
		SBBX(HL++); break;
	case 0xf6:	// sbbx d-
		SBBX(DE--); break;
	case 0xf7:	// sbbx h-
		SBBX(HL--); break;
	case 0xf9:	// eqax b
		EQAX(BC); break;
	case 0xfa:	// eqax d
		EQAX(DE); break;
	case 0xfb:	// eqax h
		EQAX(HL); break;
	case 0xfc:	// eqax d+
		EQAX(DE++); break;
	case 0xfd:	// eqax h+
		EQAX(HL++); break;
	case 0xfe:	// eqax d-
		EQAX(DE--); break;
	case 0xff:	// eqax h-
		EQAX(HL--); break;
	default:
		emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 70 %2x\n"), prevPC, ope);
	}
}

void UPD7801::OP74()
{
	uint8 ope = FETCH8();
	period += op74[ope].clock;
	
	switch(ope) {
	case 0x88:	// anaw wa
		ANAW(); break;
	case 0x90:	// xraw wa
		XRAW(); break;
	case 0x98:	// oraw wa
		ORAW(); break;
	case 0xa0:	// addncw wa
		ADDNCW(); break;
	case 0xa8:	// gtaw wa
		GTAW(); break;
	case 0xb0:	// subnbw wa
		SUBNBW(); break;
	case 0xb8:	// ltaw wa
		LTAW(); break;
	case 0xc0:	// addw wa
		ADDW(); break;
	case 0xc8:	// onaw wa
		ONAW(); break;
	case 0xd0:	// adcw wa
		ADCW(); break;
	case 0xd8:	// offaw wa
		OFFAW(); break;
	case 0xe0:	// subw wa
		SUBW(); break;
	case 0xe8:	// neaw wa
		NEAW(); break;
	case 0xf0:	// sbbw wa
		SBBW(); break;
	case 0xf8:	// eqaw wa
		EQAW(); break;
	default:
		emu->out_debug_log(_T("PC=%4x\tCPU\tUNKNOWN OP : 74 %2x\n"), prevPC, ope);
	}
}

