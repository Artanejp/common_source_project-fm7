#include "./i386_opdef.h"

int I386_OPS_BASE::i386_translate_address(int intention, offs_t *address, vtlb_entry *entry)
{
	UINT32 a = *address;
	UINT32 pdbr = cpustate->cr[3] & 0xfffff000;
	UINT32 directory = (a >> 22) & 0x3ff;
	UINT32 table = (a >> 12) & 0x3ff;
	vtlb_entry perm = 0;
	int ret = FALSE;
	bool user = (intention & TRANSLATE_USER_MASK) ? true : false;
	bool write = (intention & TRANSLATE_WRITE) ? true : false;
	bool debug = (intention & TRANSLATE_DEBUG_MASK) ? true : false;

	if(!(cpustate->cr[0] & 0x80000000))
	{
		if(entry)
			*entry = 0x77;
		return TRUE;
	}

	UINT32 page_dir = cpustate->program->read_data32(pdbr + directory * 4);
	if(page_dir & 1)
	{
		if ((page_dir & 0x80) && (cpustate->cr[4] & 0x10))
		{
			a = (page_dir & 0xffc00000) | (a & 0x003fffff);
			if(debug)
			{
				*address = a;
				return TRUE;
			}
			perm = get_permissions(page_dir, WP);
			if(write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
				ret = FALSE;
			else if(user && !(perm & VTLB_USER_READ_ALLOWED))
				ret = FALSE;
			else
			{
				if(write)
					perm |= VTLB_FLAG_DIRTY;
				if(!(page_dir & 0x40) && write)
					cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x60);
				else if(!(page_dir & 0x20))
					cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x20);
				ret = TRUE;
			}
		}
		else
		{
			UINT32 page_entry = cpustate->program->read_data32((page_dir & 0xfffff000) + (table * 4));
			if(!(page_entry & 1))
				ret = FALSE;
			else
			{
				a = (page_entry & 0xfffff000) | (a & 0xfff);
				if(debug)
				{
					*address = a;
					return TRUE;
				}
				perm = get_permissions(page_entry, WP);
				if(write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
					ret = FALSE;
				else if(user && !(perm & VTLB_USER_READ_ALLOWED))
					ret = FALSE;
				else
				{
					if(write)
						perm |= VTLB_FLAG_DIRTY;
					if(!(page_dir & 0x20))
						cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x20);
					if(!(page_entry & 0x40) && write)
						cpustate->program->write_data32((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
					else if(!(page_entry & 0x20))
						cpustate->program->write_data32((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
					ret = TRUE;
				}
			}
		}
	}
	else
		ret = FALSE;
	if(entry)
		*entry = perm;
	if(ret)
		*address = a;
	return ret;
}

/***********************************************************************************
    MSR ACCESS
***********************************************************************************/

// Pentium MSR handling
UINT64 I386_OPS_BASE::pentium_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
	// Time Stamp Counter
	case 0x10:
		*valid_msr = 1;
		popmessage("RDMSR: Reading TSC");
		return cpustate->tsc;
	// Event Counters (TODO)
	case 0x11:  // CESR
		*valid_msr = 1;
		popmessage("RDMSR: Reading CESR");
		return 0;
	case 0x12:  // CTR0
		*valid_msr = 1;
		return cpustate->perfctr[0];
	case 0x13:  // CTR1
		*valid_msr = 1;
		return cpustate->perfctr[1];
	default:
		if(!(offset & ~0xf)) // 2-f are test registers
		{
			*valid_msr = 1;
			logerror("RDMSR: Reading test MSR %x", offset);
			return 0;
		}
		logerror("RDMSR: invalid P5 MSR read %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 0;
		return 0;
	}
	return -1;
}

void I386_OPS_BASE::pentium_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		popmessage("WRMSR: Writing P5_MC_ADDR");
		*valid_msr = 1;
		break;
	case 0x01:
		popmessage("WRMSR: Writing P5_MC_TYPE");
		*valid_msr = 1;
		break;
	// Time Stamp Counter
	case 0x10:
		cpustate->tsc = data;
		popmessage("WRMSR: Writing to TSC");
		*valid_msr = 1;
		break;
	// Event Counters (TODO)
	case 0x11:  // CESR
		popmessage("WRMSR: Writing to CESR");
		*valid_msr = 1;
		break;
	case 0x12:  // CTR0
		cpustate->perfctr[0] = data;
		*valid_msr = 1;
		break;
	case 0x13:  // CTR1
		cpustate->perfctr[1] = data;
		*valid_msr = 1;
		break;
	default:
		if(!(offset & ~0xf)) // 2-f are test registers
		{
			*valid_msr = 1;
			logerror("WRMSR: Writing test MSR %x", offset);
			break;
		}
		logerror("WRMSR: invalid MSR write %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 0;
		break;
	}
}

// P6 (Pentium Pro, Pentium II, Pentium III) MSR handling
UINT64 I386_OPS_BASE::p6_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
	// Time Stamp Counter
	case 0x10:
		*valid_msr = 1;
		popmessage("RDMSR: Reading TSC");
		return cpustate->tsc;
	// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		*valid_msr = 1;
		return cpustate->perfctr[0];
	case 0xc2:  // PerfCtr1
		*valid_msr = 1;
		return cpustate->perfctr[1];
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 1;
		return 0;
	}
	return -1;
}

void I386_OPS_BASE::p6_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	// Time Stamp Counter
	case 0x10:
		cpustate->tsc = data;
		popmessage("WRMSR: Writing to TSC");
		*valid_msr = 1;
		break;
	// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		cpustate->perfctr[0] = data;
		*valid_msr = 1;
		break;
	case 0xc2:  // PerfCtr1
		cpustate->perfctr[1] = data;
		*valid_msr = 1;
		break;
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 1;
		break;
	}
}

// PIV (Pentium 4+)
UINT64 I386_OPS_BASE::piv_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 1;
		return 0;
	}
	return -1;
}

void I386_OPS_BASE::piv_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 1;
		break;
	}
}

