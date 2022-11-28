//#include	"compiler.h"
#include	"cpucore.h"
#include	"i286c.h"
#include	"i286c.mcr"


// ------------------------------------------------------------ opecode 0xfe,f

#if 0
I286_F6 _nop_int(UINT op) {

	INT_NUM(6, I286_IP - 2);
}
#endif

I286_F6 _inc_ea8(UINT op) {

	UINT32	madr;
	UINT8	*out;
	REG8	res;

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		out = REG8_B20(op);
	}
	else {
		I286_WORKCLOCK(7);
		madr = CALC_EA(op);
//		if (madr >= I286_MEMWRITEMAX) {
			res = i286_memoryread(madr);
			INCBYTE(res)
			i286_memorywrite(madr, res);
			return;
//		}
//		out = mem + madr;
	}
	res = *out;
	INCBYTE(res)
	*out = (UINT8)res;
}

I286_F6 _dec_ea8(UINT op) {

	UINT32	madr;
	UINT8	*out;
	REG8	res;

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		out = REG8_B20(op);
	}
	else {
		I286_WORKCLOCK(7);
		madr = CALC_EA(op);
//		if (madr >= I286_MEMWRITEMAX) {
			res = i286_memoryread(madr);
			DECBYTE(res)
			i286_memorywrite(madr, res);
			return;
//		}
//		out = mem + madr;
	}
	res = *out;
	DECBYTE(res)
	*out = (UINT8)res;
}

I286_F6 _inc_ea16(UINT op) {

	UINT32	madr;
	UINT16	*out;
	REG16	res;

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		out = REG16_B20(op);
	}
	else {
		I286_WORKCLOCK(7);
		madr = CALC_EA(op);
//		if (INHIBIT_WORDP(madr)) {
			res = i286_memoryread_w(madr);
			INCWORD(res)
			i286_memorywrite_w(madr, res);
			return;
//		}
//		out = (UINT16 *)(mem + madr);
	}
	res = *out;
	INCWORD(res)
	*out = (UINT16)res;
}

I286_F6 _dec_ea16(UINT op) {

	UINT32	madr;
	UINT16	*out;
	REG16	res;

	if (op >= 0xc0) {
		I286_WORKCLOCK(2);
		out = REG16_B20(op);
	}
	else {
		I286_WORKCLOCK(7);
		madr = CALC_EA(op);
//		if (INHIBIT_WORDP(madr)) {
			res = i286_memoryread_w(madr);
			DECWORD(res)
			i286_memorywrite_w(madr, res);
			return;
//		}
//		out = (UINT16 *)(mem + madr);
	}
	res = *out;
	DECWORD(res)
	*out = (UINT16)res;
}

I286_F6 _call_ea16(UINT op) {

	UINT16	src;

	if (op >= 0xc0) {
		I286_WORKCLOCK(7);
		src = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(11);
		src = i286_memoryread_w(CALC_EA(op));
	}
	REGPUSH0(I286_IP);
	I286_IP = src;
}

I286_F6 _call_far_ea16(UINT op) {

	UINT32	seg;
	UINT	ad;

	I286_WORKCLOCK(16);
	if (op < 0xc0) {
		ad = GET_EA(op, &seg);
		UINT16 newip = i286_memoryread_w(seg + ad);
		UINT16 newcs = i286_memoryread_w(seg + LOW16(ad + 2));

#ifdef I86_PSEUDO_BIOS
		if (device_bios != NULL) {
			uint16_t regs[8] = {CPU_AX, CPU_CX, CPU_DX, CPU_BX, CPU_SP, CPU_BP, CPU_SI, CPU_DI};
			uint16_t sregs[4] = {CPU_ES, CPU_CS, CPU_SS, CPU_DS};
			int32_t ZeroFlag = ((CPU_FLAG & Z_FLAG) ? 1 : 0);
			int32_t CarryFlag = ((CPU_FLAG & C_FLAG) ? 1 : 0);
			
			if (device_bios->bios_call_far_i86((newcs << 4) + newip, regs, sregs, &ZeroFlag, &CarryFlag)) {
				CPU_AX = regs[0];
				CPU_CX = regs[1];
				CPU_DX = regs[2];
				CPU_BX = regs[3];
				CPU_SP = regs[4];
				CPU_BP = regs[5];
				CPU_SI = regs[6];
				CPU_DI = regs[7];
				if (ZeroFlag) {
					CPU_FLAG |=  Z_FLAG;
				} else {
					CPU_FLAG &= ~Z_FLAG;
				}
				if (CarryFlag) {
					CPU_FLAG |=  C_FLAG;
				} else {
					CPU_FLAG &= ~C_FLAG;
				}
				I286_WORKCLOCK(1000); // temporary
				return;
			}
		}
#endif

		REGPUSH0(I286_CS)								// ToDo
		REGPUSH0(I286_IP)
//		I286_IP = i286_memoryread_w(seg + ad);
//		I286_CS = i286_memoryread_w(seg + LOW16(ad + 2));
		I286_IP = newip;
		I286_CS = newcs;
		CS_BASE = SEGSELECT(I286_CS);
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_F6 _jmp_ea16(UINT op) {

	if (op >= 0xc0) {
		I286_WORKCLOCK(7);
		I286_IP = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(11);
		I286_IP = i286_memoryread_w(CALC_EA(op));
	}
}

I286_F6 _jmp_far_ea16(UINT op) {

	UINT32	seg;
	UINT	ad;

	I286_WORKCLOCK(11);
	if (op < 0xc0) {
		ad = GET_EA(op, &seg);
		I286_IP = i286_memoryread_w(seg + ad);
		I286_CS = i286_memoryread_w(seg + LOW16(ad + 2));
		CS_BASE = SEGSELECT(I286_CS);
	}
	else {
		INT_NUM(6, I286_IP - 2);
	}
}

I286_F6 _push_ea16(UINT op) {

	UINT16	src;

	if (op >= 0xc0) {
		I286_WORKCLOCK(3);
		src = *(REG16_B20(op));
	}
	else {
		I286_WORKCLOCK(5);
		src = i286_memoryread_w(CALC_EA(op));
	}
	REGPUSH0(src);
}

I286_F6 _pop_ea16(UINT op) {

	UINT16	src;

	REGPOP0(src);
	I286_WORKCLOCK(5);
	if (op >= 0xc0) {
		*(REG16_B20(op)) = src;
	}
	else {
		i286_memorywrite_w(CALC_EA(op), src);
	}
}


const I286OPF6 c_ope0xfe_table[] = {
			_inc_ea8,			_dec_ea8};

const I286OPF6 c_ope0xff_table[] = {
			_inc_ea16,			_dec_ea16,
			_call_ea16,			_call_far_ea16,
			_jmp_ea16,			_jmp_far_ea16,
			_push_ea16,			_pop_ea16};

