
#ifndef __I8080_REGDEF_H_
#define __I8080_REGDEF_H_

#define AF	regs[0].w.l
#define BC	regs[1].w.l
#define DE	regs[2].w.l
#define HL	regs[3].w.l

#define _F	regs[0].b.l
#define _A	regs[0].b.h
#define _C	regs[1].b.l
#define _B	regs[1].b.h
#define _E	regs[2].b.l
#define _D	regs[2].b.h
#define _L	regs[3].b.l
#define _H	regs[3].b.h

#define CF	0x01
#define NF	0x02
#define VF	0x04
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#define IM_M5	0x01
#define IM_M6	0x02
#define IM_M7	0x04
#define IM_IEN	0x08
#define IM_I5	0x10
#define IM_I6	0x20
#define IM_I7	0x40
#define IM_SID	0x80
// special
#define IM_INT	0x100
#define IM_NMI	0x200
//#define IM_REQ	(IM_I5 | IM_I6 | IM_I7 | IM_INT | IM_NMI)
#define IM_REQ	0x370

#endif
