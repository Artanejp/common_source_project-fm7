//#include	"compiler.h"
#include	"cpucore.h"
#include	"i286c.h"
#include	"i286c.mcr"


I286_0F _sldt(UINT op) {

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		*(REG16_B20(op)) = I286_LDTR;
	}
	else {
		I286_WORKCLOCK(3);
		i286_memorywrite_w(CALC_EA(op), I286_LDTR);
	}
}

I286_0F _str(UINT op) {

	if (op >= 0xc0) {
		I286_WORKCLOCK(3);
		*(REG16_B20(op)) = I286_TR;
	}
	else {
		I286_WORKCLOCK(6);
		i286_memorywrite_w(CALC_EA(op), I286_TR);
	}
}

I286_0F _lldt(UINT op) {

	REG16	r;
	UINT32	addr;

	if (op >= 0xc0) {
		I286_WORKCLOCK(17);
		r = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(19);
		r = i286_memoryread_w(CALC_EA(op));
	}
	addr = i286c_selector(r);
	I286_LDTR = r;
	I286_LDTRC.limit = i286_memoryread_w(addr);
	I286_LDTRC.base = i286_memoryread_w(addr + 2);
	I286_LDTRC.base24 = i286_memoryread(addr + 4);
}

I286_0F _ltr(UINT op) {

	REG16	r;
	UINT32	addr;

	if (op >= 0xc0) {
		I286_WORKCLOCK(17);
		r = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(19);
		r = i286_memoryread_w(CALC_EA(op));
	}
	addr = i286c_selector(r);
	I286_TR = r;
	I286_TRC.limit = i286_memoryread_w(addr);
	I286_TRC.base = i286_memoryread_w(addr + 2);
	I286_TRC.base24 = i286_memoryread(addr + 4);
}

I286_0F _verr(UINT op) {

	REG16	r;

	if (op >= 0xc0) {
		I286_WORKCLOCK(14);
		r = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(16);
		r = i286_memoryread_w(CALC_EA(op));
	}
}

I286_0F _verw(UINT op) {

	REG16	r;

	if (op >= 0xc0) {
		I286_WORKCLOCK(14);
		r = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(16);
		r = i286_memoryread_w(CALC_EA(op));
	}
}

I286_0F __sgdt(UINT op) {

	UINT32	addr;

	I286_WORKCLOCK(11);
	if (op < 0xc0) {
		addr = CALC_EA(op);
		i286_memorywrite_w(addr, I286_GDTR.limit);
		i286_memorywrite_w(addr + 2, I286_GDTR.base);
		i286_memorywrite_w(addr + 4, (REG16)(0xff00 + I286_GDTR.base24));
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_0F _sidt(UINT op) {

	UINT32	addr;

	I286_WORKCLOCK(12);
	if (op < 0xc0) {
		addr = CALC_EA(op);
		i286_memorywrite_w(addr, I286_IDTR.limit);
		i286_memorywrite_w(addr + 2, I286_IDTR.base);
		i286_memorywrite_w(addr + 4, (REG16)(0xff00 + I286_IDTR.base24));
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_0F __lgdt(UINT op) {

	UINT32	addr;

	I286_WORKCLOCK(11);
	if (op < 0xc0) {
		addr = CALC_EA(op);
		I286_GDTR.limit = i286_memoryread_w(addr);
		I286_GDTR.base = i286_memoryread_w(addr + 2);
		I286_GDTR.base24 = i286_memoryread(addr + 4);
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_0F _lidt(UINT op) {

	UINT32	addr;

	I286_WORKCLOCK(12);
	if (op < 0xc0) {
		addr = CALC_EA(op);
		I286_IDTR.limit = i286_memoryread_w(addr);
		I286_IDTR.base = i286_memoryread_w(addr + 2);
		I286_IDTR.base24 = i286_memoryread(addr + 4);
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_0F _smsw(UINT op) {

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		*(REG16_B20(op)) = I286_MSW;
	}
	else {
		I286_WORKCLOCK(3);
		i286_memorywrite_w(CALC_EA(op), I286_MSW);
	}
}

I286_0F _lmsw(UINT op) {

	REG16	msw;

	if (op >= 0xc0) {
		I286_WORKCLOCK(3);
		msw = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(6);
		msw = i286_memoryread_w(CALC_EA(op));
	}
	I286_MSW = msw | (I286_MSW & MSW_PE);
	if (msw & MSW_PE) {
		TRACEOUT(("80286 ProtectMode Enable... / MSW=%.4x [%.4x:%.4x]",
												I286_MSW, I286_CS, I286_IP));
	}
}

static const I286OP_0F cts0_table[] = {
			_sldt,	_str,	_lldt,	_ltr,
			_verr,	_verw,	_verr,	_verw};

static const I286OP_0F cts1_table[] = {
			__sgdt,	_sidt,	__lgdt,	_lidt,
			_smsw,	_smsw,	_lmsw,	_lmsw};


I286_0F _loadall286(void) {

	UINT16	tmp;
	UINT32	base;

	I286_WORKCLOCK(195);
	I286_MSW = i286_memoryread_w(0x804);//LOADINTELWORD(mem + 0x804);
	I286_TR = i286_memoryread_w(0x816);//LOADINTELWORD(mem + 0x816);			// ver0.73
	tmp = i286_memoryread_w(0x818);//LOADINTELWORD(mem + 0x818);
	I286_OV = tmp & O_FLAG;
	I286_FLAG = tmp & (0xfff ^ O_FLAG);
	I286_TRAP = ((tmp & 0x300) == 0x300);
	I286_IP = i286_memoryread_w(0x81a);//LOADINTELWORD(mem + 0x81a);
	I286_LDTR = i286_memoryread_w(0x81c);//LOADINTELWORD(mem + 0x81c);			// ver0.73
	I286_DS = i286_memoryread_w(0x81e);//LOADINTELWORD(mem + 0x81e);
	I286_SS = i286_memoryread_w(0x820);//LOADINTELWORD(mem + 0x820);
	I286_CS = i286_memoryread_w(0x822);//LOADINTELWORD(mem + 0x822);
	I286_ES = i286_memoryread_w(0x824);//LOADINTELWORD(mem + 0x824);
	I286_DI = i286_memoryread_w(0x826);//LOADINTELWORD(mem + 0x826);
	I286_SI = i286_memoryread_w(0x828);//LOADINTELWORD(mem + 0x828);
	I286_BP = i286_memoryread_w(0x82a);//LOADINTELWORD(mem + 0x82a);
	I286_SP = i286_memoryread_w(0x82c);//LOADINTELWORD(mem + 0x82c);
	I286_BX = i286_memoryread_w(0x82e);//LOADINTELWORD(mem + 0x82e);
	I286_DX = i286_memoryread_w(0x830);//LOADINTELWORD(mem + 0x830);
	I286_CX = i286_memoryread_w(0x832);//LOADINTELWORD(mem + 0x832);
	I286_AX = i286_memoryread_w(0x834);//LOADINTELWORD(mem + 0x834);
	base = i286_memoryread_d(0x836) & 0x00ffffff;//LOADINTELDWORD(mem + 0x836) & 0x00ffffff;
	ES_BASE = base;
	base = i286_memoryread_d(0x83c) & 0x00ffffff;//LOADINTELDWORD(mem + 0x83c) & 0x00ffffff;
	CS_BASE = base;
	base = i286_memoryread_d(0x842) & 0x00ffffff;//LOADINTELDWORD(mem + 0x842) & 0x00ffffff;
	SS_BASE = base;
	SS_FIX = base;
	base = i286_memoryread_d(0x848) & 0x00ffffff;//LOADINTELDWORD(mem + 0x848) & 0x00ffffff;
	DS_BASE = base;
	DS_FIX = base;

	I286_GDTR.base = i286_memoryread_w(0x84e);//LOADINTELWORD(mem + 0x84e);
	*(UINT16 *)(&I286_GDTR.base24) = i286_memoryread_w(0x850);//LOADINTELWORD(mem + 0x850);
	I286_GDTR.limit = i286_memoryread_w(0x852);//LOADINTELWORD(mem + 0x852);

	I286_LDTRC.base = i286_memoryread_w(0x854);//LOADINTELWORD(mem + 0x854);
	*(UINT16 *)(&I286_LDTRC.base24) = i286_memoryread_w(0x856);//LOADINTELWORD(mem + 0x856);
	I286_LDTRC.limit = i286_memoryread_w(0x858);//LOADINTELWORD(mem + 0x858);

	I286_IDTR.base = i286_memoryread_w(0x85a);//LOADINTELWORD(mem + 0x85a);
	*(UINT16 *)(&I286_IDTR.base24) = i286_memoryread_w(0x85c);//LOADINTELWORD(mem + 0x85c);
	I286_IDTR.limit = i286_memoryread_w(0x85e);//LOADINTELWORD(mem + 0x85e);

	I286_TRC.base = i286_memoryread_w(0x860);//LOADINTELWORD(mem + 0x860);
	*(UINT16 *)(&I286_TRC.base24) = i286_memoryread_w(0x8620);//LOADINTELWORD(mem + 0x8620);
	I286_TRC.limit = i286_memoryread_w(0x864);//LOADINTELWORD(mem + 0x864);

	I286IRQCHECKTERM
}

I286EXT i286c_cts(void) {

	UINT16	ip;
	UINT	op;
	UINT	op2;

	ip = I286_IP;
	GET_PCBYTE(op);

	if (op == 0) {
		if (!(I286_MSW & MSW_PE)) {
			INT_NUM(6, ip - 1);
		}
		else {
			GET_PCBYTE(op2);
			cts0_table[(op2 >> 3) & 7](op2);
		}
	}
	else if (op == 1) {
		GET_PCBYTE(op2);
		cts1_table[(op2 >> 3) & 7](op2);
	}
	else if (op == 5) {
		_loadall286();
	}
	else {
		INT_NUM(6, ip - 1);
	}
}

