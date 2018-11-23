
#include "../vm.h"
#include "../../emu.h"
#include "./i386_real.h"

#ifdef I386_PSEUDO_BIOS
#define BIOS_INT(num) if(cpustate->bios != NULL) { \
	uint16_t regs[8], sregs[4]; \
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
	sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
	sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
	int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF; \
	if(cpustate->bios->bios_int_i86(num, regs, sregs, &ZeroFlag, &CarryFlag)) { \
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
		cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
		CYCLES(CYCLES_IRET); \
		return; \
	} \
}

#define BIOS_CALL_FAR(address) if(cpustate->bios != NULL) {	\
	uint16_t regs[8], sregs[4]; \
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
	sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
	sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
	int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF; \
	if(cpustate->bios->bios_call_far_i86(address, regs, sregs, &ZeroFlag, &CarryFlag)) { \
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
		cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
		CYCLES(CYCLES_RET_INTERSEG);							\
		return;															\
	} \
}
#endif

void I386_OPS::i386_call_abs16()        // Opcode 0x9a
{
	UINT16 offset = FETCH16();
	UINT16 ptr = FETCH16();
    CYCLES(CYCLES_CALL_INTERSEG);      /* TODO: Timing = 17 + m */

#ifdef I386_PSEUDO_BIOS
	BIOS_CALL_FAR(((ptr << 4) + offset) & cpustate->a20_mask)
#endif
		//printf(" \n");

	if( PROTECTED_MODE && !V8086_MODE)
	{
		i386_protected_mode_call(ptr,offset,0,0);
	}
	else
	{
		PUSH16( cpustate->sreg[CS].selector );
		PUSH16( cpustate->eip );
		cpustate->sreg[CS].selector = ptr;
		cpustate->performed_intersegment_jump = 1;
		cpustate->eip = offset;
		i386_load_segment_descriptor(CS);
	}
	CHANGE_PC(cpustate->eip);
}

void I386_OPS::i386_call_rel16()        // Opcode 0xe8
{
	INT16 disp = FETCH16();

	PUSH16( cpustate->eip );
	if (cpustate->sreg[CS].d)
	{
		cpustate->eip += disp;
	}
	else
	{
		cpustate->eip = (cpustate->eip + disp) & 0xffff;
	}
	CHANGE_PC(cpustate->eip);
	CYCLES(CYCLES_CALL);       /* TODO: Timing = 7 + m */
}

void I386_OPS::i386_groupFF_16()        // Opcode 0xff
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = INC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1, 2);
				UINT16 dst = READ16(ea);
				dst = INC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm16 */
			if( modrm >= 0xc0 ) {
				UINT16 dst = LOAD_RM16(modrm);
				dst = DEC16(dst);
				STORE_RM16(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1, 2);
				UINT16 dst = READ16(ea);
				dst = DEC16(dst);
				WRITE16(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 2:         /* CALL Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_CALL_REG);       /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm,0, 2);
					address = READ16(ea);
					CYCLES(CYCLES_CALL_MEM);       /* TODO: Timing = 10 + m */
				}
				PUSH16( cpustate->eip );
				cpustate->eip = address;
				CHANGE_PC(cpustate->eip);
			}
			break;
		case 3:         /* CALL FAR Rm16 */
			{
				UINT16 address, selector;
				if( modrm >= 0xc0 )
				{
					report_invalid_modrm( "groupFF_16", modrm);
				}
				else
				{
					UINT32 ea = GetEA(modrm,0, 4);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_CALL_MEM_INTERSEG);      /* TODO: Timing = 10 + m */
#ifdef I386_PSEUDO_BIOS
					BIOS_CALL_FAR(((selector << 4) + address) & cpustate->a20_mask)
#endif
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_call(selector,address,1,0);
					}
					else
					{
						PUSH16( cpustate->sreg[CS].selector );
						PUSH16( cpustate->eip );
						cpustate->sreg[CS].selector = selector;
						cpustate->performed_intersegment_jump = 1;
						i386_load_segment_descriptor( CS );
						cpustate->eip = address;
						CHANGE_PC(cpustate->eip);
					}
				}
			}
			break;
		case 4:         /* JMP Rm16 */
			{
				UINT16 address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM16(modrm);
					CYCLES(CYCLES_JMP_REG);        /* TODO: Timing = 7 + m */
				} else {
					UINT32 ea = GetEA(modrm,0, 2);
					address = READ16(ea);
					CYCLES(CYCLES_JMP_MEM);        /* TODO: Timing = 10 + m */
				}
				cpustate->eip = address;
				CHANGE_PC(cpustate->eip);
			}
			break;
		case 5:         /* JMP FAR Rm16 */
			{
				UINT16 address, selector;

				if( modrm >= 0xc0 )
				{
					report_invalid_modrm( "groupFF_16", modrm);
				}
				else
				{
					UINT32 ea = GetEA(modrm,0, 4);
					address = READ16(ea + 0);
					selector = READ16(ea + 2);
					CYCLES(CYCLES_JMP_MEM_INTERSEG);       /* TODO: Timing = 10 + m */
					if(PROTECTED_MODE && !V8086_MODE)
					{
						i386_protected_mode_jump(selector,address,1,0);
					}
					else
					{
						cpustate->sreg[CS].selector = selector;
						cpustate->performed_intersegment_jump = 1;
						i386_load_segment_descriptor( CS );
						cpustate->eip = address;
						CHANGE_PC(cpustate->eip);
					}
				}
			}
			break;
		case 6:         /* PUSH Rm16 */
			{
				UINT16 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM16(modrm);
				} else {
					UINT32 ea = GetEA(modrm,0, 2);
					value = READ16(ea);
				}
				PUSH16(value);
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			report_invalid_modrm( "groupFF_16", modrm);
			break;
	}
}

void I386_OPS::i386__int()               // Opcode 0xcd
{
	int interrupt = FETCH();
	CYCLES(CYCLES_INT);

#ifdef I386_PSEUDO_BIOS
	BIOS_INT(interrupt)
#endif
	cpustate->ext = 0; // not an external interrupt
	i386_trap(interrupt, 1, 0);
	cpustate->ext = 1;
}
