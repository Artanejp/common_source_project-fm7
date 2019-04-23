// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
/*
    Intel 386 emulator

    Written by Ville Linde

    Currently supports:
        Intel 386
        Intel 486
        Intel Pentium
        Cyrix MediaGX
        Intel Pentium MMX
        Intel Pentium Pro
        Intel Pentium II
        Intel Pentium III
        Amd Athlon XP
        Intel Pentium 4
*/

#include "emu.h"
#include "vm_template.h"
#include "./i386_device.h"
#include "./i386priv.h"
#include "./x87priv.h"
#include "./cycles.h"
#include "./i386ops.h"

#include "debugger.h"
#include "debug/debugcpu.h"

/* seems to be defined on mingw-gcc */
#undef i386

/*
DEFINE_DEVICE_TYPE(I386,        i386_device,        "i386",        "Intel I386")
DEFINE_DEVICE_TYPE(I386SX,      i386sx_device,      "i386sx",      "Intel I386SX")
DEFINE_DEVICE_TYPE(I486,        i486_device,        "i486",        "Intel I486")
DEFINE_DEVICE_TYPE(I486DX4,     i486dx4_device,     "i486dx4",     "Intel I486DX4")
DEFINE_DEVICE_TYPE(PENTIUM,     pentium_device,     "pentium",     "Intel Pentium")
DEFINE_DEVICE_TYPE(PENTIUM_MMX, pentium_mmx_device, "pentium_mmx", "Intel Pentium MMX")
DEFINE_DEVICE_TYPE(MEDIAGX,     mediagx_device,     "mediagx",     "Cyrix MediaGX")
DEFINE_DEVICE_TYPE(PENTIUM_PRO, pentium_pro_device, "pentium_pro", "Intel Pentium Pro")
DEFINE_DEVICE_TYPE(PENTIUM2,    pentium2_device,    "pentium2",    "Intel Pentium II")
DEFINE_DEVICE_TYPE(PENTIUM3,    pentium3_device,    "pentium3",    "Intel Pentium III")
DEFINE_DEVICE_TYPE(ATHLONXP,    athlonxp_device,    "athlonxp",    "Amd Athlon XP")
DEFINE_DEVICE_TYPE(PENTIUM4,    pentium4_device,    "pentium4",    "Intel Pentium 4")
*/

i386_device::i386_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: DEVICE(parent_vm, parent_emu)
{
	// 32 unified
	d_vtlb = new device_vtlb_interface(parent_vm, parent_emu, this, AS_PROGRAM);
	m_smiact = this;
	m_ferr_handler = this;
	m_smiact_enabled = false;

	initialize_output_signals(&outputs_reset);
	d_debugger = NULL;
	d_dma = NULL;
	d_mem = NULL;
	d_io = NULL;
	d_bios = NULL;
	d_pic = NULL;
	d_program_stored = NULL;
	d_io_stored = NULL;
	
	d_vtlb->set_vtlb_dynamic_entries(32);
	d_vtlb->set_vtlb_page_shift(12);
	d_vtlb->set_vtlb_addr_width(32);

	m_ferr_err_value = 0;
	data_width = 32;
	set_device_name(_T("i386 DX CPU"));

	
}

i386_device::~i386_device()
{
	if(d_vtlb != NULL) delete d_vtlb;
}

i386sx_device::i386sx_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: i386_device(parent_vm, parent_emu)
{
	data_width = 16;
	set_device_name(_T("i386 SX CPU"));
}

i486_device::i486_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: i386_device(parent_vm, parent_emu)
{
	set_device_name(_T("Intel i486"));
}

i486dx4_device::i486dx4_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: i486_device(parent_vm, parent_emu)
{
	set_device_name(_T("Intel i486 DX4"));
}

pentium_device::pentium_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: i386_device(parent_vm, parent_emu)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb
	d_vtlb->set_vtlb_dynamic_entries(96);
	set_device_name(_T("Intel Pentium(i586)"));
}

mediagx_device::mediagx_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: i386_device(parent_vm, parent_emu)
{
	set_device_name(_T("Cyrix MediaGX"));
}


pentium_pro_device::pentium_pro_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_device(parent_vm, parent_emu)
{
	set_device_name(_T("Intel Pentium Pro(i686)"));
}

pentium_mmx_device::pentium_mmx_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_device(parent_vm, parent_emu)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	d_vtlb->set_vtlb_dynamic_entries(96);
	set_device_name(_T("Intel Pentium MMX"));
}

pentium2_device::pentium2_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_pro_device(parent_vm, parent_emu)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	d_vtlb->set_vtlb_dynamic_entries(96);
	set_device_name(_T("Intel Pentium2"));
}

pentium3_device::pentium3_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_pro_device(parent_vm, parent_emu)
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	d_vtlb->set_vtlb_dynamic_entries(96);
	set_device_name(_T("Intel Pentium3"));
}

// ToDo: AthlonXP
#if 0
athlonxp_device::athlonxp_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_device(parent_vm, parent_emu)
{
	// TODO: put correct value
	d_vtlb->set_vtlb_dynamic_entries(256);
	set_device_name(_T("AMD AthlonXP"));
}
#endif

pentium4_device::pentium4_device(VM_TEMPLATE* parent_vm, EMU* parent_emu)
	: pentium_device(parent_vm, parent_emu)
{
	// 128 dtlb, 64 itlb
	d_vtlb->set_vtlb_dynamic_entries(196);
	set_device_name(_T("Intel Pentium4"));
}

device_memory_interface::space_config_vector i386_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

int i386_parity_table[256];
MODRM_TABLE i386_MODRM_table[256];

/*************************************************************************/

uint32_t i386_device::i386_translate(int segment, uint32_t ip, int rwn)
{
	// TODO: segment limit access size, execution permission, handle exception thrown from exception handler
	if (PROTECTED_MODE && !V8086_MODE && (rwn != -1))
	{
		if (!(m_sreg[segment].valid))
			FAULT_THROW((segment == SS) ? FAULT_SS : FAULT_GP, 0);
		if (i386_limit_check(segment, ip))
			FAULT_THROW((segment == SS) ? FAULT_SS : FAULT_GP, 0);
		if ((rwn == 0) && ((m_sreg[segment].flags & 8) && !(m_sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
		if ((rwn == 1) && ((m_sreg[segment].flags & 8) || !(m_sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
	}
	return m_sreg[segment].base + ip;
}

vtlb_entry i386_device::get_permissions(uint32_t pte, int wp)
{
	vtlb_entry ret = VTLB_READ_ALLOWED | ((pte & 4) ? VTLB_USER_READ_ALLOWED : 0);
	if (!wp)
		ret |= VTLB_WRITE_ALLOWED;
	if (pte & 2)
		ret |= VTLB_WRITE_ALLOWED | ((pte & 4) ? VTLB_USER_WRITE_ALLOWED : 0);
	return ret;
}

bool i386_device::i386_translate_address(int intention, offs_t *address, vtlb_entry *entry)
{
	uint32_t a = *address;
	uint32_t pdbr = m_cr[3] & 0xfffff000;
	uint32_t directory = (a >> 22) & 0x3ff;
	uint32_t table = (a >> 12) & 0x3ff;
	vtlb_entry perm = 0;
	bool ret;
	bool user = (intention & TRANSLATE_USER_MASK) ? true : false;
	bool write = (intention & TRANSLATE_WRITE) ? true : false;
	bool debug = (intention & TRANSLATE_DEBUG_MASK) ? true : false;

	if (!(m_cr[0] & 0x80000000))
	{
		if (entry)
			*entry = 0x77;
		return true;
	}

	uint32_t page_dir = m_program->read_dword(pdbr + directory * 4);
	if (page_dir & 1)
	{
		if ((page_dir & 0x80) && (m_cr[4] & 0x10))
		{
			a = (page_dir & 0xffc00000) | (a & 0x003fffff);
			if (debug)
			{
				*address = a;
				return true;
			}
			perm = get_permissions(page_dir, WP);
			if (write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
				ret = false;
			else if (user && !(perm & VTLB_USER_READ_ALLOWED))
				ret = false;
			else
			{
				if (write)
					perm |= VTLB_FLAG_DIRTY;
				if (!(page_dir & 0x40) && write)
					m_program->write_dword(pdbr + directory * 4, page_dir | 0x60);
				else if (!(page_dir & 0x20))
					m_program->write_dword(pdbr + directory * 4, page_dir | 0x20);
				ret = true;
			}
		}
		else
		{
			uint32_t page_entry = m_program->read_dword((page_dir & 0xfffff000) + (table * 4));
			if (!(page_entry & 1))
				ret = false;
			else
			{
				a = (page_entry & 0xfffff000) | (a & 0xfff);
				if (debug)
				{
					*address = a;
					return true;
				}
				perm = get_permissions(page_entry, WP);
				if (write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
					ret = false;
				else if (user && !(perm & VTLB_USER_READ_ALLOWED))
					ret = false;
				else
				{
					if (write)
						perm |= VTLB_FLAG_DIRTY;
					if (!(page_dir & 0x20))
						m_program->write_dword(pdbr + directory * 4, page_dir | 0x20);
					if (!(page_entry & 0x40) && write)
						m_program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
					else if (!(page_entry & 0x20))
						m_program->write_dword((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
					ret = true;
				}
			}
		}
	}
	else
		ret = false;
	if (entry)
		*entry = perm;
	if (ret)
		*address = a;
	return ret;
}

//#define TEST_TLB

bool i386_device::translate_address(int pl, int type, uint32_t *address, uint32_t *error)
{
	if (!(m_cr[0] & 0x80000000)) // Some (very few) old OS's won't work with this
		return true;

	const vtlb_entry *table = d_vtlb->vtlb_table();
	uint32_t index = *address >> 12;
	vtlb_entry entry = table[index];
	if (type == TRANSLATE_FETCH)
		type = TRANSLATE_READ;
	if (pl == 3)
		type |= TRANSLATE_USER_MASK;
#ifdef TEST_TLB
	uint32_t test_addr = *address;
#endif

	if (!(entry & VTLB_FLAG_VALID) || ((type & TRANSLATE_WRITE) && !(entry & VTLB_FLAG_DIRTY)))
	{
		if (!i386_translate_address(type, address, &entry))
		{
			*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((m_CPL == 3) ? 4 : 0);
			if (entry)
				*error |= 1;
			return false;
		}
		d_vtlb->vtlb_dynload(index, *address, entry);
		return true;
	}
	if (!(entry & (1 << type)))
	{
		*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((m_CPL == 3) ? 4 : 0) | 1;
		return false;
	}
	*address = (entry & 0xfffff000) | (*address & 0xfff);
#ifdef TEST_TLB
	int test_ret = i386_translate_address(type | TRANSLATE_DEBUG_MASK, &test_addr, nullptr);
	if (!test_ret || (test_addr != *address))
		logerror("TLB-PTE mismatch! %06X %06X %06x\n", *address, test_addr, m_pc);
#endif
	return true;
}

uint32_t i386_device::i386_load_protected_mode_segment(I386_SREG *seg, uint64_t *desc )
{
	uint32_t v1,v2;
	uint32_t base, limit;
	int entry;

	if(!seg->selector)
	{
		seg->flags = 0;
		seg->base = 0;
		seg->limit = 0;
		seg->d = 0;
		seg->valid = false;
		return 0;
	}

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	v1 = READ32PL0(base + entry );
	v2 = READ32PL0(base + entry + 4 );

	seg->flags = (v2 >> 8) & 0xf0ff;
	seg->base = (v2 & 0xff000000) | ((v2 & 0xff) << 16) | ((v1 >> 16) & 0xffff);
	seg->limit = (v2 & 0xf0000) | (v1 & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = true;

	if(desc)
		*desc = ((uint64_t)v2<<32)|v1;
	return 1;
}

void i386_device::i386_load_call_gate(I386_CALL_GATE *gate)
{
	uint32_t v1,v2;
	uint32_t base,limit;
	int entry;

	if ( gate->segment & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = gate->segment & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return;

	v1 = READ32PL0(base + entry );
	v2 = READ32PL0(base + entry + 4 );

	/* Note that for task gates, offset and dword_count are not used */
	gate->selector = (v1 >> 16) & 0xffff;
	gate->offset = (v1 & 0x0000ffff) | (v2 & 0xffff0000);
	gate->ar = (v2 >> 8) & 0xff;
	gate->dword_count = v2 & 0x001f;
	gate->present = (gate->ar >> 7) & 0x01;
	gate->dpl = (gate->ar >> 5) & 0x03;
}

void i386_device::i386_set_descriptor_accessed(uint16_t selector)
{
	// assume the selector is valid, we don't need to check it again
	uint32_t base, addr;
	uint8_t rights;
	if(!(selector & ~3))
		return;

	if ( selector & 0x4 )
		base = m_ldtr.base;
	else
		base = m_gdtr.base;

	addr = base + (selector & ~7) + 5;
	i386_translate_address(TRANSLATE_READ, &addr, nullptr);
	rights = m_program->read_byte(addr);
	// Should a fault be thrown if the table is read only?
	m_program->write_byte(addr, rights | 1);
}

void i386_device::i386_load_segment_descriptor(int segment )
{
	if (PROTECTED_MODE)
	{
		uint16_t old_flags = m_sreg[segment].flags;
		if (!V8086_MODE)
		{
			i386_load_protected_mode_segment(&m_sreg[segment], nullptr);
			if (m_sreg[segment].selector)
			{
				i386_set_descriptor_accessed(m_sreg[segment].selector);
				m_sreg[segment].flags |= 0x0001;
			}
		}
		else
		{
			m_sreg[segment].base = m_sreg[segment].selector << 4;
			m_sreg[segment].limit = 0xffff;
			m_sreg[segment].flags = (segment == CS) ? 0x00fb : 0x00f3;
			m_sreg[segment].d = 0;
			m_sreg[segment].valid = true;
		}
		if (segment == CS && m_sreg[segment].flags != old_flags)
			debugger_privilege_hook();
	}
	else
	{
		m_sreg[segment].base = m_sreg[segment].selector << 4;
		m_sreg[segment].d = 0;
		m_sreg[segment].valid = true;

		if (segment == CS)
		{
			if (!m_performed_intersegment_jump)
				m_sreg[segment].base |= 0xfff00000;
			if (m_cpu_version < 0x500)
				m_sreg[segment].flags = 0x93;
		}
	}
}

/* Retrieves the stack selector located in the current TSS */
uint32_t i386_device::i386_get_stack_segment(uint8_t privilege)
{
	uint32_t ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL0((m_task.base+8) + (8*privilege));
	else
		ret = READ16PL0((m_task.base+4) + (4*privilege));

	return ret;
}

/* Retrieves the stack pointer located in the current TSS */
uint32_t i386_device::i386_get_stack_ptr(uint8_t privilege)
{
	uint32_t ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL0((m_task.base+4) + (8*privilege));
	else
		ret = READ16PL0((m_task.base+2) + (4*privilege));

	return ret;
}

uint32_t i386_device::get_flags() const
{
	uint32_t f = 0x2;
	f |= m_CF;
	f |= m_PF << 2;
	f |= m_AF << 4;
	f |= m_ZF << 6;
	f |= m_SF << 7;
	f |= m_TF << 8;
	f |= m_IF << 9;
	f |= m_DF << 10;
	f |= m_OF << 11;
	f |= m_IOP1 << 12;
	f |= m_IOP2 << 13;
	f |= m_NT << 14;
	f |= m_RF << 16;
	f |= m_VM << 17;
	f |= m_AC << 18;
	f |= m_VIF << 19;
	f |= m_VIP << 20;
	f |= m_ID << 21;
	return (m_eflags & ~m_eflags_mask) | (f & m_eflags_mask);
}

void i386_device::set_flags(uint32_t f )
{
	f &= m_eflags_mask;
	m_CF = (f & 0x1) ? 1 : 0;
	m_PF = (f & 0x4) ? 1 : 0;
	m_AF = (f & 0x10) ? 1 : 0;
	m_ZF = (f & 0x40) ? 1 : 0;
	m_SF = (f & 0x80) ? 1 : 0;
	m_TF = (f & 0x100) ? 1 : 0;
	m_IF = (f & 0x200) ? 1 : 0;
	m_DF = (f & 0x400) ? 1 : 0;
	m_OF = (f & 0x800) ? 1 : 0;
	m_IOP1 = (f & 0x1000) ? 1 : 0;
	m_IOP2 = (f & 0x2000) ? 1 : 0;
	m_NT = (f & 0x4000) ? 1 : 0;
	m_RF = (f & 0x10000) ? 1 : 0;
	m_VM = (f & 0x20000) ? 1 : 0;
	m_AC = (f & 0x40000) ? 1 : 0;
	m_VIF = (f & 0x80000) ? 1 : 0;
	m_VIP = (f & 0x100000) ? 1 : 0;
	m_ID = (f & 0x200000) ? 1 : 0;
	m_eflags = f;
}

void i386_device::sib_byte(uint8_t mod, uint32_t* out_ea, uint8_t* out_segment)
{
	uint32_t ea = 0;
	uint8_t segment = 0;
	uint8_t scale, i, base;
	uint8_t sib = FETCH();
	scale = (sib >> 6) & 0x3;
	i = (sib >> 3) & 0x7;
	base = sib & 0x7;

	switch( base )
	{
		case 0: ea = REG32(EAX); segment = DS; break;
		case 1: ea = REG32(ECX); segment = DS; break;
		case 2: ea = REG32(EDX); segment = DS; break;
		case 3: ea = REG32(EBX); segment = DS; break;
		case 4: ea = REG32(ESP); segment = SS; break;
		case 5:
			if( mod == 0 ) {
				ea = FETCH32();
				segment = DS;
			} else if( mod == 1 ) {
				ea = REG32(EBP);
				segment = SS;
			} else if( mod == 2 ) {
				ea = REG32(EBP);
				segment = SS;
			}
			break;
		case 6: ea = REG32(ESI); segment = DS; break;
		case 7: ea = REG32(EDI); segment = DS; break;
	}
	switch( i )
	{
		case 0: ea += REG32(EAX) * (1 << scale); break;
		case 1: ea += REG32(ECX) * (1 << scale); break;
		case 2: ea += REG32(EDX) * (1 << scale); break;
		case 3: ea += REG32(EBX) * (1 << scale); break;
		case 4: break;
		case 5: ea += REG32(EBP) * (1 << scale); break;
		case 6: ea += REG32(ESI) * (1 << scale); break;
		case 7: ea += REG32(EDI) * (1 << scale); break;
	}
	*out_ea = ea;
	*out_segment = segment;
}

void i386_device::modrm_to_EA(uint8_t mod_rm, uint32_t* out_ea, uint8_t* out_segment)
{
	int8_t disp8;
	int16_t disp16;
	int32_t disp32;
	uint8_t mod = (mod_rm >> 6) & 0x3;
	uint8_t rm = mod_rm & 0x7;
	uint32_t ea;
	uint8_t segment;

	if( mod_rm >= 0xc0 )
		fatalerror("i386: Called modrm_to_EA with modrm value %02X!\n",mod_rm);


	if( m_address_size ) {
		switch( rm )
		{
			default:
			case 0: ea = REG32(EAX); segment = DS; break;
			case 1: ea = REG32(ECX); segment = DS; break;
			case 2: ea = REG32(EDX); segment = DS; break;
			case 3: ea = REG32(EBX); segment = DS; break;
			case 4: sib_byte(mod, &ea, &segment ); break;
			case 5:
				if( mod == 0 ) {
					ea = FETCH32(); segment = DS;
				} else {
					ea = REG32(EBP); segment = SS;
				}
				break;
			case 6: ea = REG32(ESI); segment = DS; break;
			case 7: ea = REG32(EDI); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (int32_t)disp8;
		} else if( mod == 2 ) {
			disp32 = FETCH32();
			ea += disp32;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

		*out_ea = ea;
		*out_segment = segment;

	} else {
		switch( rm )
		{
			default:
			case 0: ea = REG16(BX) + REG16(SI); segment = DS; break;
			case 1: ea = REG16(BX) + REG16(DI); segment = DS; break;
			case 2: ea = REG16(BP) + REG16(SI); segment = SS; break;
			case 3: ea = REG16(BP) + REG16(DI); segment = SS; break;
			case 4: ea = REG16(SI); segment = DS; break;
			case 5: ea = REG16(DI); segment = DS; break;
			case 6:
				if( mod == 0 ) {
					ea = FETCH16(); segment = DS;
				} else {
					ea = REG16(BP); segment = SS;
				}
				break;
			case 7: ea = REG16(BX); segment = DS; break;
		}
		if( mod == 1 ) {
			disp8 = FETCH();
			ea += (int32_t)disp8;
		} else if( mod == 2 ) {
			disp16 = FETCH16();
			ea += (int32_t)disp16;
		}

		if( m_segment_prefix )
			segment = m_segment_override;

		*out_ea = ea & 0xffff;
		*out_segment = segment;
	}
}

uint32_t i386_device::GetNonTranslatedEA(uint8_t modrm,uint8_t *seg)
{
	uint8_t segment;
	uint32_t ea;
	modrm_to_EA(modrm, &ea, &segment );
	if(seg) *seg = segment;
	return ea;
}

uint32_t i386_device::GetEA(uint8_t modrm, int rwn)
{
	uint8_t segment;
	uint32_t ea;
	modrm_to_EA(modrm, &ea, &segment );
	return i386_translate(segment, ea, rwn );
}

/* Check segment register for validity when changing privilege level after an RETF */
void i386_device::i386_check_sreg_validity(int reg)
{
	uint16_t selector = m_sreg[reg].selector;
	uint8_t CPL = m_CPL;
	uint8_t DPL,RPL;
	I386_SREG desc;
	int invalid;

	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;

	/* Must be within the relevant descriptor table limits */
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
			invalid = 1;
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
			invalid = 1;
	}

	/* Must be either a data or readable code segment */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0002)) || (desc.flags & 0x0018) == 0x0010)
		invalid = 0;
	else
		invalid = 1;

	/* If a data segment or non-conforming code segment, then either DPL >= CPL or DPL >= RPL */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0004) == 0) || (desc.flags & 0x0018) == 0x0010)
	{
		if((DPL < CPL) || (DPL < RPL))
			invalid = 1;
	}

	/* if segment is invalid, then segment register is nulled */
	if(invalid != 0)
	{
		m_sreg[reg].selector = 0;
		i386_load_segment_descriptor(reg);
	}
}

int i386_device::i386_limit_check(int seg, uint32_t offset)
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		if((m_sreg[seg].flags & 0x0018) == 0x0010 && m_sreg[seg].flags & 0x0004) // if expand-down data segment
		{
			// compare if greater then 0xffffffff when we're passed the access size
			if((offset <= m_sreg[seg].limit) || ((m_sreg[seg].d)?0:(offset > 0xffff)))
			{
				logerror("Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x (expand-down)\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				return 1;
			}
		}
		else
		{
			if(offset > m_sreg[seg].limit)
			{
				logerror("Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				return 1;
			}
		}
	}
	return 0;
}

void i386_device::i386_sreg_load(uint16_t selector, uint8_t reg, bool *fault)
{
	// Checks done when MOV changes a segment register in protected mode
	uint8_t CPL,RPL,DPL;

	CPL = m_CPL;
	RPL = selector & 0x0003;

	if(!PROTECTED_MODE || V8086_MODE)
	{
		m_sreg[reg].selector = selector;
		i386_load_segment_descriptor(reg);
		if(fault) *fault = false;
		return;
	}

	if(fault) *fault = true;
	if(reg == SS)
	{
		I386_SREG stack;

		memset(&stack, 0, sizeof(stack));
		stack.selector = selector;
		i386_load_protected_mode_segment(&stack,nullptr);
		DPL = (stack.flags >> 5) & 0x03;

		if((selector & ~0x0003) == 0)
		{
			logerror("SReg Load (%08x): Selector is null.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if (RPL != CPL)
		{
			logerror("SReg Load (%08x): Selector RPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
		{
			logerror("SReg Load (%08x): Segment is not a writable data segment.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(DPL != CPL)
		{
			logerror("SReg Load (%08x): Segment DPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(!(stack.flags & 0x0080))
		{
			logerror("SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_SS,selector & ~0x03)
		}
	}
	if(reg == DS || reg == ES || reg == FS || reg == GS)
	{
		I386_SREG desc;

		if((selector & ~0x0003) == 0)
		{
			m_sreg[reg].selector = selector;
			i386_load_segment_descriptor(reg );
			if(fault) *fault = false;
			return;
		}

		memset(&desc, 0, sizeof(desc));
		desc.selector = selector;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;

		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				logerror("SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x10)
		{
			if((((desc.flags & 0x0002) != 0) && ((desc.flags & 0x0018) != 0x18)) || !(desc.flags & 0x10))
			{
				logerror("SReg Load (%08x): Segment is not a data segment or readable code segment.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(((desc.flags & 0x0018) == 0x10) || ((!(desc.flags & 0x0004)) && ((desc.flags & 0x0018) == 0x18)))
		{
			// if data or non-conforming code segment
			if((RPL > DPL) || (CPL > DPL))
			{
				logerror("SReg Load (%08x): Selector RPL or CPL is not less or equal to segment DPL.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(!(desc.flags & 0x0080))
		{
			logerror("SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)
		}
	}

	m_sreg[reg].selector = selector;
	i386_load_segment_descriptor(reg );
	if(fault) *fault = false;
}

void i386_device::i386_trap(int irq, int irq_gate, int trap_level)
{
	/*  I386 Interrupts/Traps/Faults:
	 *
	 *  0x00    Divide by zero
	 *  0x01    Debug exception
	 *  0x02    NMI
	 *  0x03    Int3
	 *  0x04    Overflow
	 *  0x05    Array bounds check
	 *  0x06    Illegal Opcode
	 *  0x07    FPU not available
	 *  0x08    Double fault
	 *  0x09    Coprocessor segment overrun
	 *  0x0a    Invalid task state
	 *  0x0b    Segment not present
	 *  0x0c    Stack exception
	 *  0x0d    General Protection Fault
	 *  0x0e    Page fault
	 *  0x0f    Reserved
	 *  0x10    Coprocessor error
	 */
	uint32_t v1, v2;
	uint32_t offset, oldflags = get_flags();
	uint16_t segment;
	int entry = irq * (PROTECTED_MODE ? 8 : 4);
	int SetRPL;
	m_lock = false;

	if( !(PROTECTED_MODE) )
	{
		/* 16-bit */
		PUSH16(oldflags & 0xffff );
		PUSH16(m_sreg[CS].selector );
		if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
			PUSH16(m_eip );
		else
			PUSH16(m_prev_eip );

		m_sreg[CS].selector = READ16(m_idtr.base + entry + 2 );
		m_eip = READ16(m_idtr.base + entry );

		m_TF = 0;
		m_IF = 0;
	}
	else
	{
		int type;
		uint16_t flags;
		I386_SREG desc;
		uint8_t CPL = m_CPL, DPL; //, RPL = 0;

		/* 32-bit */
		v1 = READ32PL0(m_idtr.base + entry );
		v2 = READ32PL0(m_idtr.base + entry + 4 );
		offset = (v2 & 0xffff0000) | (v1 & 0xffff);
		segment = (v1 >> 16) & 0xffff;
		type = (v2>>8) & 0x1F;
		flags = (v2>>8) & 0xf0ff;

		if(trap_level == 2)
		{
			logerror("IRQ: Double fault.\n");
			FAULT_EXP(FAULT_DF,0);
		}
		if(trap_level >= 3)
		{
			logerror("IRQ: Triple fault. CPU reset.\n");
			//pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			m_shutdown = true;
			write_signals(&outputs_reset, 0xffffffff);
			reset();
			return;
		}

		/* segment privilege checks */
		if(entry >= m_idtr.limit)
		{
			logerror("IRQ (%08x): Vector %02xh is past IDT limit.\n",m_pc,entry);
			FAULT_EXP(FAULT_GP,entry+2)
		}
		/* segment must be interrupt gate, trap gate, or task gate */
		if(type != 0x05 && type != 0x06 && type != 0x07 && type != 0x0e && type != 0x0f)
		{
			logerror("IRQ#%02x (%08x): Vector segment %04x is not an interrupt, trap or task gate.\n",irq,m_pc,segment);
			FAULT_EXP(FAULT_GP,entry+2)
		}

		if(m_ext == 0) // if software interrupt (caused by INT/INTO/INT3)
		{
			if(((flags >> 5) & 0x03) < CPL)
			{
				logerror("IRQ (%08x): Software IRQ - gate DPL is less than CPL.\n",m_pc);
				FAULT_EXP(FAULT_GP,entry+2)
			}
			if(V8086_MODE)
			{
				if((!m_IOP1 || !m_IOP2) && (m_opcode != 0xcc))
				{
					logerror("IRQ (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
					FAULT(FAULT_GP,0)
				}

			}
		}

		if((flags & 0x0080) == 0)
		{
			logerror("IRQ: Vector segment is not present.\n");
			FAULT_EXP(FAULT_NP,entry+2)
		}

		if(type == 0x05)
		{
			/* Task gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			if(segment & 0x04)
			{
				logerror("IRQ: Task gate: TSS is not in the GDT.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			else
			{
				if(segment > m_gdtr.limit)
				{
					logerror("IRQ: Task gate: TSS is past GDT limit.\n");
					FAULT_EXP(FAULT_TS,segment & ~0x03);
				}
			}
			if((desc.flags & 0x000f) != 0x09 && (desc.flags & 0x000f) != 0x01)
			{
				logerror("IRQ: Task gate: TSS is not an available TSS.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Task gate: TSS is not present.\n");
				FAULT_EXP(FAULT_NP,segment & ~0x03);
			}
			if(!(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1))
				m_eip = m_prev_eip;
			if(desc.flags & 0x08)
				i386_task_switch(desc.selector,1);
			else
				i286_task_switch(desc.selector,1);
			return;
		}
		else
		{
			/* Interrupt or Trap gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			CPL = m_CPL;  // current privilege level
			DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
//          RPL = segment & 0x03;  // requested privilege level

			if((segment & ~0x03) == 0)
			{
				logerror("IRQ: Gate segment is null.\n");
				FAULT_EXP(FAULT_GP,m_ext)
			}
			if(segment & 0x04)
			{
				if((segment & ~0x07) > m_ldtr.limit)
				{
					logerror("IRQ: Gate segment is past LDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			else
			{
				if((segment & ~0x07) > m_gdtr.limit)
				{
					logerror("IRQ: Gate segment is past GDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			if((desc.flags & 0x0018) != 0x18)
			{
				logerror("IRQ: Gate descriptor is not a code segment.\n");
				FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Gate segment is not present.\n");
				FAULT_EXP(FAULT_NP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0004) == 0 && (DPL < CPL))
			{
				/* IRQ to inner privilege */
				I386_SREG stack;
				uint32_t newESP,oldSS,oldESP;

				if(V8086_MODE && DPL)
				{
					logerror("IRQ: Gate to CPL>0 from VM86 mode.\n");
					FAULT_EXP(FAULT_GP,segment & ~0x03);
				}
				/* Check new stack segment in TSS */
				memset(&stack, 0, sizeof(stack));
				stack.selector = i386_get_stack_segment(DPL);
				i386_load_protected_mode_segment(&stack,nullptr);
				oldSS = m_sreg[SS].selector;
				if(flags & 0x0008)
					oldESP = REG32(ESP);
				else
					oldESP = REG16(SP);
				if((stack.selector & ~0x03) == 0)
				{
					logerror("IRQ: New stack selector is null.\n");
					FAULT_EXP(FAULT_GP,m_ext)
				}
				if(stack.selector & 0x04)
				{
					if((stack.selector & ~0x07) > m_ldtr.base)
					{
						logerror("IRQ: New stack selector is past LDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				else
				{
					if((stack.selector & ~0x07) > m_gdtr.base)
					{
						logerror("IRQ: New stack selector is past GDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				if((stack.selector & 0x03) != DPL)
				{
					logerror("IRQ: New stack selector RPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags >> 5) & 0x03) != DPL)
				{
					logerror("IRQ: New stack segment DPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
				{
					logerror("IRQ: New stack segment is not a writable data segment.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				if((stack.flags & 0x0080) == 0)
				{
					logerror("IRQ: New stack segment is not present.\n");
					FAULT_EXP(FAULT_SS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				newESP = i386_get_stack_ptr(DPL);
				if(type & 0x08) // 32-bit gate
				{
					if(((newESP < (V8086_MODE?36:20)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?36:20))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				else // 16-bit gate
				{
					newESP &= 0xffff;
					if(((newESP < (V8086_MODE?18:10)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?18:10))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				if(offset > desc.limit)
				{
					logerror("IRQ: New EIP is past code segment limit.\n");
					FAULT_EXP(FAULT_GP,0)
				}
				/* change CPL before accessing the stack */
				m_CPL = DPL;
				/* check for page fault at new stack TODO: check if stack frame crosses page boundary */
				WRITE_TEST(stack.base+newESP-1);
				/* Load new stack segment descriptor */
				m_sreg[SS].selector = stack.selector;
				i386_load_protected_mode_segment(&m_sreg[SS],nullptr);
				i386_set_descriptor_accessed(stack.selector);
				REG32(ESP) = newESP;
				if(V8086_MODE)
				{
					//logerror("IRQ (%08x): Interrupt during V8086 task\n",m_pc);
					if(type & 0x08)
					{
						PUSH32SEG(m_sreg[GS].selector & 0xffff);
						PUSH32SEG(m_sreg[FS].selector & 0xffff);
						PUSH32SEG(m_sreg[DS].selector & 0xffff);
						PUSH32SEG(m_sreg[ES].selector & 0xffff);
					}
					else
					{
						PUSH16(m_sreg[GS].selector);
						PUSH16(m_sreg[FS].selector);
						PUSH16(m_sreg[DS].selector);
						PUSH16(m_sreg[ES].selector);
					}
					m_sreg[GS].selector = 0;
					m_sreg[FS].selector = 0;
					m_sreg[DS].selector = 0;
					m_sreg[ES].selector = 0;
					m_VM = 0;
					i386_load_segment_descriptor(GS);
					i386_load_segment_descriptor(FS);
					i386_load_segment_descriptor(DS);
					i386_load_segment_descriptor(ES);
				}
				if(type & 0x08)
				{
					// 32-bit gate
					PUSH32SEG(oldSS);
					PUSH32(oldESP);
				}
				else
				{
					// 16-bit gate
					PUSH16(oldSS);
					PUSH16(oldESP);
				}
				SetRPL = 1;
			}
			else
			{
				int stack_limit;
				if((desc.flags & 0x0004) || (DPL == CPL))
				{
					/* IRQ to same privilege */
					if(V8086_MODE && !m_ext)
					{
						logerror("IRQ: Gate to same privilege from VM86 mode.\n");
						FAULT_EXP(FAULT_GP,segment & ~0x03);
					}
					if(type == 0x0e || type == 0x0f)  // 32-bit gate
						stack_limit = 10;
					else
						stack_limit = 6;
					// TODO: Add check for error code (2 extra bytes)
					if(REG32(ESP) < stack_limit)
					{
						logerror("IRQ: Stack has no space left (needs %i bytes).\n",stack_limit);
						FAULT_EXP(FAULT_SS,0)
					}
					if(offset > desc.limit)
					{
						logerror("IRQ: Gate segment offset is past segment limit.\n");
						FAULT_EXP(FAULT_GP,0)
					}
					SetRPL = 1;
				}
				else
				{
					logerror("IRQ: Gate descriptor is non-conforming, and DPL does not equal CPL.\n");
					FAULT_EXP(FAULT_GP,segment)
				}
			}
		}
		uint32_t tempSP = REG32(ESP);
		try
		{
			// this is ugly but the alternative is worse
			if(type != 0x0e && type != 0x0f)  // if not 386 interrupt or trap gate
			{
				PUSH16(oldflags & 0xffff );
				PUSH16(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH16(m_eip );
				else
					PUSH16(m_prev_eip );
			}
			else
			{
				PUSH32(oldflags & 0x00ffffff );
				PUSH32SEG(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH32(m_eip );
				else
					PUSH32(m_prev_eip );
			}
		}
		catch(uint64_t e)
		{
			REG32(ESP) = tempSP;
			throw e;
		}
		if(SetRPL != 0)
			segment = (segment & ~0x03) | m_CPL;
		m_sreg[CS].selector = segment;
		m_eip = offset;

		if(type == 0x0e || type == 0x06)
			m_IF = 0;
		m_TF = 0;
		m_NT = 0;
		m_eflags = get_flags();
#if 0
		if((irq >= 0x10) && (irq_gate == 1) && (m_ext == 0)) {
			// Try to call pseudo bios
			i386_load_segment_descriptor(CS);
			uint32_t tmp_pc = i386_translate(CS, m_eip, -1, 1 );
			int stat = 0;
			bios_trap_x86(tmp_pc, stat);
			if(stat != 0) { // HIT
				try	{
					// this is ugly but the alternative is worse
					if(/*type != 0x0e && type != 0x0f*/ (type & 0x08) == 0)  // if not 386 interrupt or trap gate
					{
						m_eip = POP16();
						m_sreg[CS].selector = POP16();
						UINT32 __flags = POP16();
						m_eflags = (get_flags() & 0xffff0000) | (__flags & 0x0000ffff);
						set_flags(m_eflags);
					}
					else
					{
						m_eip = POP32();
						UINT32 sel;
						sel = POP32();
						m_sreg[CS].selector = sel; // ToDo: POP32SEG()
						UINT32 __flags = POP32();
						m_eflags = (get_flags() & 0xff000000) | (__flags & 0x00ffffff);
						set_flags(m_eflags);
					}
				}
				catch(UINT64 e)
				{
					REG32(ESP) = tempSP;
					//logerror("THROWN EXCEPTION %08X at i386_trap() IRQ=%02x EIP=%08x V8086_MODE=%s line %d\n", e, irq, cpustate->eip, (V8086_MODE) ? "Yes" : "No", __LINE__);
					throw e;
				}
				return;
			}
			CHANGE_PC(m_eip);
			return;
		}
#endif
	}
	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

void i386_device::i386_trap_with_error(int irq, int irq_gate, int trap_level, uint32_t error)
{
	i386_trap(irq,irq_gate,trap_level);
	if(irq == 8 || irq == 10 || irq == 11 || irq == 12 || irq == 13 || irq == 14)
	{
		// for these exceptions, an error code is pushed onto the stack by the processor.
		// no error code is pushed for software interrupts, either.
		if(PROTECTED_MODE)
		{
			uint32_t entry = irq * 8;
			uint32_t v2,type;
			v2 = READ32PL0(m_idtr.base + entry + 4 );
			type = (v2>>8) & 0x1F;
			if(type == 5)
			{
				v2 = READ32PL0(m_idtr.base + entry);
				v2 = READ32PL0(m_gdtr.base + ((v2 >> 16) & 0xfff8) + 4);
				type = (v2>>8) & 0x1F;
			}
			if(type >= 9)
				PUSH32(error);
			else
				PUSH16(error);
		}
		else
			PUSH16(error);
	}
}


void i386_device::i286_task_switch(uint16_t selector, uint8_t nested)
{
	uint32_t tss;
	I386_SREG seg;
	uint16_t old_task;
	uint8_t ar_byte;  // access rights byte

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE16(tss+0x0e,m_eip & 0x0000ffff);
	WRITE16(tss+0x10,get_flags() & 0x0000ffff);
	WRITE16(tss+0x12,REG16(AX));
	WRITE16(tss+0x14,REG16(CX));
	WRITE16(tss+0x16,REG16(DX));
	WRITE16(tss+0x18,REG16(BX));
	WRITE16(tss+0x1a,REG16(SP));
	WRITE16(tss+0x1c,REG16(BP));
	WRITE16(tss+0x1e,REG16(SI));
	WRITE16(tss+0x20,REG16(DI));
	WRITE16(tss+0x22,m_sreg[ES].selector);
	WRITE16(tss+0x24,m_sreg[CS].selector);
	WRITE16(tss+0x26,m_sreg[SS].selector);
	WRITE16(tss+0x28,m_sreg[DS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= 0x08;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ16(tss+0x2a) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ16(tss+0x0e);
	set_flags(READ16(tss+0x10));
	REG16(AX) = READ16(tss+0x12);
	REG16(CX) = READ16(tss+0x14);
	REG16(DX) = READ16(tss+0x16);
	REG16(BX) = READ16(tss+0x18);
	REG16(SP) = READ16(tss+0x1a);
	REG16(BP) = READ16(tss+0x1c);
	REG16(SI) = READ16(tss+0x1e);
	REG16(DI) = READ16(tss+0x20);
	m_sreg[ES].selector = READ16(tss+0x22) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ16(tss+0x24) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ16(tss+0x26) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ16(tss+0x28) & 0xffff;
	i386_load_segment_descriptor(DS);

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register */
	if(nested != 0)
	{
		WRITE16(tss+0,old_task);
		m_NT = 1;
	}
	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;
//  printf("286 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_task_switch(uint16_t selector, uint8_t nested)
{
	uint32_t tss;
	I386_SREG seg;
	uint16_t old_task;
	uint8_t ar_byte;  // access rights byte
	uint32_t oldcr3 = m_cr[3];

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE32(tss+0x1c,m_cr[3]);  // correct?
	WRITE32(tss+0x20,m_eip);
	WRITE32(tss+0x24,get_flags());
	WRITE32(tss+0x28,REG32(EAX));
	WRITE32(tss+0x2c,REG32(ECX));
	WRITE32(tss+0x30,REG32(EDX));
	WRITE32(tss+0x34,REG32(EBX));
	WRITE32(tss+0x38,REG32(ESP));
	WRITE32(tss+0x3c,REG32(EBP));
	WRITE32(tss+0x40,REG32(ESI));
	WRITE32(tss+0x44,REG32(EDI));
	WRITE32(tss+0x48,m_sreg[ES].selector);
	WRITE32(tss+0x4c,m_sreg[CS].selector);
	WRITE32(tss+0x50,m_sreg[SS].selector);
	WRITE32(tss+0x54,m_sreg[DS].selector);
	WRITE32(tss+0x58,m_sreg[FS].selector);
	WRITE32(tss+0x5c,m_sreg[GS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= 0x08;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ32(tss+0x60) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ32(tss+0x20);
	set_flags(READ32(tss+0x24));
	REG32(EAX) = READ32(tss+0x28);
	REG32(ECX) = READ32(tss+0x2c);
	REG32(EDX) = READ32(tss+0x30);
	REG32(EBX) = READ32(tss+0x34);
	REG32(ESP) = READ32(tss+0x38);
	REG32(EBP) = READ32(tss+0x3c);
	REG32(ESI) = READ32(tss+0x40);
	REG32(EDI) = READ32(tss+0x44);
	m_sreg[ES].selector = READ32(tss+0x48) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ32(tss+0x4c) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ32(tss+0x50) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ32(tss+0x54) & 0xffff;
	i386_load_segment_descriptor(DS);
	m_sreg[FS].selector = READ32(tss+0x58) & 0xffff;
	i386_load_segment_descriptor(FS);
	m_sreg[GS].selector = READ32(tss+0x5c) & 0xffff;
	i386_load_segment_descriptor(GS);
	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register before setting cr3 as the old tss address might be gone */
	if(nested != 0)
	{
		WRITE32(tss+0,old_task);
		m_NT = 1;
	}
	m_cr[3] = READ32(tss+0x1c);  // CR3 (PDBR)
	if(oldcr3 != m_cr[3])
		d_vtlb->vtlb_flush_dynamic();

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;
//  printf("386 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_check_irq_line()
{
	if(!m_smm && m_smi)
	{
		pentium_smi();
		return;
	}

	/* Check if the interrupts are enabled */
	if ( (m_irq_state) && m_IF )
	{
		m_cycles -= 2;
		int irqnum = d_pic->get_intr_ack();
		try {
			i386_trap(irqnum, 1, 0);
		} catch(uint64_t e) {
			//logdebug("EXCEPTION %08X VIA INTERRUPT/TRAP HANDLING IRQ=%02Xh(%d) ADDR=%08X\n", e, irqnum, irqnum, m_pc);
		}
		m_irq_state = 0;
	}
}

bool i386_device::bios_int_x86(int num)
{
	if(d_bios == NULL) return false;
	uint16_t regs[10], sregs[4]; // ToDo: Full calling
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); 
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI);
	regs[8] = 0x0000; regs[9] = 0x0000;							
	sregs[0] = m_sreg[ES].selector; sregs[1] = m_sreg[CS].selector;
	sregs[2] = m_sreg[SS].selector; sregs[3] = m_sreg[DS].selector;
	int32_t ZeroFlag = m_ZF, CarryFlag = m_CF;
	if(d_bios->bios_int_i86(num, regs, sregs, &ZeroFlag, &CarryFlag, &m_cycles, &total_cycles)) { 
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; 
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; 
		m_ZF = (UINT8)ZeroFlag; m_CF = (UINT8)CarryFlag; 
		CYCLES(CYCLES_IRET);							
		if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		
			uint32_t hi = regs[9];								
			uint32_t lo = regs[8];								
			uint32_t addr = (hi << 16) | lo;					
			m_eip = addr;								
		}														
		return true;													
	}
	return false;
}		


bool i386_device::bios_call_far_x86(uint32_t address)
{
	if(d_bios == NULL) return false;
	if(((m_cr[0] & 0x0001) != 0) && (m_VM == 0)) return false; // Return if (!(VM8086) && (Protected))
	
	uint16_t regs[10], sregs[4]; // ToDo: Full calling
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); 
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI);
	regs[8] = 0x0000; regs[9] = 0x0000;							
	sregs[0] = m_sreg[ES].selector; sregs[1] = m_sreg[CS].selector;
	sregs[2] = m_sreg[SS].selector; sregs[3] = m_sreg[DS].selector;
	int32_t ZeroFlag = m_ZF, CarryFlag = m_CF;
	if(d_bios->bios_call_far_i86(address, regs, sregs, &ZeroFlag, &CarryFlag, &m_cycles, &total_cycles)) { 
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; 
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; 
		m_ZF = (UINT8)ZeroFlag; m_CF = (UINT8)CarryFlag; 
		CYCLES(CYCLES_RET_INTERSEG);							
		if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		
			uint32_t hi = regs[9];								
			uint32_t lo = regs[8];								
			uint32_t addr = (hi << 16) | lo;					
			m_eip = addr;								
		}														
		return true;													
	}
	return false;
}		

bool i386_device::bios_trap_x86(uint32_t address, int &stat)
{
	if(d_bios == NULL) return false;
	if(((m_cr[0] & 0x0001) != 0) && (m_VM == 0)) return false; // Return if (!(VM8086) && (Protected))
	
	uint16_t regs[10], sregs[4]; // ToDo: Full calling
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); 
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI);
	regs[8] = 0x0000; regs[9] = 0x0000;							
	sregs[0] = m_sreg[ES].selector; sregs[1] = m_sreg[CS].selector;
	sregs[2] = m_sreg[SS].selector; sregs[3] = m_sreg[DS].selector;
	int32_t ZeroFlag = m_ZF, CarryFlag = m_CF;
	stat = 0;
	if(d_bios->bios_call_far_i86(address, regs, sregs, &ZeroFlag, &CarryFlag, &m_cycles, &total_cycles)) { 
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; 
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; 
		m_ZF = (UINT8)ZeroFlag; m_CF = (UINT8)CarryFlag; 
		CYCLES(CYCLES_RET_INTERSEG);							
		if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		
			uint32_t hi = regs[9];								
			uint32_t lo = regs[8];								
			uint32_t addr = (hi << 16) | lo;					
			m_eip = addr;								
		}
		stat = 1;
		return true;													
	}
	return false;
}		

void i386_device::i386_protected_mode_jump(uint16_t seg, uint32_t off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE call_gate;
	uint8_t CPL,DPL,RPL;
	uint8_t SetRPL;
	uint16_t segment = seg;
	uint32_t offset = off;

	/* Check selector is not null */
	if((segment & ~0x03) == 0)
	{
		logerror("JMP: Segment is null.\n");
		FAULT(FAULT_GP,0)
	}
	/* Selector is within descriptor table limit */
	if((segment & 0x04) == 0)
	{
		/* check GDT limit */
		if((segment & ~0x07) > (m_gdtr.limit))
		{
			logerror("JMP: Segment is past GDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	else
	{
		/* check LDT limit */
		if((segment & ~0x07) > (m_ldtr.limit))
		{
			logerror("JMP: Segment is past LDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = segment;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = segment & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x0018)
	{
		/* code segment */
		if((desc.flags & 0x0004) == 0)
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				logerror("JMP: RPL %i is less than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
			if(DPL != CPL)
			{
				logerror("JMP: DPL %i is not equal CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		else
		{
			/* conforming */
			if(DPL > CPL)
			{
				logerror("JMP: DPL %i is less than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			logerror("JMP: Segment is not present\n");
			FAULT(FAULT_NP,segment & 0xfffc)
		}
		if(offset > desc.limit)
		{
			logerror("JMP: Offset is past segment limit\n");
			FAULT(FAULT_GP,0)
		}
	}
	else
	{
		if((desc.flags & 0x0010) != 0)
		{
			logerror("JMP: Segment is a data segment\n");
			FAULT(FAULT_GP,segment & 0xfffc)  // #GP (cannot execute code in a data segment)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // 286 Available TSS
			case 0x09:  // 386 Available TSS
				logerror("JMP: Available 386 TSS at %08x\n",m_pc);
				memset(&desc, 0, sizeof(desc));
				desc.selector = segment;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				if(DPL < CPL)
				{
					logerror("JMP: TSS: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: TSS: DPL %i is less than TSS RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: TSS: Segment is not present\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(desc.flags & 0x0008)
					i386_task_switch(desc.selector,0);
				else
					i286_task_switch(desc.selector,0);
				return;
			case 0x04:  // 286 Call Gate
			case 0x0c:  // 386 Call Gate
				//logerror("JMP: Call gate at %08x\n",m_pc);
				SetRPL = 1;
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					logerror("JMP: Call Gate: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: Call Gate: DPL %i is less than RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: Call Gate: Segment is not present\n");
					FAULT(FAULT_NP,segment & 0xfffc)
				}
				/* Now we examine the segment that the call gate refers to */
				if(call_gate.selector == 0)
				{
					logerror("JMP: Call Gate: Gate selector is null\n");
					FAULT(FAULT_GP,0)
				}
				if(call_gate.selector & 0x04)
				{
					if((call_gate.selector & ~0x07) > m_ldtr.limit)
					{
						logerror("JMP: Call Gate: Gate Selector is past LDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("JMP: Call Gate: Gate Selector is past GDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;
				if((desc.flags & 0x0018) != 0x18)
				{
					logerror("JMP: Call Gate: Gate does not point to a code segment\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if((desc.flags & 0x0004) == 0)
				{  // non-conforming
					if(DPL != CPL)
					{
						logerror("JMP: Call Gate: Gate DPL does not equal CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{  // conforming
					if(DPL > CPL)
					{
						logerror("JMP: Call Gate: Gate DPL is greater than CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("JMP: Call Gate: Gate Segment is not present\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.offset > desc.limit)
				{
					logerror("JMP: Call Gate: Gate offset is past Gate segment limit\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				segment = call_gate.selector;
				offset = call_gate.offset;
				break;
			case 0x05:  // Task Gate
				logerror("JMP: Task gate at %08x\n",m_pc);
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					logerror("JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					logerror("JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					logerror("JMP: Task Gate: Gate is not present.\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = call_gate.selector & 0x03;  // requested privilege level
				if(call_gate.selector & 0x04)
				{
					logerror("JMP: Task Gate TSS: TSS must be global.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("JMP: Task Gate TSS: TSS is past GDT limit.\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((call_gate.ar & 0x000f) == 0x0009 || (call_gate.ar & 0x000f) == 0x0001)
				{
					logerror("JMP: Task Gate TSS: Segment is not an available TSS.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					logerror("JMP: Task Gate TSS: TSS is not present.\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.ar & 0x08)
					i386_task_switch(call_gate.selector,0);
				else
					i286_task_switch(call_gate.selector,0);
				return;
			default:  // invalid segment type
				logerror("JMP: Invalid segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
	}

	if(SetRPL != 0)
		segment = (segment & ~0x03) | m_CPL;
	if(operand32 == 0)
		m_eip = offset & 0x0000ffff;
	else
		m_eip = offset;
	m_sreg[CS].selector = segment;
	m_performed_intersegment_jump = 1;
	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_call(uint16_t seg, uint32_t off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE gate;
	uint8_t SetRPL;
	uint8_t CPL, DPL, RPL;
	uint16_t selector = seg;
	uint32_t offset = off;
	int x;

	if((selector & ~0x03) == 0)
	{
		logerror("CALL (%08x): Selector is null.\n",m_pc);
		FAULT(FAULT_GP,0)  // #GP(0)
	}
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
		{
			logerror("CALL: Selector is past LDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
		{
			logerror("CALL: Selector is past GDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}

	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x18)  // is a code segment
	{
		if(desc.flags & 0x0004)
		{
			/* conforming */
			if(DPL > CPL)
			{
				logerror("CALL: Code segment DPL %i is greater than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		else
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				logerror("CALL: RPL %i is greater than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
			if(DPL != CPL)
			{
				logerror("CALL: Code segment DPL %i is not equal to CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			logerror("CALL (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)  // #NP(selector)
		}
		if (operand32 != 0)  // if 32-bit
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) - 8 : (REG16(SP) - 8) & 0xffff);
			if(i386_limit_check(SS, offset))
			{
				logerror("CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) - 4 : (REG16(SP) - 4) & 0xffff);
			if(i386_limit_check(SS, offset))
			{
				logerror("CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		if(offset > desc.limit)
		{
			logerror("CALL: EIP is past segment limit.\n");
			FAULT(FAULT_GP,0)  // #GP(0)
		}
	}
	else
	{
		/* special segment type */
		if(desc.flags & 0x0010)
		{
			logerror("CALL: Segment is a data segment.\n");
			FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // Available 286 TSS
			case 0x09:  // Available 386 TSS
				logerror("CALL: Available TSS at %08x\n",m_pc);
				if(DPL < CPL)
				{
					logerror("CALL: TSS: DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: TSS: DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x0002)
				{
					logerror("CALL: TSS: TSS is busy.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL: TSS: Segment %02x is not present.\n",selector);
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);
				else
					i286_task_switch(desc.selector,1);
				return;
			case 0x04:  // 286 call gate
			case 0x0c:  // 386 call gate
				if((desc.flags & 0x000f) == 0x04)
					operand32 = 0;
				else
					operand32 = 1;
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				//logerror("CALL: Call gate at %08x (%i parameters)\n",m_pc,gate.dword_count);
				if(DPL < CPL)
				{
					logerror("CALL: Call gate DPL %i is less than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: Call gate DPL %i is less than RPL %i.\n",DPL,RPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(gate.present == 0)
				{
					logerror("CALL: Call gate is not present.\n");
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #GP(selector)
				}
				desc.selector = gate.selector;
				if((gate.selector & ~0x03) == 0)
				{
					logerror("CALL: Call gate: Segment is null.\n");
					FAULT(FAULT_GP,0)  // #GP(0)
				}
				if(desc.selector & 0x04)
				{
					if((desc.selector & ~0x07) > m_ldtr.limit)
					{
						logerror("CALL: Call gate: Segment is past LDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				else
				{
					if((desc.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("CALL: Call gate: Segment is past GDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				i386_load_protected_mode_segment(&desc,nullptr);
				if((desc.flags & 0x0018) != 0x18)
				{
					logerror("CALL: Call gate: Segment is not a code segment.\n");
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				DPL = ((desc.flags >> 5) & 0x03);
				if(DPL > CPL)
				{
					logerror("CALL: Call gate: Segment DPL %i is greater than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL (%08x): Code segment is not present.\n",m_pc);
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #NP(selector)
				}
				if(DPL < CPL && (desc.flags & 0x0004) == 0)
				{
					I386_SREG stack;
					I386_SREG temp;
					uint32_t oldSS,oldESP;
					/* more privilege */
					/* Check new SS segment for privilege level from TSS */
					memset(&stack, 0, sizeof(stack));
					stack.selector = i386_get_stack_segment(DPL);
					i386_load_protected_mode_segment(&stack,nullptr);
					if((stack.selector & ~0x03) == 0)
					{
						logerror("CALL: Call gate: TSS selector is null\n");
						FAULT(FAULT_TS,0)  // #TS(0)
					}
					if(stack.selector & 0x04)
					{
						if((stack.selector & ~0x07) > m_ldtr.limit)
						{
							logerror("CALL: Call gate: TSS selector is past LDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					else
					{
						if((stack.selector & ~0x07) > m_gdtr.limit)
						{
							logerror("CALL: Call gate: TSS selector is past GDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					if((stack.selector & 0x03) != DPL)
					{
						logerror("CALL: Call gate: Stack selector RPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if(((stack.flags >> 5) & 0x03) != DPL)
					{
						logerror("CALL: Call gate: Stack DPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0018) != 0x10 && (stack.flags & 0x0002))
					{
						logerror("CALL: Call gate: Stack segment is not a writable data segment\n");
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0080) == 0)
					{
						logerror("CALL: Call gate: Stack segment is not present\n");
						FAULT(FAULT_SS,stack.selector)  // #SS(SS selector)
					}
					uint32_t newESP = i386_get_stack_ptr(DPL);
					if(!stack.d)
					{
						newESP &= 0xffff;
					}
					if(operand32 != 0)
					{
						if(newESP < ((gate.dword_count & 0x1f) + 16))
						{
							logerror("CALL: Call gate: New stack has no room for 32-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if(gate.offset > desc.limit)
						{
							logerror("CALL: Call gate: EIP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					else
					{
						if(newESP < ((gate.dword_count & 0x1f) + 8))
						{
							logerror("CALL: Call gate: New stack has no room for 16-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if((gate.offset & 0xffff) > desc.limit)
						{
							logerror("CALL: Call gate: IP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					selector = gate.selector;
					offset = gate.offset;

					m_CPL = (stack.flags >> 5) & 0x03;
					/* check for page fault at new stack */
					WRITE_TEST(stack.base+newESP-1);
					/* switch to new stack */
					oldSS = m_sreg[SS].selector;
					m_sreg[SS].selector = i386_get_stack_segment(m_CPL);
					if(operand32 != 0)
					{
						oldESP = REG32(ESP);
					}
					else
					{
						oldESP = REG16(SP);
					}
					i386_load_segment_descriptor(SS );
					REG32(ESP) = newESP;

					if(operand32 != 0)
					{
						PUSH32SEG(oldSS);
						PUSH32(oldESP);
					}
					else
					{
						PUSH16(oldSS);
						PUSH16(oldESP & 0xffff);
					}

					memset(&temp, 0, sizeof(temp));
					temp.selector = oldSS;
					i386_load_protected_mode_segment(&temp,nullptr);
					/* copy parameters from old stack to new stack */
					for(x=(gate.dword_count & 0x1f)-1;x>=0;x--)
					{
						uint32_t addr = oldESP + (operand32?(x*4):(x*2));
						addr = temp.base + (temp.d?addr:(addr&0xffff));
						if(operand32)
							PUSH32(READ32(addr));
						else
							PUSH16(READ16(addr));
					}
					SetRPL = 1;
				}
				else
				{
					/* same privilege */
					if (operand32 != 0)  // if 32-bit
					{
						uint32_t stkoff = (STACK_32BIT ? REG32(ESP) - 8 : (REG16(SP) - 8) & 0xffff);
						if(i386_limit_check(SS, stkoff))
						{
							logerror("CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset;
					}
					else
					{
						uint32_t stkoff = (STACK_32BIT ? REG32(ESP) - 4 : (REG16(SP) - 4) & 0xffff);
						if(i386_limit_check(SS, stkoff))
						{
							logerror("CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset & 0xffff;
					}
					if(offset > desc.limit)
					{
						logerror("CALL: EIP is past segment limit.\n");
						FAULT(FAULT_GP,0) // #GP(0)
					}
					SetRPL = 1;
				}
				break;
			case 0x05:  // task gate
				logerror("CALL: Task gate at %08x\n",m_pc);
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				if(DPL < CPL)
				{
					logerror("CALL: Task Gate: Gate DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					logerror("CALL: Task Gate: Gate DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((gate.ar & 0x0080) == 0)
				{
					logerror("CALL: Task Gate: Gate is not present.\n");
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				if(gate.selector & 0x04)
				{
					logerror("CALL: Task Gate: TSS is not global.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				else
				{
					if((gate.selector & ~0x07) > m_gdtr.limit)
					{
						logerror("CALL: Task Gate: TSS is past GDT limit.\n");
						FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
					}
				}
				if(desc.flags & 0x0002)
				{
					logerror("CALL: Task Gate: TSS is busy.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("CALL: Task Gate: TSS is not present.\n");
					FAULT(FAULT_NP,gate.selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);  // with nesting
				else
					i286_task_switch(desc.selector,1);
				return;
			default:
				logerror("CALL: Invalid special segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,selector & ~0x07)  // #GP(selector)
			}
		}
	}

	if(SetRPL != 0)
		selector = (selector & ~0x03) | m_CPL;

	uint32_t tempSP = REG32(ESP);
	try
	{
		// this is ugly but the alternative is worse
		if(operand32 == 0)
		{
			/* 16-bit operand size */
			PUSH16(m_sreg[CS].selector );
			PUSH16(m_eip & 0x0000ffff );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS);
		}
		else
		{
			/* 32-bit operand size */
			PUSH32SEG(m_sreg[CS].selector );
			PUSH32(m_eip );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS );
		}
	}
	catch(uint64_t e)
	{
		REG32(ESP) = tempSP;
		throw e;
	}

	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_retf(uint8_t count, uint8_t operand32)
{
	uint32_t newCS, newEIP;
	I386_SREG desc;
	uint8_t CPL, RPL, DPL;

	uint32_t ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);

	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
	}

	memset(&desc, 0, sizeof(desc));
	desc.selector = newCS;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = newCS & 0x03;

	if(RPL < CPL)
	{
		logerror("RETF (%08x): Return segment RPL is less than CPL.\n",m_pc);
		FAULT(FAULT_GP,newCS & ~0x03)
	}

	if(RPL == CPL)
	{
		/* same privilege level */
		if((newCS & ~0x03) == 0)
		{
			logerror("RETF: Return segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				logerror("RETF: Return segment is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				logerror("RETF: Return segment is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			logerror("RETF: Return segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				logerror("RETF: Conforming code segment DPL is greater than CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				logerror("RETF: Non-conforming code segment DPL does not equal CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			logerror("RETF: EIP is past code segment limit.\n");
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+3) != 0)
			{
				logerror("RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				logerror("RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		if(STACK_32BIT)
			REG32(ESP) += (operand32 ? 8 : 4) + count;
		else
			REG16(SP) +=  (operand32 ? 8 : 4) + count;
	}
	else if(RPL > CPL)
	{
		uint32_t newSS, newESP;  // when changing privilege
		/* outer privilege level */
		if(operand32 == 0)
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				logerror("RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+15) != 0)
			{
				logerror("RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		/* Check CS selector and descriptor */
		if((newCS & ~0x03) == 0)
		{
			logerror("RETF: CS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				logerror("RETF: CS segment selector is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				logerror("RETF: CS segment selector is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			logerror("RETF: CS segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				logerror("RETF: Conforming CS segment DPL is greater than return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				logerror("RETF: Non-conforming CS segment DPL is not equal to return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF: CS segment is not present.\n");
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			logerror("RETF: EIP is past return CS segment limit.\n");
			FAULT(FAULT_GP,0)
		}

		if(operand32 == 0)
		{
			ea += count+4;
			newESP = READ16(ea) & 0xffff;
			newSS = READ16(ea+2) & 0xffff;
		}
		else
		{
			ea += count+8;
			newESP = READ32(ea);
			newSS = READ32(ea+4) & 0xffff;
		}

		/* Check SS selector and descriptor */
		desc.selector = newSS;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
		if((newSS & ~0x07) == 0)
		{
			logerror("RETF: SS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newSS & 0x04)
		{
			if((newSS & ~0x07) > m_ldtr.limit)
			{
				logerror("RETF (%08x): SS segment selector is past LDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		else
		{
			if((newSS & ~0x07) > m_gdtr.limit)
			{
				logerror("RETF (%08x): SS segment selector is past GDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		if((newSS & 0x03) != RPL)
		{
			logerror("RETF: SS segment RPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0018) != 0x0010 || (desc.flags & 0x0002) == 0)
		{
			logerror("RETF: SS segment is not a writable data segment.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if(((desc.flags >> 5) & 0x03) != RPL)
		{
			logerror("RETF: SS DPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("RETF: SS segment is not present.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		m_CPL = newCS & 0x03;

		/* Load new SS:(E)SP */
		if(operand32 == 0)
			REG16(SP) = (newESP+count) & 0xffff;
		else
			REG32(ESP) = newESP+count;
		m_sreg[SS].selector = newSS;
		i386_load_segment_descriptor(SS );

		/* Check that DS, ES, FS and GS are valid for the new privilege level */
		i386_check_sreg_validity(DS);
		i386_check_sreg_validity(ES);
		i386_check_sreg_validity(FS);
		i386_check_sreg_validity(GS);
	}

	/* Load new CS:(E)IP */
	if(operand32 == 0)
		m_eip = newEIP & 0xffff;
	else
		m_eip = newEIP;
	m_sreg[CS].selector = newCS;
	i386_load_segment_descriptor(CS );
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_iret(int operand32)
{
	uint32_t newCS, newEIP;
	uint32_t newSS, newESP;  // when changing privilege
	I386_SREG desc,stack;
	uint8_t CPL, RPL, DPL;
	uint32_t newflags;
	uint8_t IOPL = m_IOP1 | (m_IOP2 << 1);

	CPL = m_CPL;
	uint32_t ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);
	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
		newflags = READ16(ea+4) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
		newflags = READ32(ea+8);
	}

	if(V8086_MODE)
	{
		uint32_t oldflags = get_flags();
		if(IOPL != 3)
		{
			logerror("IRET (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			m_eip = newEIP & 0xffff;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= (((oldflags>>12)&3)<<12);  // IOPL cannot be changed in V86 mode
			set_flags((newflags & 0xffff) | (oldflags & ~0xffff));
			REG16(SP) += 6;
		}
		else
		{
			m_eip = newEIP;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= 0x20000 | (((oldflags>>12)&3)<<12);  // IOPL and VM cannot be changed in V86 mode
			set_flags(newflags);
			REG32(ESP) += 12;
		}
	}
	else if(NESTED_TASK)
	{
		uint32_t task = READ32(m_task.base);
		/* Task Return */
		logerror("IRET (%08x): Nested task return.\n",m_pc);
		/* Check back-link selector in TSS */
		if(task & 0x04)
		{
			logerror("IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((task & ~0x07) >= m_gdtr.limit)
		{
			logerror("IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		memset(&desc, 0, sizeof(desc));
		desc.selector = task;
		i386_load_protected_mode_segment(&desc,nullptr);
		if((desc.flags & 0x001f) != 0x000b)
		{
			logerror("IRET (%08x): Task return: Back-linked TSS is not a busy TSS.\n",m_pc);
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			logerror("IRET: Task return: Back-linked TSS is not present.\n");
			FAULT(FAULT_NP,task & ~0x03)
		}
		if(desc.flags & 0x08)
			i386_task_switch(desc.selector,0);
		else
			i286_task_switch(desc.selector,0);
		return;
	}
	else
	{
		if(newflags & 0x00020000) // if returning to virtual 8086 mode
		{
			// 16-bit iret can't reach here
			newESP = READ32(ea+12);
			newSS = READ32(ea+16) & 0xffff;
			/* Return to v86 mode */
			//logerror("IRET (%08x): Returning to Virtual 8086 mode.\n",m_pc);
			if(CPL != 0)
			{
				uint32_t oldflags = get_flags();
				newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
				if(CPL > IOPL)
					newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
			}
			set_flags(newflags);
			m_eip = POP32() & 0xffff;  // high 16 bits are ignored
			m_sreg[CS].selector = POP32() & 0xffff;
			POP32();  // already set flags
			newESP = POP32();
			newSS = POP32() & 0xffff;
			m_sreg[ES].selector = POP32() & 0xffff;
			m_sreg[DS].selector = POP32() & 0xffff;
			m_sreg[FS].selector = POP32() & 0xffff;
			m_sreg[GS].selector = POP32() & 0xffff;
			REG32(ESP) = newESP;  // all 32 bits are loaded
			m_sreg[SS].selector = newSS;
			i386_load_segment_descriptor(ES);
			i386_load_segment_descriptor(DS);
			i386_load_segment_descriptor(FS);
			i386_load_segment_descriptor(GS);
			i386_load_segment_descriptor(SS);
			m_CPL = 3;  // Virtual 8086 tasks are always run at CPL 3
		}
		else
		{
			if(operand32 == 0)
			{
				uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+3) != 0)
				{
					logerror("IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			else
			{
				uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+7) != 0)
				{
					logerror("IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			RPL = newCS & 0x03;
			if(RPL < CPL)
			{
				logerror("IRET (%08x): Return CS RPL is less than CPL.\n",m_pc);
				FAULT(FAULT_GP,newCS & ~0x03)
			}
			if(RPL == CPL)
			{
				/* return to same privilege level */
				if(operand32 == 0)
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+5) != 0)
					{
						logerror("IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+11) != 0)
					{
						logerror("IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				if((newCS & ~0x03) == 0)
				{
					logerror("IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return CS selector (%04x) is past LDT limit.\n",newCS);
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if((desc.flags & 0x0018) != 0x0018)
				{
					logerror("IRET (%08x): Return CS segment is not a code segment.\n",m_pc);
					FAULT(FAULT_GP,newCS & ~0x07)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						logerror("IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						logerror("IRET: Non-conforming return CS DPL is not equal to CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("IRET: (%08x) Return CS segment is not present.\n", m_pc);
					FAULT(FAULT_NP,newCS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					logerror("IRET: Return EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

				if(CPL != 0)
				{
					uint32_t oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) += 6;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) += 12;
				}
			}
			else if(RPL > CPL)
			{
				/* return to outer privilege level */
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if(operand32 == 0)
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+9) != 0)
					{
						logerror("IRET: SP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+19) != 0)
					{
						logerror("IRET: ESP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				/* Check CS selector and descriptor */
				if((newCS & ~0x03) == 0)
				{
					logerror("IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return CS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				if((desc.flags & 0x0018) != 0x0018)
				{
					logerror("IRET: Return CS segment is not a code segment.\n");
					FAULT(FAULT_GP,newCS & ~0x03)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						logerror("IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						logerror("IRET: Non-conforming return CS DPL does not equal CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					logerror("IRET: Return CS segment is not present.\n");
					FAULT(FAULT_NP,newCS & ~0x03)
				}

				/* Check SS selector and descriptor */
				if(operand32 == 0)
				{
					newESP = READ16(ea+6) & 0xffff;
					newSS = READ16(ea+8) & 0xffff;
				}
				else
				{
					newESP = READ32(ea+12);
					newSS = READ32(ea+16) & 0xffff;
				}
				memset(&stack, 0, sizeof(stack));
				stack.selector = newSS;
				i386_load_protected_mode_segment(&stack,nullptr);
				DPL = (stack.flags >> 5) & 0x03;
				if((newSS & ~0x03) == 0)
				{
					logerror("IRET: Return SS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newSS & 0x04)
				{
					if((newSS & ~0x07) >= m_ldtr.limit)
					{
						logerror("IRET: Return SS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				else
				{
					if((newSS & ~0x07) >= m_gdtr.limit)
					{
						logerror("IRET: Return SS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				if((newSS & 0x03) != RPL)
				{
					logerror("IRET: Return SS RPL is not equal to return CS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0018) != 0x0010)
				{
					logerror("IRET: Return SS segment is not a data segment.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0002) == 0)
				{
					logerror("IRET: Return SS segment is not writable.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if(DPL != RPL)
				{
					logerror("IRET: Return SS DPL does not equal SS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0080) == 0)
				{
					logerror("IRET: Return SS segment is not present.\n");
					FAULT(FAULT_NP,newSS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					logerror("IRET: EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

//              if(operand32 == 0)
//                  REG16(SP) += 10;
//              else
//                  REG32(ESP) += 20;

				// IOPL can only change if CPL is zero
				if(CPL != 0)
				{
					uint32_t oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP & 0xffff;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) = newESP & 0xffff;
					m_sreg[SS].selector = newSS;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) = newESP;
					m_sreg[SS].selector = newSS & 0xffff;
				}
				m_CPL = newCS & 0x03;
				i386_load_segment_descriptor(SS);

				/* Check that DS, ES, FS and GS are valid for the new privilege level */
				i386_check_sreg_validity(DS);
				i386_check_sreg_validity(ES);
				i386_check_sreg_validity(FS);
				i386_check_sreg_validity(GS);
			}
		}
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

void i386_device::build_cycle_table()
{
	int i, j;
	for (j=0; j < X86_NUM_CPUS; j++)
	{
		cycle_table_rm[j] = std::make_unique<uint8_t[]>(CYCLES_NUM_OPCODES);
		cycle_table_pm[j] = std::make_unique<uint8_t[]>(CYCLES_NUM_OPCODES);

		for (i=0; i < sizeof(x86_cycle_table)/sizeof(X86_CYCLE_TABLE); i++)
		{
			int opcode = x86_cycle_table[i].op;
			cycle_table_rm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][0];
			cycle_table_pm[j][opcode] = x86_cycle_table[i].cpu_cycles[j][1];
		}
	}
}

void i386_device::report_invalid_opcode()
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid opcode %02X at %08X %s\n", m_opcode, m_pc - 1, m_lock ? "with lock" : "");
#else
	logerror("i386: Invalid opcode");
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X\n", m_opcode_pc);
#endif
}

void i386_device::report_invalid_modrm(const char* opcode, uint8_t modrm)
{
#ifndef DEBUG_MISSING_OPCODE
	logerror("i386: Invalid %s modrm %01X at %08X\n", opcode, modrm, m_pc - 2);
#else
	logerror("i386: Invalid %s modrm %01X", opcode, modrm);
	for (int a = 0; a < m_opcode_bytes_length; a++)
		logerror(" %02X", m_opcode_bytes[a]);
	logerror(" at %08X\n", m_opcode_pc);
#endif
	i386_trap(6, 0, 0);
}


#include "i386ops.hxx"
#include "i386op16.hxx"
#include "i386op32.hxx"
#include "i486ops.hxx"
#include "pentops.hxx"
#include "x87ops.hxx"
#include "cpuidmsrs.hxx"


void i386_device::i386_decode_opcode()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[0][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table1_32[m_opcode])();
	else
		(this->*m_opcode_table1_16[m_opcode])();
}

/* Two-byte opcode 0f xx */
void i386_device::i386_decode_two_byte()
{
	m_opcode = FETCH();

	if(m_lock && !m_lock_table[1][m_opcode])
		return i386_invalid();

	if( m_operand_size )
		(this->*m_opcode_table2_32[m_opcode])();
	else
		(this->*m_opcode_table2_16[m_opcode])();
}

/* Three-byte opcode 0f 38 xx */
void i386_device::i386_decode_three_byte38()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table338_32[m_opcode])();
	else
		(this->*m_opcode_table338_16[m_opcode])();
}

/* Three-byte opcode 0f 3a xx */
void i386_device::i386_decode_three_byte3a()
{
	m_opcode = FETCH();

	if (m_operand_size)
		(this->*m_opcode_table33a_32[m_opcode])();
	else
		(this->*m_opcode_table33a_16[m_opcode])();
}

/* Three-byte opcode prefix 66 0f xx */
void i386_device::i386_decode_three_byte66()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table366_32[m_opcode])();
	else
		(this->*m_opcode_table366_16[m_opcode])();
}

/* Three-byte opcode prefix f2 0f xx */
void i386_device::i386_decode_three_bytef2()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f2_32[m_opcode])();
	else
		(this->*m_opcode_table3f2_16[m_opcode])();
}

/* Three-byte opcode prefix f3 0f */
void i386_device::i386_decode_three_bytef3()
{
	m_opcode = FETCH();
	if( m_operand_size )
		(this->*m_opcode_table3f3_32[m_opcode])();
	else
		(this->*m_opcode_table3f3_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 38 xx */
void i386_device::i386_decode_four_byte3866()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table46638_32[m_opcode])();
	else
		(this->*m_opcode_table46638_16[m_opcode])();
}

/* Four-byte opcode prefix 66 0f 3a xx */
void i386_device::i386_decode_four_byte3a66()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4663a_32[m_opcode])();
	else
		(this->*m_opcode_table4663a_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 38 xx */
void i386_device::i386_decode_four_byte38f2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f238_32[m_opcode])();
	else
		(this->*m_opcode_table4f238_16[m_opcode])();
}

/* Four-byte opcode prefix f2 0f 3a xx */
void i386_device::i386_decode_four_byte3af2()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f23a_32[m_opcode])();
	else
		(this->*m_opcode_table4f23a_16[m_opcode])();
}

/* Four-byte opcode prefix f3 0f 38 xx */
void i386_device::i386_decode_four_byte38f3()
{
	m_opcode = FETCH();
	if (m_operand_size)
		(this->*m_opcode_table4f338_32[m_opcode])();
	else
		(this->*m_opcode_table4f338_16[m_opcode])();
}


/*************************************************************************/

uint8_t i386_device::read8_debug(uint32_t ea, uint8_t *data)
{
	uint32_t address = ea;

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&address,nullptr))
		return 0;

	address &= m_a20_mask;
	*data = m_program->read_byte(address);
	return 1;
}

uint32_t i386_device::i386_get_debug_desc(I386_SREG *seg)
{
	uint32_t base, limit, address;
	union { uint8_t b[8]; uint32_t w[2]; } data;
	uint8_t ret;
	int entry;

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	address = entry + base;

	// todo: bigendian
	ret = read8_debug( address+0, &data.b[0] );
	ret += read8_debug( address+1, &data.b[1] );
	ret += read8_debug( address+2, &data.b[2] );
	ret += read8_debug( address+3, &data.b[3] );
	ret += read8_debug( address+4, &data.b[4] );
	ret += read8_debug( address+5, &data.b[5] );
	ret += read8_debug( address+6, &data.b[6] );
	ret += read8_debug( address+7, &data.b[7] );

	if(ret != 8)
		return 0;

	seg->flags = (data.w[1] >> 8) & 0xf0ff;
	seg->base = (data.w[1] & 0xff000000) | ((data.w[1] & 0xff) << 16) | ((data.w[0] >> 16) & 0xffff);
	seg->limit = (data.w[1] & 0xf0000) | (data.w[0] & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = (seg->selector & ~3)?(true):(false);

	return seg->valid;
}

uint64_t i386_device::debug_segbase(symbol_table &table, int params, const uint64_t *param)
{
	uint32_t result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.base;
	}
	else
	{
		result = param[0] << 4;
	}
	return result;
}

uint64_t i386_device::debug_seglimit(symbol_table &table, int params, const uint64_t *param)
{
	uint32_t result = 0;
	I386_SREG seg;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		result = seg.limit;
	}
	return result;
}

uint64_t i386_device::debug_segofftovirt(symbol_table &table, int params, const uint64_t *param)
{
	uint32_t result;
	I386_SREG seg;

	if(param[0] > 65535)
		return 0;

	if (PROTECTED_MODE && !V8086_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = param[0];
		if(!i386_get_debug_desc(&seg))
			return 0;
		if((seg.flags & 0x0090) != 0x0090) // not system and present
			return 0;
		if((seg.flags & 0x0018) == 0x0010 && seg.flags & 0x0004) // expand down
		{
			if(param[1] <= seg.limit)
				return 0;
		}
		else
		{
			if(param[1] > seg.limit)
				return 0;
		}
		result = seg.base+param[1];
	}
	else
	{
		if(param[1] > 65535)
			return 0;

		result = (param[0] << 4) + param[1];
	}
	return result;
}

uint64_t i386_device::debug_virttophys(symbol_table &table, int params, const uint64_t *param)
{
	uint32_t result = param[0];

	if(!i386_translate_address(TRANSLATE_DEBUG_MASK,&result,nullptr))
		return 0;
	return result;
}

void i386_device::device_debug_setup()
{
	using namespace std::placeholders;
	debug()->symtable().add("segbase", 1, 1, std::bind(&i386_device::debug_segbase, this, _1, _2, _3));
	debug()->symtable().add("seglimit", 1, 1, std::bind(&i386_device::debug_seglimit, this, _1, _2, _3));
	debug()->symtable().add("segofftovirt", 2, 2, std::bind(&i386_device::debug_segofftovirt, this, _1, _2, _3));
	debug()->symtable().add("virttophys", 1, 1, std::bind(&i386_device::debug_virttophys, this, _1, _2, _3));
}

/*************************************************************************/

void i386_device::i386_postload()
{
	int i;
	for (i = 0; i < 6; i++)
		i386_load_segment_descriptor(i);
	CHANGE_PC(m_eip);
}

void i386_device::i386_common_init()
{
	DEVICE::initialize();
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};

	assert((sizeof(XMM_REG)/sizeof(double)) == 2);

	build_cycle_table();

	for( i=0; i < 256; i++ ) {
		int c=0;
		for( j=0; j < 8; j++ ) {
			if( i & (1 << j) )
				c++;
		}
		i386_parity_table[i] = ~(c & 0x1) & 0x1;
	}

	for( i=0; i < 256; i++ ) {
		i386_MODRM_table[i].reg.b = regs8[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.w = regs16[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.d = regs32[(i >> 3) & 0x7];

		i386_MODRM_table[i].rm.b = regs8[i & 0x7];
		i386_MODRM_table[i].rm.w = regs16[i & 0x7];
		i386_MODRM_table[i].rm.d = regs32[i & 0x7];
	}

	m_program = &space(AS_PROGRAM);
	if(data_width == 16) {
		// for the 386sx
		macache16 = m_program->cache<1, 0, ENDIANNESS_LITTLE>();
	} else {
		macache32 = m_program->cache<2, 0, ENDIANNESS_LITTLE>();
	}

	m_io = &space(AS_IO);
	m_smi = false;
	m_debugger_temp = 0;
	m_lock = false;

	zero_state();


	//m_smiact.resolve_safe();
	//m_ferr_handler.resolve_safe();
	m_ferr_err_value =0;

	if((d_debugger != NULL) && (osd->check_feature(USE_DEBUGGER))) {
		d_program_stored = d_mem;
		d_io_stored = d_io;
	} else {
		d_debugger = NULL;
	}
	set_icountptr(m_cycles);
}

void i386_device::initialize()
{
	i386_common_init();

	build_opcode_table(OP_I386);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I386].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I386].get();

//	register_state_i386();
}

#define STATE_VERSION 6

bool i386_device::process_state_segment(int num, FILEIO* state_fio, bool loading)
{
	state_fio->StateValue(m_sreg[num].selector);
	state_fio->StateValue(m_sreg[num].base);
	state_fio->StateValue(m_sreg[num].limit);
	state_fio->StateValue(m_sreg[num].flags);

	return true;
}

bool i386_device::process_state_i386(FILEIO* state_fio, bool loading)
{
	state_fio->StateValue(m_pc);
	state_fio->StateValue(m_eip);
	state_fio->StateValue(m_prev_eip);
	state_fio->StateValue(m_reg.d[EAX]); 
	state_fio->StateValue(m_reg.d[EBX]); 
	state_fio->StateValue(m_reg.d[ECX]); 
	state_fio->StateValue(m_reg.d[EDX]); 
	state_fio->StateValue(m_reg.d[EBP]); 
	state_fio->StateValue(m_reg.d[ESP]); 
	state_fio->StateValue(m_reg.d[ESI]); 
	state_fio->StateValue(m_reg.d[EDI]);
	
	state_fio->StateValue(m_eflags);
	state_fio->StateValue(m_eflags_mask);
	
	process_state_segment(CS, state_fio, loading); 
	process_state_segment(SS, state_fio, loading); 
	process_state_segment(DS, state_fio, loading); 
	process_state_segment(ES, state_fio, loading); 
	process_state_segment(FS, state_fio, loading); 
	process_state_segment(GS, state_fio, loading);
	state_fio->StateArray(m_cr, sizeof(m_cr), 1);
	state_fio->StateArray(m_dr, sizeof(m_dr), 1);
	state_fio->StateArray(m_tr, sizeof(m_tr), 1);

	state_fio->StateValue(m_gdtr.base);
	state_fio->StateValue(m_gdtr.limit);
	
	state_fio->StateValue(m_idtr.base);
	state_fio->StateValue(m_idtr.limit);
	
	state_fio->StateValue(m_ldtr.segment);
	state_fio->StateValue(m_ldtr.base);
	state_fio->StateValue(m_ldtr.limit);
	state_fio->StateValue(m_ldtr.flags);
	
	state_fio->StateValue(m_task.segment);
	state_fio->StateValue(m_task.base);
	state_fio->StateValue(m_task.limit);
	state_fio->StateValue(m_task.flags);

	state_fio->StateValue(m_CF);
	state_fio->StateValue(m_DF);
	state_fio->StateValue(m_SF);
	state_fio->StateValue(m_OF);
	state_fio->StateValue(m_ZF);
	state_fio->StateValue(m_PF);
	state_fio->StateValue(m_AF);
	state_fio->StateValue(m_IF);
	state_fio->StateValue(m_TF);
	state_fio->StateValue(m_IOP1);
	state_fio->StateValue(m_IOP2);
	state_fio->StateValue(m_NT);
	state_fio->StateValue(m_RF);
	state_fio->StateValue(m_VM);
	state_fio->StateValue(m_AC);
	state_fio->StateValue(m_VIF);
	state_fio->StateValue(m_VIP);
	state_fio->StateValue(m_ID);
	
	state_fio->StateValue(m_CPL);
	
	state_fio->StateValue(m_performed_intersegment_jump);
	
	state_fio->StateValue(m_segment_override);
	state_fio->StateValue(m_irq_state);
	state_fio->StateValue(m_a20_mask);
	state_fio->StateValue(m_shutdown);
	state_fio->StateValue(m_halted);
	state_fio->StateValue(m_busreq);
	
	state_fio->StateValue(m_cycles);
	state_fio->StateValue(extra_cycles);
	state_fio->StateValue(total_cycles);
	state_fio->StateValue(m_tsc);
						  
	state_fio->StateValue(m_mxcsr);
	state_fio->StateValue(m_smm);
	state_fio->StateValue(m_smi);
	state_fio->StateValue(m_smi_latched);
	state_fio->StateValue(m_nmi_masked);
	state_fio->StateValue(m_nmi_latched);
	state_fio->StateValue(m_smbase);
	state_fio->StateValue(m_lock);
	state_fio->StateValue(m_ferr_err_value);
	state_fio->StateValue(m_smiact_enabled);
	
	state_fio->StateValue(m_operand_size);

	state_fio->StateValue(icount);
	if(loading) {
		i386_postload();
		prev_total_cycles = total_cycles;
	}

	//m_smiact.resolve_safe();
	//m_ferr_handler.resolve_safe();

//	state_add( STATE_GENPC, "GENPC", m_pc).noshow();
//	state_add( STATE_GENPCBASE, "CURPC", m_pc).noshow();
//	state_add( STATE_GENFLAGS, "GENFLAGS", m_debugger_temp).formatstr("%8s").noshow();
//	state_add( STATE_GENSP, "GENSP", REG32(ESP)).noshow();
	return true;
}

bool i386_device::process_state_i386_x87(FILEIO* state_fio, bool loading)
{
	if(!process_state_i386(state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(m_x87_cw);
	state_fio->StateValue(m_x87_sw);
	state_fio->StateValue(m_x87_tw);
	state_fio->StateValue(m_x87_data_ptr);
	state_fio->StateValue(m_x87_inst_ptr);
	state_fio->StateValue(m_x87_opcode);
	state_fio->StateValue(m_x87_operand_size);
	for(int i = 0; i < 8; i++) {
		state_fio->StateValue(m_x87_reg[i].high);
		state_fio->StateValue(m_x87_reg[i].low);
	}
	return true;
}

bool i386_device::process_state_i386_x87_xmm(FILEIO* state_fio, bool loading)
{
	if(!process_state_i386_x87(state_fio, loading)) {
		return false;
	}
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 4; j++) {
			state_fio->StateValue(m_sse_reg[i].d[j]);
		}
	}
	state_fio->StateValue(m_xmm_operand_size);
	return true;
}

bool i386_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386(state_fio, loading);
}

bool i486_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool pentium_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool mediagx_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool pentium_pro_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool pentium_mmx_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool pentium2_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87(state_fio, loading);
}

bool pentium3_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87_xmm(state_fio, loading);
}

bool pentium4_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87_xmm(state_fio, loading);
}

#if 0
bool athlonxp_device::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return process_state_i386_x87_xmm(state_fio, loading);
}
#endif
void i386_device::state_import(int index)
{
	switch (index)
	{
		case I386_EIP:
			CHANGE_PC(m_eip);
			break;
		case I386_IP:
			m_eip = ( m_eip & ~0xffff ) | ( m_debugger_temp & 0xffff);
			CHANGE_PC(m_eip);
			break;
		case I386_CS:
			i386_load_segment_descriptor(CS);
			break;
		case I386_SS:
			i386_load_segment_descriptor(SS);
			break;
		case I386_DS:
			i386_load_segment_descriptor(DS);
			break;
		case I386_ES:
			i386_load_segment_descriptor(ES);
			break;
		case I386_FS:
			i386_load_segment_descriptor(FS);
			break;
		case I386_GS:
			i386_load_segment_descriptor(GS);
			break;
	}
}

void i386_device::state_export(int index)
{
	switch (index())
	{
		case I386_IP:
			m_debugger_temp = m_eip & 0xffff;
			break;
	}
}

void i386_device::build_opcode_table(uint32_t features)
{
	int i;
	for (i=0; i < 256; i++)
	{
		m_opcode_table1_16[i] = &i386_device::i386_invalid;
		m_opcode_table1_32[i] = &i386_device::i386_invalid;
		m_opcode_table2_16[i] = &i386_device::i386_invalid;
		m_opcode_table2_32[i] = &i386_device::i386_invalid;
		m_opcode_table366_16[i] = &i386_device::i386_invalid;
		m_opcode_table366_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f2_32[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_16[i] = &i386_device::i386_invalid;
		m_opcode_table3f3_32[i] = &i386_device::i386_invalid;
		m_lock_table[0][i] = false;
		m_lock_table[1][i] = false;
	}

	for (i=0; i < sizeof(s_x86_opcode_table)/sizeof(X86_OPCODE); i++)
	{
		const X86_OPCODE *op = &s_x86_opcode_table[i];

		if ((op->flags & features))
		{
			if (op->flags & OP_2BYTE)
			{
				m_opcode_table2_32[op->opcode] = op->handler32;
				m_opcode_table2_16[op->opcode] = op->handler16;
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
				m_lock_table[1][op->opcode] = op->lockable;
			}
			else if (op->flags & OP_3BYTE66)
			{
				m_opcode_table366_32[op->opcode] = op->handler32;
				m_opcode_table366_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF2)
			{
				m_opcode_table3f2_32[op->opcode] = op->handler32;
				m_opcode_table3f2_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTEF3)
			{
				m_opcode_table3f3_32[op->opcode] = op->handler32;
				m_opcode_table3f3_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE38)
			{
				m_opcode_table338_32[op->opcode] = op->handler32;
				m_opcode_table338_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_3BYTE3A)
			{
				m_opcode_table33a_32[op->opcode] = op->handler32;
				m_opcode_table33a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3866)
			{
				m_opcode_table46638_32[op->opcode] = op->handler32;
				m_opcode_table46638_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3A66)
			{
				m_opcode_table4663a_32[op->opcode] = op->handler32;
				m_opcode_table4663a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F2)
			{
				m_opcode_table4f238_32[op->opcode] = op->handler32;
				m_opcode_table4f238_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE3AF2)
			{
				m_opcode_table4f23a_32[op->opcode] = op->handler32;
				m_opcode_table4f23a_16[op->opcode] = op->handler16;
			}
			else if (op->flags & OP_4BYTE38F3)
			{
				m_opcode_table4f338_32[op->opcode] = op->handler32;
				m_opcode_table4f338_16[op->opcode] = op->handler16;
			}
			else
			{
				m_opcode_table1_32[op->opcode] = op->handler32;
				m_opcode_table1_16[op->opcode] = op->handler16;
				m_lock_table[0][op->opcode] = op->lockable;
			}
		}
	}
}

void i386_device::zero_state()
{
	memset( &m_reg, 0, sizeof(m_reg) );
	memset( m_sreg, 0, sizeof(m_sreg) );
	m_eip = 0;
	m_pc = 0;
	m_prev_eip = 0;
	m_eflags = 0;
	m_eflags_mask = 0;
	m_CF = 0;
	m_DF = 0;
	m_SF = 0;
	m_OF = 0;
	m_ZF = 0;
	m_PF = 0;
	m_AF = 0;
	m_IF = 0;
	m_TF = 0;
	m_IOP1 = 0;
	m_IOP2 = 0;
	m_NT = 0;
	m_RF = 0;
	m_VM = 0;
	m_AC = 0;
	m_VIF = 0;
	m_VIP = 0;
	m_ID = 0;
	m_CPL = 0;
	m_performed_intersegment_jump = 0;
	m_delayed_interrupt_enable = 0;
	memset( m_cr, 0, sizeof(m_cr) );
	memset( m_dr, 0, sizeof(m_dr) );
	memset( m_tr, 0, sizeof(m_tr) );
	memset( &m_gdtr, 0, sizeof(m_gdtr) );
	memset( &m_idtr, 0, sizeof(m_idtr) );
	memset( &m_task, 0, sizeof(m_task) );
	memset( &m_ldtr, 0, sizeof(m_ldtr) );
	m_ext = 0;
	m_halted = 0;
	m_busreq = 0;
	m_operand_size = 0;
	m_xmm_operand_size = 0;
	m_address_size = 0;
	m_operand_prefix = 0;
	m_address_prefix = 0;
	m_segment_prefix = 0;
	m_segment_override = 0;
	m_cycles = 0;
	m_base_cycles = 0;
	m_opcode = 0;
	m_irq_state = 0;
	m_a20_mask = 0;
	m_shutdown = false;
	m_cpuid_max_input_value_eax = 0;
	m_cpuid_id0 = 0;
	m_cpuid_id1 = 0;
	m_cpuid_id2 = 0;
	m_cpu_version = 0;
	m_feature_flags = 0;
	m_tsc = 0;
	total_cycles = 0;
	prev_total_cycles = 0;
	extra_cycles = 0;
	m_perfctr[0] = m_perfctr[1] = 0;
	memset( m_x87_reg, 0, sizeof(m_x87_reg) );
	m_x87_cw = 0;
	m_x87_sw = 0;
	m_x87_tw = 0;
	m_x87_data_ptr = 0;
	m_x87_inst_ptr = 0;
	m_x87_opcode = 0;
	memset( m_sse_reg, 0, sizeof(m_sse_reg) );
	m_mxcsr = 0;
	m_smm = false;
	m_smi = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;
	m_smbase = 0;
	memset( m_opcode_bytes, 0, sizeof(m_opcode_bytes) );
	m_opcode_pc = 0;
	m_opcode_bytes_length = 0;
}

void i386_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x93;
	m_sreg[CS].valid    = true;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	m_a20_mask = ~0;

	m_cr[0] = 0x7fffffe0; // reserved bits set to 1
	m_eflags = 0;
	m_eflags_mask = 0x00037fd7;
	m_eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);
	m_cpu_version = REG32(EDX);

	m_CPL = 0;

	CHANGE_PC(m_eip);
}

void i386_device::pentium_smi()
{
	uint32_t smram_state = m_smbase + 0xfe00;
	uint32_t old_cr0 = m_cr[0];
	uint32_t old_flags = get_flags();

	if(m_smm)
		return;

	m_cr[0] &= ~(0x8000000d);
	set_flags(2);
	if(m_smiact != NULL) 
		m_smiact_enabled = true;
	m_smm = true;
	m_smi_latched = false;

	// save state
	WRITE32(smram_state + SMRAM_SMBASE, m_smbase);
	WRITE32(smram_state + SMRAM_IP5_CR4, m_cr[4]);
	WRITE32(smram_state + SMRAM_IP5_ESLIM, m_sreg[ES].limit);
	WRITE32(smram_state + SMRAM_IP5_ESBASE, m_sreg[ES].base);
	WRITE32(smram_state + SMRAM_IP5_ESACC, m_sreg[ES].flags);
	WRITE32(smram_state + SMRAM_IP5_CSLIM, m_sreg[CS].limit);
	WRITE32(smram_state + SMRAM_IP5_CSBASE, m_sreg[CS].base);
	WRITE32(smram_state + SMRAM_IP5_CSACC, m_sreg[CS].flags);
	WRITE32(smram_state + SMRAM_IP5_SSLIM, m_sreg[SS].limit);
	WRITE32(smram_state + SMRAM_IP5_SSBASE, m_sreg[SS].base);
	WRITE32(smram_state + SMRAM_IP5_SSACC, m_sreg[SS].flags);
	WRITE32(smram_state + SMRAM_IP5_DSLIM, m_sreg[DS].limit);
	WRITE32(smram_state + SMRAM_IP5_DSBASE, m_sreg[DS].base);
	WRITE32(smram_state + SMRAM_IP5_DSACC, m_sreg[DS].flags);
	WRITE32(smram_state + SMRAM_IP5_FSLIM, m_sreg[FS].limit);
	WRITE32(smram_state + SMRAM_IP5_FSBASE, m_sreg[FS].base);
	WRITE32(smram_state + SMRAM_IP5_FSACC, m_sreg[FS].flags);
	WRITE32(smram_state + SMRAM_IP5_GSLIM, m_sreg[GS].limit);
	WRITE32(smram_state + SMRAM_IP5_GSBASE, m_sreg[GS].base);
	WRITE32(smram_state + SMRAM_IP5_GSACC, m_sreg[GS].flags);
	WRITE32(smram_state + SMRAM_IP5_LDTACC, m_ldtr.flags);
	WRITE32(smram_state + SMRAM_IP5_LDTLIM, m_ldtr.limit);
	WRITE32(smram_state + SMRAM_IP5_LDTBASE, m_ldtr.base);
	WRITE32(smram_state + SMRAM_IP5_GDTLIM, m_gdtr.limit);
	WRITE32(smram_state + SMRAM_IP5_GDTBASE, m_gdtr.base);
	WRITE32(smram_state + SMRAM_IP5_IDTLIM, m_idtr.limit);
	WRITE32(smram_state + SMRAM_IP5_IDTBASE, m_idtr.base);
	WRITE32(smram_state + SMRAM_IP5_TRLIM, m_task.limit);
	WRITE32(smram_state + SMRAM_IP5_TRBASE, m_task.base);
	WRITE32(smram_state + SMRAM_IP5_TRACC, m_task.flags);

	WRITE32(smram_state + SMRAM_ES, m_sreg[ES].selector);
	WRITE32(smram_state + SMRAM_CS, m_sreg[CS].selector);
	WRITE32(smram_state + SMRAM_SS, m_sreg[SS].selector);
	WRITE32(smram_state + SMRAM_DS, m_sreg[DS].selector);
	WRITE32(smram_state + SMRAM_FS, m_sreg[FS].selector);
	WRITE32(smram_state + SMRAM_GS, m_sreg[GS].selector);
	WRITE32(smram_state + SMRAM_LDTR, m_ldtr.segment);
	WRITE32(smram_state + SMRAM_TR, m_task.segment);

	WRITE32(smram_state + SMRAM_DR7, m_dr[7]);
	WRITE32(smram_state + SMRAM_DR6, m_dr[6]);
	WRITE32(smram_state + SMRAM_EAX, REG32(EAX));
	WRITE32(smram_state + SMRAM_ECX, REG32(ECX));
	WRITE32(smram_state + SMRAM_EDX, REG32(EDX));
	WRITE32(smram_state + SMRAM_EBX, REG32(EBX));
	WRITE32(smram_state + SMRAM_ESP, REG32(ESP));
	WRITE32(smram_state + SMRAM_EBP, REG32(EBP));
	WRITE32(smram_state + SMRAM_ESI, REG32(ESI));
	WRITE32(smram_state + SMRAM_EDI, REG32(EDI));
	WRITE32(smram_state + SMRAM_EIP, m_eip);
	WRITE32(smram_state + SMRAM_EFLAGS, old_flags);
	WRITE32(smram_state + SMRAM_CR3, m_cr[3]);
	WRITE32(smram_state + SMRAM_CR0, old_cr0);

	m_sreg[DS].selector = m_sreg[ES].selector = m_sreg[FS].selector = m_sreg[GS].selector = m_sreg[SS].selector = 0;
	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffffffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x8093;
	m_sreg[DS].valid = m_sreg[ES].valid = m_sreg[FS].valid = m_sreg[GS].valid = m_sreg[SS].valid =true;
	m_sreg[CS].selector = 0x3000; // pentium only, ppro sel = smbase >> 4
	m_sreg[CS].base = m_smbase;
	m_sreg[CS].limit = 0xffffffff;
	m_sreg[CS].flags = 0x8093;
	m_sreg[CS].valid = true;
	m_cr[4] = 0;
	m_dr[7] = 0x400;
	m_eip = 0x8000;

	m_nmi_masked = true;
	CHANGE_PC(m_eip);
}

void i386_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_A20 )
	{
		i386_set_a20_line( state );
		return;
	}
	if ( irqline == INPUT_LINE_NMI )
	{
		if ( state != CLEAR_LINE && m_halted)
		{
			m_halted = 0;
		}

		/* NMI (I do not think that this is 100% right) */
		if(m_nmi_masked)
		{
			m_nmi_latched = true;
			return;
		}
		if ( state )
			i386_trap(2, 1, 0);
	}
	else
	{
		if (irqline >= 0 && irqline <= MAX_INPUT_LINES)
		{
			if ( state != CLEAR_LINE && m_halted )
			{
				m_halted = 0;
			}

			m_irq_state = state;
		}
	}
}

void pentium_device::execute_set_input(int irqline, int state)
{
	if ( irqline == INPUT_LINE_SMI )
	{
		if ( !m_smi && state && m_smm )
		{
			m_smi_latched = true;
		}
		m_smi = state;
	}
	else
	{
		i386_device::execute_set_input(irqline, state);
	}
}

void i386_device::i386_set_a20_line(int state)
{
	if (state)
	{
		m_a20_mask = ~0;
	}
	else
	{
		m_a20_mask = ~(1 << 20);
	}
	// TODO: how does A20M and the tlb interact
	d_vtlb->vtlb_flush_dynamic();
}

void i386_device::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		i386_set_irq_line(INPUT_LINE_NMI, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_IRQ) {
		i386_set_irq_line(INPUT_LINE_IRQ, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_BUSREQ) {
		m_busreq = (data & mask) ? 1 : 0;
	} else if(id == SIG_I386_A20) {
		i386_set_a20_line(data & mask);
	}
}

int i386_device::run(int clocks)
{
	int cycles = m_cycles;
	m_base_cycles = cycles;
	CHANGE_PC(m_eip);

	if ((m_halted) || (m_busreq))
	{
		if(d_dma != NULL) {
			d_dma->do_dma();
		}
		if(clocks < 0) {
			int passed_cycles = max(1, extra_cycles);
			extra_cycles = 0;
			tsc += passed_cycles;
			total_cycles += passed_cycles;
			return passed_cycles;
		} else {
			m_cycles += clocks;
			m_cycles -= extra_cycles;
			extra_cycles = 0;
			/* if busreq is raised, spin cpu while remained clock */
			if(m_cycles > 0) {
				m_cycles = 0;
			}
			int passed_cycles = m_base_cycles - m_cycles;
			tsc += passed_cycles;
//#ifdef USE_DEBUGGER
			total_cycles += passed_cycles;
//#endif
			return passed_cycles;
		}
	}

	if(clocks < 0) {
		m_cycles = 1;
	} else {
		m_cycles += clocks;
	}
	m_base_cycles = m_cycles;
	total_cycles += extra_cycles;
	m_cycles -= extra_cycles;
	extra_cycles - 0;
	
	while( (m_cycles > 0) && !(m_busreq) )
	{
		bool now_debugging = false;
		if((d_debugger != NULL) && (d_emu != NULL)){
			now_debugging = d_debugger->now_debugging;
		}
		if(now_debugging) {
			d_debugger->check_break_points(m_pc);
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
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			int first_cycles = m_cycles;
			i386_check_irq_line();
			m_operand_size = m_sreg[CS].d;
			m_xmm_operand_size = 0;
			m_address_size = m_sreg[CS].d;
			m_operand_prefix = 0;
			m_address_prefix = 0;

			m_ext = 1;
			int old_tf = m_TF;

			d_debugger->add_cpu_trace(m_pc);
			m_segment_prefix = 0;
			m_prev_eip = m_eip;
			m_prev_pc = m_pc;

			if(m_delayed_interrupt_enable != 0)
			{
				m_IF = 1;
				m_delayed_interrupt_enable = 0;
			}
#ifdef DEBUG_MISSING_OPCODE
			m_opcode_bytes_length = 0;
			m_opcode_pc = m_pc;
#endif
			try
			{
				i386_decode_opcode();
				if(m_TF && old_tf)
				{
					m_prev_eip = m_eip;
					m_ext = 1;
					i386_trap(1,0,0);
				}
				if(m_lock && (m_opcode != 0xf0))
					m_lock = false;
			}
			catch(uint64_t e)
			{
				m_ext = 1;
				//logerror("Illegal instruction EIP=%08x VM8086=%s exception %08x irq=0 irq_gate=0 ERROR=%08x\n", cpustate->eip, (cpustate->VM) ? "YES" : "NO", e & 0xffffffff, e >> 32); 
				i386_trap_with_error(e&0xffffffff,0,0,e>>32, 1);
			}
			
//#ifdef SINGLE_MODE_DMA
			if(d_dma != NULL) {
				d_dma->do_dma();
			}
//#endif
			/* adjust for any interrupts that came in */
			m_cycles -= extra_cycles;
			extra_cycles = 0;
			total_cycles += first_cycles - m_cycles;
			
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_program_stored;
				d_io = d_io_stored;
			}
		} else {
			int first_cycles = m_cycles;

			i386_check_irq_line();
			m_operand_size = m_sreg[CS].d;
			m_xmm_operand_size = 0;
			m_address_size = m_sreg[CS].d;
			m_operand_prefix = 0;
			m_address_prefix = 0;

			m_ext = 1;
			int old_tf = m_TF;

			m_segment_prefix = 0;
			m_prev_eip = m_eip;

			debugger_instruction_hook(m_pc);

			if(m_delayed_interrupt_enable != 0)
			{
				m_IF = 1;
				m_delayed_interrupt_enable = 0;
			}
#ifdef DEBUG_MISSING_OPCODE
			m_opcode_bytes_length = 0;
			m_opcode_pc = m_pc;
#endif
			try
			{
				i386_decode_opcode();
				if(m_TF && old_tf)
				{
					m_prev_eip = m_eip;
					m_ext = 1;
					i386_trap(1,0,0);
				}
				if(m_lock && (m_opcode != 0xf0))
					m_lock = false;
			}
			catch(uint64_t e)
			{
				m_ext = 1;
				i386_trap_with_error(e&0xffffffff,0,0,e>>32);
			}
			if(d_dma != NULL) {
				d_dma->do_dma();
			}
		}
		if ((m_cycles > 0) && (m_busreq)) {
			total_cycles += m_cycles;
			m_cycles = 0;
		}
	}
	int passed_cycles = m_base_cycles - m_cycles;
	m_tsc += passed_cycles;
	return passed_cycles;
}

/*************************************************************************/

bool i386_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	bool ret = true;
	if(spacenum == AS_PROGRAM)
		ret = i386_translate_address(intention, &address, nullptr);
	address &= m_a20_mask;
	return ret;
}

void i386_device::set_intr_line(bool line, bool pending, uint32_t bit)
{
	i386_set_irq_line(INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void i386_device::set_extra_clock(int cycles)
{
	extra_cycles += cycles;
}

int i386_device::get_extra_clock()
{
	return extra_cycles;
}

uint32_t i386_device::get_pc()
{
	return m_prev_pc;
}

uint32_t i386_device::get_next_pc()
{
	return m_pc;
}

void i386_device::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

void i386_device::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data16w(addr, data, &wait);
}

void i386_device::write_debug_data32(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data32w(addr, data, &wait);
}

uint32_t i386_device::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

uint32_t i386_device::read_debug_data16(uint32_t addr)
{
	int wait;
	return d_mem->read_data16w(addr, &wait);
}

uint32_t i386_device::read_debug_data32(uint32_t addr)
{
	int wait;
	return d_mem->read_data32w(addr, &wait);
}

void i386_device::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t i386_device::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

void i386_device::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io16w(addr, data, &wait);
}

uint32_t i386_device::read_debug_io16(uint32_t addr) {
	int wait;
	return d_io->read_io16w(addr, &wait);
}

void i386_device::write_debug_io32(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io32w(addr, data, &wait);
}

uint32_t i386_device::read_debug_io32(uint32_t addr) {
	int wait;
	return d_io->read_io32w(addr, &wait);
}

int i386_device::get_mode() const
{
	return m_sreg[CS].d ? 32 : 16;
}

std::unique_ptr<util::disasm_interface> i386_device::create_disassembler()
{
	return std::make_unique<i386_disassembler>(this);
}

void i386_device::opcode_cpuid()
{
	logerror("CPUID called with unsupported EAX=%08x at %08x!\n", REG32(EAX), m_eip);
}

uint64_t i386_device::opcode_rdmsr(bool &valid_msr)
{
	valid_msr = false;
	logerror("RDMSR called with unsupported ECX=%08x at %08x!\n", REG32(ECX), m_eip);
	return -1;
}

void i386_device::opcode_wrmsr(uint64_t data, bool &valid_msr)
{
	valid_msr = false;
	logerror("WRMSR called with unsupported ECX=%08x (%08x%08x) at %08x!\n", REG32(ECX), (uint32_t)(data >> 32), (uint32_t)data, m_eip);
}

/*****************************************************************************/
/* Intel 486 */


void i486_device::initialize()
{
	i386_common_init();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_I486].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_I486].get();

//	register_state_i386_x87();
}

void i486_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0;
	m_eflags_mask = 0x00077fd7;
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4 (486), Model 0/1 (DX), Stepping 3
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (0 << 4) | (3);
	m_cpu_version = REG32(EDX);

	CHANGE_PC(m_eip);
}

void i486dx4_device::reset()
{
	i486_device::reset();
	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);
}

/*****************************************************************************/
/* Pentium */


void pentium_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87();

	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM);
	build_x87_opcode_table();
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();
}

void pentium_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x003f7fd7;
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5 (Pentium), Model 2 (75 - 200MHz), Stepping 5
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (2 << 4) | (5);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	m_feature_flags = 0x000001bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Cyrix MediaGX */


void mediagx_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_CYRIX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_MEDIAGX].get();
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_MEDIAGX].get();
}

void mediagx_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x00000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_smm = false;
	m_smi_latched = false;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4, Model 4 (MediaGX)
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (4 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x69727943;   // Cyri
	m_cpuid_id1 = 0x736e4978;   // xIns
	m_cpuid_id2 = 0x6d616574;   // tead

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium Pro */

void pentium_pro_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_pro_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 1 (Pentium Pro)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (1 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [15:15] CMOV and FCMOV
	// No MMX
	m_feature_flags = 0x000081bf;

	CHANGE_PC(m_eip);
}


/*****************************************************************************/
/* Intel Pentium MMX */

void pentium_mmx_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium_mmx_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5, Model 4 (P55C)
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (4 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [23:23] MMX instructions
	m_feature_flags = 0x008001bf;

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium II */

void pentium2_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium2_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 3 (Pentium II / Klamath)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (3 << 4) | (1); /* TODO: is this correct? */

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x008081bf;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

/*****************************************************************************/
/* Intel Pentium III */

void pentium3_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium3_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 8 (Pentium III / Coppermine)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (10);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x03;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 4:4] Time Stamp Counter
	// [ D:D] PTE Global Bit
	m_feature_flags = 0x00002011;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}
#if 0
/*****************************************************************************/
/* AMD Athlon XP
   Model: Athlon XP 2400+
   Part number: AXDA2400DKV3C
   Stepping code: AIUCP
   Date code: 0240MPMW
*/

void athlonxp_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void athlonxp_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base = 0xffff0000;
	m_sreg[CS].limit = 0xffff;
	m_sreg[CS].flags = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 8, Stepping 1
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (1);

	m_cpuid_id0 = ('h' << 24) | ('t' << 16) | ('u' << 8) | 'A';   // Auth
	m_cpuid_id1 = ('i' << 24) | ('t' << 16) | ('n' << 8) | 'e';   // enti
	m_cpuid_id2 = ('D' << 24) | ('M' << 16) | ('A' << 8) | 'c';   // cAMD
	memset(m_processor_name_string, 0, 48);
	strcpy((char *)m_processor_name_string, "AMD Athlon(tm) Processor");
	for (int n = 0; n < 11; n++)
		m_msr_mtrrfix[n] = 0;
	for (int n = 0; n < (1024 / 4); n++)
		m_memory_ranges_1m[n] = 0; // change the 0 to 6 to test the cache just after reset

	m_cpuid_max_input_value_eax = 0x01;
	m_cpu_version = REG32(EDX);

	// see FEATURE_FLAGS enum for bit names
	m_feature_flags = 0x0383fbff;

	CHANGE_PC(m_eip);
}

void athlonxp_device::parse_mtrrfix(u64 mtrr, offs_t base, int kblock)
{
	int nb = kblock / 4;
	int range = (int)(base >> 12); // base must never be higher than 1 megabyte

	for (int n = 0; n < 8; n++)
	{
		uint8_t type = mtrr & 0xff;

		for (int b = 0; b < nb; b++)
		{
			m_memory_ranges_1m[range] = type;
			range++;
		}
		mtrr = mtrr >> 8;
	}
}

int athlonxp_device::check_cacheable(offs_t address)
{
	offs_t block;
	int disabled;

	disabled = 0;
	if (m_cr[0] & (1 << 30))
		disabled = 128;
	if (address >= 0x100000)
		return disabled;
	block = address >> 12;
	return m_memory_ranges_1m[block] | disabled;
}

template <class dt, offs_t xorle>
dt athlonxp_device::opcode_read_cache(offs_t address)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	u8 *data;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = (address & 63) ^ xorle;
		data = cache.search<CacheRead>(address);
		if (data)
			return *(dt *)(data + offset);
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheRead>(address, &data);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					macache32->write_data32(old_address + w, *(u32 *)(data + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(data + r) = macache32->read_data32(address + r);
			return *(dt *)(data + offset);
		}
		else
		{
			if (sizeof(dt) == 1)
				return macache32->read_data8(address);
			else if (sizeof(dt) == 2)
				return macache32->read_data16(address);
			else
				return macache32->read_data32(address);
		}
	}
	else
	{
		if (sizeof(dt) == 1)
			return macache32->read_data8(address);
		else if (sizeof(dt) == 2)
			return macache32->read_data16(address);
		else
			return macache32->read_data32(address);
	}
}

template <class dt, offs_t xorle>
dt athlonxp_device::program_read_cache(offs_t address)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	u8 *data;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = (address & 63) ^ xorle;
		data = cache.search<CacheRead>(address);
		if (data)
			return *(dt *)(data + offset);
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheRead>(address, &data);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					m_program->write_dword(old_address + w, *(u32 *)(data + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(data + r) = m_program->read_dword(address + r);
			return *(dt *)(data + offset);
		}
		else
		{
			if (sizeof(dt) == 1)
				return m_program->read_byte(address);
			else if (sizeof(dt) == 2)
				return m_program->read_word(address);
			else
				return m_program->read_dword(address);
		}
	}
	else
	{
		if (sizeof(dt) == 1)
			return m_program->read_byte(address);
		else if (sizeof(dt) == 2)
			return m_program->read_word(address);
		else
			return m_program->read_dword(address);
	}
}

template <class dt, offs_t xorle>
void athlonxp_device::program_write_cache(offs_t address, dt data)
{
	int mode = check_cacheable(address);
	bool nocache = false;
	u8 *dataw;

	if ((mode & 7) == 0)
		nocache = true;
	if (mode & 1)
		nocache = true;
	if (nocache == false)
	{
		int offset = (address & 63) ^ xorle;
		dataw = cache.search<CacheWrite>(address);
		if (dataw)
		{
			*(dt *)(dataw + offset) = data;
			return;
		}
		if (!(mode & 128))
		{
			bool dirty = cache.allocate<CacheWrite>(address, &dataw);
			address = cache.base(address);
			if (dirty)
			{
				offs_t old_address = cache.old();

				for (int w = 0; w < 64; w += 4)
					m_program->write_dword(old_address + w, *(u32 *)(dataw + w));
			}
			for (int r = 0; r < 64; r += 4)
				*(u32 *)(dataw + r) = m_program->read_dword(address + r);
			*(dt *)(dataw + offset) = data;
		}
		else
		{
			if (sizeof(dt) == 1)
				m_program->write_byte(address, data);
			else if (sizeof(dt) == 2)
				m_program->write_word(address, data);
			else
				m_program->write_dword(address, data);
		}
	}
	else
	{
		if (sizeof(dt) == 1)
			m_program->write_byte(address, data);
		else if (sizeof(dt) == 2)
			m_program->write_word(address, data);
		else
			m_program->write_dword(address, data);
	}
}

void athlonxp_device::invalidate_cache(bool writeback)
{
	u32 base;
	u8 *data;

	data = cache.first_dirty(base, true);
	while (data != nullptr)
	{
		if (writeback)
			for (int w = 0; w < 64; w += 4)
				m_program->write_dword(base + w, *(u32 *)(data + w));
		data = cache.next_dirty(base, true);
	}
	cache.reset();
}

void athlonxp_device::opcode_invd()
{
	invalidate_cache(false);
}

void athlonxp_device::opcode_wbinvd()
{
	invalidate_cache(true);
}
#endif

/*****************************************************************************/
/* Intel Pentium 4 */

void pentium4_device::initialize()
{
	i386_common_init();
//	register_state_i386_x87_xmm();

	build_x87_opcode_table();
	build_opcode_table(OP_I386 | OP_FPU | OP_I486 | OP_PENTIUM | OP_PPRO | OP_MMX | OP_SSE | OP_SSE2);
	m_cycle_table_rm = cycle_table_rm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
	m_cycle_table_pm = cycle_table_pm[CPU_CYCLES_PENTIUM].get();  // TODO: generate own cycle tables
}

void pentium4_device::reset()
{
	zero_state();

	m_sreg[CS].selector = 0xf000;
	m_sreg[CS].base     = 0xffff0000;
	m_sreg[CS].limit    = 0xffff;
	m_sreg[CS].flags    = 0x0093;

	m_sreg[DS].base = m_sreg[ES].base = m_sreg[FS].base = m_sreg[GS].base = m_sreg[SS].base = 0x00000000;
	m_sreg[DS].limit = m_sreg[ES].limit = m_sreg[FS].limit = m_sreg[GS].limit = m_sreg[SS].limit = 0xffff;
	m_sreg[DS].flags = m_sreg[ES].flags = m_sreg[FS].flags = m_sreg[GS].flags = m_sreg[SS].flags = 0x0093;

	m_idtr.base = 0;
	m_idtr.limit = 0x3ff;

	m_a20_mask = ~0;

	m_cr[0] = 0x60000010;
	m_eflags = 0x00200000;
	m_eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	m_eip = 0xfff0;
	m_mxcsr = 0x1f80;
	m_smm = false;
	m_smi_latched = false;
	m_smbase = 0x30000;
	m_nmi_masked = false;
	m_nmi_latched = false;

	x87_reset();

	// [27:20] Extended family
	// [19:16] Extended model
	// [13:12] Type
	// [11: 8] Family
	// [ 7: 4] Model
	// [ 3: 0] Stepping ID
	// Family 15, Model 0 (Pentium 4 / Willamette)
	REG32(EAX) = 0;
	REG32(EDX) = (0 << 20) | (0xf << 8) | (0 << 4) | (1);

	m_cpuid_id0 = 0x756e6547;   // Genu
	m_cpuid_id1 = 0x49656e69;   // ineI
	m_cpuid_id2 = 0x6c65746e;   // ntel

	m_cpuid_max_input_value_eax = 0x02;
	m_cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	m_feature_flags = 0x00000001;       // TODO: enable relevant flags here

	CHANGE_PC(m_eip);
}

bool i386_device::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("IP")) == 0) {
		m_eip = data & 0xffff;
		CHANGE_PC(m_eip);
	} else if(_tcsicmp(reg, _T("EIP")) == 0) {
		m_eip = data;
		CHANGE_PC(m_eip);
	} else if(_tcsicmp(reg, _T("EAX")) == 0) {
		REG32(EAX) = data;
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		REG32(EBX) = data;
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		REG32(ECX) = data;
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		REG32(EDX) = data;
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		REG32(ESP) = data;
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		REG32(EBP) = data;
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		REG32(ESI) = data;
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		REG32(EDI) = data;
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		REG16(AX) = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		REG16(BX) = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		REG16(CX) = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		REG16(DX) = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		REG16(SP) = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		REG16(BP) = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		REG16(SI) = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		REG16(DI) = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		REG8(AL) = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		REG8(AH) = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		REG8(BL) = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		REG8(BH) = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		REG8(CL) = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		REG8(CH) = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		REG8(DL) = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		REG8(DH) = data;
	} else {
		return false;
	}
	return true;
}

uint32_t i386_device::read_debug_reg(const _TCHAR *reg)
{
	if(_tcsicmp(reg, _T("EIP")) == 0) {
		return m_eip;
	} else if(_tcsicmp(reg, _T("IP")) == 0) {
		return m_eip & 0xffff;
	}  else if(_tcsicmp(reg, _T("EAX")) == 0) {
		return REG32(EAX);
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		return REG32(EBX);
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		return REG32(ECX);
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		return REG32(EDX);
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		return REG32(ESP);
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		return REG32(EBP);
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		return REG32(ESI);
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		return REG32(EDI);
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		return REG16(AX);
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		return REG16(BX);
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		return REG16(CX);
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		return REG16(DX);
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		return REG16(SP);
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		return REG16(BP);
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		return REG16(SI);
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		return REG16(DI);
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		return REG8(AL);
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		return REG8(AH);
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		return REG8(BL);
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		return REG8(BH);
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		return REG8(CL);
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		return REG8(CH);
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		return REG8(DL);
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		return REG8(DH);
	}
	return 0;
}

void i386_device::dump_segs(_TCHAR *buffer, _TCHAR* label, int segnum, size_t len)
{
	if((segnum < 0) || (segnum >= 6)) {
		memset(buffer, 0x00, len);
		return;
	}
	snprintf_s(buffer, len, "%s:%04X BASE=%08X LIMIT=%08X D=%d FLAGS=%04X VALID=%s\n",
			   label,
			   m_sreg[segnum].selector, m_sreg[segnum].base, m_sreg[segnum].limit,
			   m_sreg[segnum].d, m_sreg[segnum].flags, (m_sreg[segnum].valid) ? "YES" : "NO");
}
	
void i386_device::dump_regs(_TCHAR *buffer, size_t len)
{
	_TCHAR minibuffer[512];
	snprintf(minibuffer, 512, "PC=%08X EIP=%08X ", m_pc, m_eip);
	strncat(buffer, minibuffer, len); 
	snprintf(minibuffer, 512, "PREV_PC=%08X PREV_EIP=%08X \n", m_prev_pc, m_prev_eip);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "CS", CS, 512);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "SS", SS, 512);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "DS", DS, 512);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "ES", ES, 512);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "FS", FS, 512);
	strncat(buffer, minibuffer, len);
	dump_segs(minibuffer, "GS", GS, 512);
	strncat(buffer, minibuffer, len);
	
	snprintf(minibuffer, 512, "IOPL=%d CPL=%d EFLAGS=%08X EFLAGS_MASK=%08X\n", ((m_IOP1) | (m_IOP2 << 1)), m_CPL, m_eflags, m_eflags_mask); 
	strncat(buffer, minibuffer, len);
	snprintf(minibuffer, 512, "FLAGS: %s%s%s%s %s%s%s%s\n      %s%s%s%s %s%s%s%s\n",
			 (m_CF == 0) ? "--" : "CF", (m_DF == 0) ? "--" : "CF", (m_SF == 0) ? "--" : "SF", (m_OF == 0) ? "--" : "OF",
			 (m_ZF == 0) ? "-- " : "ZF ", (m_PF == 0) ? "-- " : "PF ", (m_AF == 0) ? "-- " : "AF ", (m_IF == 0) ? "-- " : "IF ",
			 (m_TF == 0) ? "--" : "TF", (m_NT == 0) ? "--" : "NT", (m_RF == 0) ? "--" : "RF", (m_VM == 0) ? "--" : "VM",
			 (m_AC == 0) ? "-- " : "AC ",
			 (m_VIF == 0) ? "---" : "VIF", (m_VIP == 0) ? "---" : "VIP", (m_ID == 0) ? "-- " : "ID ");
	strncat(buffer, minibuffer, len);

	snprintf(minibuffer, 512, "EAX=%08X ECX=%08X EDX=%08X EBX=%08X\n", REG32(EAX), REG32(ECX), REG32(EDX), REG32(EBX));
	strncat(buffer, minibuffer, len);
	
	snprintf(minibuffer, 512, "ESP=%08X EBP=%08X ESI=%08X EDI=%08X\n", REG32(ESP), REG32(EBP), REG32(ESI), REG32(EDI));
	strncat(buffer, minibuffer, len);
}

bool i386_device::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == NULL) return false;
	if(buffer_len <= 0) return false;
	memset(buffer, 0x00, buffer_len * sizeof(_TCHAR));
	// ToDo: XMM, x87
	dump_regs(buffer, buffer_len);
	if(buffer[buffer_len - 1] != '\0') return false;
	return true;
}

const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

int I386::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
//	UINT64 eip = pc - sreg[CS].base;
//	UINT8 ops[16];
//	for(int i = 0; i < 16; i++) {
//		int wait;
//		ops[i] = d_mem->read_data8w(pc + i, &wait);
//	}
//	UINT8 *oprom = ops;
	if(d_dasm == NULL) return 0;
	if((buffer == NULL) || (buffer_len <= 0)) return 0;
	int reply =  d_dasm->disassemble(pc, buffer, buffer_len, this);
	reply = reply & DASMFLAG_LENGTHMASK;
	return reply;
}
