/*
 *  Copyright (C) 2002-2015  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#pragma once
namespace I386_DOSBOX {
	
typedef PhysPt (*EA_LookupHandler)(void);

/* The MOD/RM Decoder for EA for this decoder's addressing modes */
__OPCALL PhysPt EA_16_00_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_si); }
__OPCALL PhysPt EA_16_01_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_di); }
__OPCALL PhysPt EA_16_02_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_si); }
__OPCALL PhysPt EA_16_03_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_di); }
__OPCALL PhysPt EA_16_04_n(void) { return BaseDS+(Bit16u)(reg_si); }
__OPCALL PhysPt EA_16_05_n(void) { return BaseDS+(Bit16u)(reg_di); }
__OPCALL PhysPt EA_16_06_n(void) { return BaseDS+(Bit16u)(Fetchw());}
__OPCALL PhysPt EA_16_07_n(void) { return BaseDS+(Bit16u)(reg_bx); }

__OPCALL PhysPt EA_16_40_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_si+Fetchbs()); }
__OPCALL PhysPt EA_16_41_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_di+Fetchbs()); }
__OPCALL PhysPt EA_16_42_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_si+Fetchbs()); }
__OPCALL PhysPt EA_16_43_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_di+Fetchbs()); }
__OPCALL PhysPt EA_16_44_n(void) { return BaseDS+(Bit16u)(reg_si+Fetchbs()); }
__OPCALL PhysPt EA_16_45_n(void) { return BaseDS+(Bit16u)(reg_di+Fetchbs()); }
__OPCALL PhysPt EA_16_46_n(void) { return BaseSS+(Bit16u)(reg_bp+Fetchbs()); }
__OPCALL PhysPt EA_16_47_n(void) { return BaseDS+(Bit16u)(reg_bx+Fetchbs()); }

__OPCALL PhysPt EA_16_80_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_si+Fetchws()); }
__OPCALL PhysPt EA_16_81_n(void) { return BaseDS+(Bit16u)(reg_bx+(Bit16s)reg_di+Fetchws()); }
__OPCALL PhysPt EA_16_82_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_si+Fetchws()); }
__OPCALL PhysPt EA_16_83_n(void) { return BaseSS+(Bit16u)(reg_bp+(Bit16s)reg_di+Fetchws()); }
__OPCALL PhysPt EA_16_84_n(void) { return BaseDS+(Bit16u)(reg_si+Fetchws()); }
__OPCALL PhysPt EA_16_85_n(void) { return BaseDS+(Bit16u)(reg_di+Fetchws()); }
__OPCALL PhysPt EA_16_86_n(void) { return BaseSS+(Bit16u)(reg_bp+Fetchws()); }
__OPCALL PhysPt EA_16_87_n(void) { return BaseDS+(Bit16u)(reg_bx+Fetchws()); }

__OPCALL Bit32u SIBZero=0;
__OPCALL Bit32u * SIBIndex[8]= { &reg_eax,&reg_ecx,&reg_edx,&reg_ebx,&SIBZero,&reg_ebp,&reg_esi,&reg_edi };

__OPCALL_INLINE PhysPt Sib(Bitu mode) {
	Bit8u sib=Fetchb();
	PhysPt base;
	switch (sib&7) {
	case 0:	/* EAX Base */
		base=BaseDS+reg_eax;break;
	case 1:	/* ECX Base */
		base=BaseDS+reg_ecx;break;
	case 2:	/* EDX Base */
		base=BaseDS+reg_edx;break;
	case 3:	/* EBX Base */
		base=BaseDS+reg_ebx;break;
	case 4:	/* ESP Base */
		base=BaseSS+reg_esp;break;
	case 5:	/* #1 Base */
		if (!mode) {
			base=BaseDS+Fetchd();break;
		} else {
			base=BaseSS+reg_ebp;break;
		}
	case 6:	/* ESI Base */
		base=BaseDS+reg_esi;break;
	case 7:	/* EDI Base */
		base=BaseDS+reg_edi;break;
	}
	base+=*SIBIndex[(sib >> 3) &7] << (sib >> 6);
	return base;
}

__OPCALL PhysPt EA_32_00_n(void) { return BaseDS+reg_eax; }
__OPCALL PhysPt EA_32_01_n(void) { return BaseDS+reg_ecx; }
__OPCALL PhysPt EA_32_02_n(void) { return BaseDS+reg_edx; }
__OPCALL PhysPt EA_32_03_n(void) { return BaseDS+reg_ebx; }
__OPCALL PhysPt EA_32_04_n(void) { return Sib(0);}
__OPCALL PhysPt EA_32_05_n(void) { return BaseDS+Fetchd(); }
__OPCALL PhysPt EA_32_06_n(void) { return BaseDS+reg_esi; }
__OPCALL PhysPt EA_32_07_n(void) { return BaseDS+reg_edi; }

__OPCALL PhysPt EA_32_40_n(void) { return BaseDS+reg_eax+Fetchbs(); }
__OPCALL PhysPt EA_32_41_n(void) { return BaseDS+reg_ecx+Fetchbs(); }
__OPCALL PhysPt EA_32_42_n(void) { return BaseDS+reg_edx+Fetchbs(); }
__OPCALL PhysPt EA_32_43_n(void) { return BaseDS+reg_ebx+Fetchbs(); }
__OPCALL PhysPt EA_32_44_n(void) { PhysPt temp=Sib(1);return temp+Fetchbs();}
//__OPCALL PhysPt EA_32_44_n(void) { return Sib(1)+Fetchbs();}
__OPCALL PhysPt EA_32_45_n(void) { return BaseSS+reg_ebp+Fetchbs(); }
__OPCALL PhysPt EA_32_46_n(void) { return BaseDS+reg_esi+Fetchbs(); }
__OPCALL PhysPt EA_32_47_n(void) { return BaseDS+reg_edi+Fetchbs(); }

__OPCALL PhysPt EA_32_80_n(void) { return BaseDS+reg_eax+Fetchds(); }
__OPCALL PhysPt EA_32_81_n(void) { return BaseDS+reg_ecx+Fetchds(); }
__OPCALL PhysPt EA_32_82_n(void) { return BaseDS+reg_edx+Fetchds(); }
__OPCALL PhysPt EA_32_83_n(void) { return BaseDS+reg_ebx+Fetchds(); }
__OPCALL PhysPt EA_32_84_n(void) { PhysPt temp=Sib(2);return temp+Fetchds();}
//__OPCALL PhysPt EA_32_84_n(void) { return Sib(2)+Fetchds();}
__OPCALL PhysPt EA_32_85_n(void) { return BaseSS+reg_ebp+Fetchds(); }
__OPCALL PhysPt EA_32_86_n(void) { return BaseDS+reg_esi+Fetchds(); }
__OPCALL PhysPt EA_32_87_n(void) { return BaseDS+reg_edi+Fetchds(); }

__OPCALL GetEAHandler EATable[512]={
/* 00 */
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
	EA_16_00_n,EA_16_01_n,EA_16_02_n,EA_16_03_n,EA_16_04_n,EA_16_05_n,EA_16_06_n,EA_16_07_n,
/* 01 */
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
	EA_16_40_n,EA_16_41_n,EA_16_42_n,EA_16_43_n,EA_16_44_n,EA_16_45_n,EA_16_46_n,EA_16_47_n,
/* 10 */
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
	EA_16_80_n,EA_16_81_n,EA_16_82_n,EA_16_83_n,EA_16_84_n,EA_16_85_n,EA_16_86_n,EA_16_87_n,
/* 11 These are illegal so make em 0 */
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
/* 00 */
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
	EA_32_00_n,EA_32_01_n,EA_32_02_n,EA_32_03_n,EA_32_04_n,EA_32_05_n,EA_32_06_n,EA_32_07_n,
/* 01 */
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
	EA_32_40_n,EA_32_41_n,EA_32_42_n,EA_32_43_n,EA_32_44_n,EA_32_45_n,EA_32_46_n,EA_32_47_n,
/* 10 */
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
	EA_32_80_n,EA_32_81_n,EA_32_82_n,EA_32_83_n,EA_32_84_n,EA_32_85_n,EA_32_86_n,EA_32_87_n,
/* 11 These are illegal so make em 0 */
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0
};

#define GetEADirect							\
	PhysPt eaa;								\
	if (TEST_PREFIX_ADDR) {					\
		eaa=BaseDS+Fetchd();				\
	} else {								\
		eaa=BaseDS+Fetchw();				\
	}										\

};
