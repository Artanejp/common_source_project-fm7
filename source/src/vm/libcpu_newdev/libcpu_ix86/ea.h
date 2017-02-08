#ifndef __NEWDEV_LIBX86_EA_H__
#define __NEWDEV_LIBX86_EA_H__
unsigned X86_OPS_BASE::EA_000() { cpustate->icount-=7; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[SI]); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_001() { cpustate->icount-=8; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[DI]); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_002() { cpustate->icount-=8; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[SI]); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_003() { cpustate->icount-=7; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[DI]); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_004() { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_005() { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_006() { cpustate->icount-=6; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_007() { cpustate->icount-=5; cpustate->eo=cpustate->regs.w[BX]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

unsigned X86_OPS_BASE::EA_100() { cpustate->icount-=11; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_101() { cpustate->icount-=12; cpustate->eo=(WORD)(cpustate->regs.w[BX]+cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_102() { cpustate->icount-=12; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_103() { cpustate->icount-=11; cpustate->eo=(WORD)(cpustate->regs.w[BP]+cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_104() { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[SI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_105() { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[DI]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_106() { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[BP]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_107() { cpustate->icount-=9; cpustate->eo=(WORD)(cpustate->regs.w[BX]+(INT8)FETCHOP); cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+cpustate->eo; return cpustate->ea; }

unsigned X86_OPS_BASE::EA_200() { cpustate->icount-=11; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]+cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_201() { cpustate->icount-=12; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]+cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_202() { cpustate->icount-=12; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]+cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_203() { cpustate->icount-=11; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]+cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_204() { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[SI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_205() { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[DI]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_206() { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BP]; cpustate->ea_seg=DefaultSeg(SS); cpustate->ea=DefaultBase(SS)+(WORD)cpustate->eo; return cpustate->ea; }
unsigned X86_OPS_BASE::EA_207() { cpustate->icount-=9; cpustate->eo=FETCHOP; cpustate->eo+=FETCHOP<<8; cpustate->eo+=cpustate->regs.w[BX]; cpustate->ea_seg=DefaultSeg(DS); cpustate->ea=DefaultBase(DS)+(WORD)cpustate->eo; return cpustate->ea; }

unsigned (X86_OPS_BASE::*GetEA[192])()={
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,
	&X86_OPS_BASE::EA_000, &X86_OPS_BASE::EA_001, &X86_OPS_BASE::EA_002, &X86_OPS_BASE::EA_003, &X86_OPS_BASE::EA_004, &X86_OPS_BASE::EA_005, &X86_OPS_BASE::EA_006, &X86_OPS_BASE::EA_007,

	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,
	&X86_OPS_BASE::EA_100, &X86_OPS_BASE::EA_101, &X86_OPS_BASE::EA_102, &X86_OPS_BASE::EA_103, &X86_OPS_BASE::EA_104, &X86_OPS_BASE::EA_105, &X86_OPS_BASE::EA_106, &X86_OPS_BASE::EA_107,

	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207,
	&X86_OPS_BASE::EA_200, &X86_OPS_BASE::EA_201, &X86_OPS_BASE::EA_202, &X86_OPS_BASE::EA_203, &X86_OPS_BASE::EA_204, &X86_OPS_BASE::EA_205, &X86_OPS_BASE::EA_206, &X86_OPS_BASE::EA_207
};
#endif