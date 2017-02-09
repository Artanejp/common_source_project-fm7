
/************************************************************************/

#define SetTF(x)		(TF = (x))
#define SetIF(x)		(IF = (x))
#define SetDF(x)		(DirVal = (x) ? -1 : 1)
#define SetMD(x)		(MF = (x))

#define SetOFW_Add(x, y, z)	(OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x, y, z)	(OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x, y, z)	(OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x, y, z)	(OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define SetCFB(x)		(CarryVal = (x) & 0x100)
#define SetCFW(x)		(CarryVal = (x) & 0x10000)
#define SetAF(x, y, z)		(AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(SignVal = (x))
#define SetZF(x)		(ZeroVal = (x))
#define SetPF(x)		(ParityVal = (x))

#define SetSZPF_Byte(x)		(ParityVal = SignVal = ZeroVal = (int8_t)(x))
#define SetSZPF_Word(x)		(ParityVal = SignVal = ZeroVal = (int16_t)(x))

#define ADDB(dst, src)		{ unsigned res = (dst) + (src); SetCFB(res); SetOFB_Add(res, src, dst); SetAF(res, src, dst); SetSZPF_Byte(res); dst = (uint8_t)res; }
#define ADDW(dst, src)		{ unsigned res = (dst) + (src); SetCFW(res); SetOFW_Add(res, src, dst); SetAF(res, src, dst); SetSZPF_Word(res); dst = (uint16_t)res; }

#define SUBB(dst, src)		{ unsigned res = (dst) - (src); SetCFB(res); SetOFB_Sub(res, src, dst); SetAF(res, src, dst); SetSZPF_Byte(res); dst = (uint8_t)res; }
#define SUBW(dst, src)		{ unsigned res = (dst) - (src); SetCFW(res); SetOFW_Sub(res, src, dst); SetAF(res, src, dst); SetSZPF_Word(res); dst = (uint16_t)res; }

#define ORB(dst, src)		dst |= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define ORW(dst, src)		dst |= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define ANDB(dst, src)		dst &= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define ANDW(dst, src)		dst &= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define XORB(dst, src)		dst ^= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define XORW(dst, src)		dst ^= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define CF			(CarryVal != 0)
#define SF			(SignVal < 0)
#define ZF			(ZeroVal == 0)
#define PF			parity_table[ParityVal]
#define AF			(AuxVal != 0)
#define OF			(OverVal != 0)
#define DF			(DirVal < 0)
#define MD			(MF != 0)

/************************************************************************/

#define AMASK	0xfffff

#define read_mem_byte(a)	d_mem->read_data8((a) & AMASK)
#define read_mem_word(a)	d_mem->read_data16((a) & AMASK)
#define write_mem_byte(a, d)	d_mem->write_data8((a) & AMASK, (d))
#define write_mem_word(a, d)	d_mem->write_data16((a) & AMASK, (d))

#define read_port_byte(a)	d_io->read_io8(a)
#define read_port_word(a)	d_io->read_io16(a)
#define write_port_byte(a, d)	d_io->write_io8((a), (d))
#define write_port_word(a, d)	d_io->write_io16((a), (d))

/************************************************************************/

#define SegBase(Seg)		(sregs[Seg] << 4)

#define DefaultSeg(Seg)		((seg_prefix && (Seg == DS || Seg == SS)) ? prefix_seg : Seg)
#define DefaultBase(Seg)	((seg_prefix && (Seg == DS || Seg == SS)) ? base[prefix_seg] : base[Seg])

#define GetMemB(Seg, Off)	(read_mem_byte((DefaultBase(Seg) + (Off)) & AMASK))
#define GetMemW(Seg, Off)	(read_mem_word((DefaultBase(Seg) + (Off)) & AMASK))
#define PutMemB(Seg, Off, x)	write_mem_byte((DefaultBase(Seg) + (Off)) & AMASK, (x))
#define PutMemW(Seg, Off, x)	write_mem_word((DefaultBase(Seg) + (Off)) & AMASK, (x))

#define ReadByte(ea)		(read_mem_byte((ea) & AMASK))
#define ReadWord(ea)		(read_mem_word((ea) & AMASK))
#define WriteByte(ea, val)	write_mem_byte((ea) & AMASK, val);
#define WriteWord(ea, val)	write_mem_word((ea) & AMASK, val);

#define FETCH			read_mem_byte(pc++)
#define FETCHOP			read_mem_byte(pc++)
#define FETCHWORD(var)		{ var = read_mem_word(pc); pc += 2; }
#define PUSH(val)		{ regs.w[SP] -= 2; WriteWord(((base[SS] + regs.w[SP]) & AMASK), val); }
#define POP(var)		{ regs.w[SP] += 2; var = ReadWord(((base[SS] + ((regs.w[SP]-2) & 0xffff)) & AMASK)); }

/************************************************************************/

#define CompressFlags() (uint16_t)(CF | (PF << 2) | (AF << 4) | (ZF << 6) | (SF << 7) | (TF << 8) | (IF << 9) | (DF << 10) | (OF << 11) | (MD << 15))

#define ExpandFlags(f) { \
	CarryVal = (f) & 1; \
	ParityVal = !((f) & 4); \
	AuxVal = (f) & 0x10; \
	ZeroVal = !((f) & 0x40); \
	SignVal = ((f) & 0x80) ? -1 : 0; \
	TF = ((f) & 0x100) >> 8; \
	IF = ((f) & 0x200) >> 9; \
	MF = ((f) & 0x8000) >> 15; \
	DirVal = ((f) & 0x400) ? -1 : 1; \
	OverVal = (f) & 0x800; \
}

/************************************************************************/

#define RegWord(ModRM) regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? regs.w[Mod_RM.RM.w[ModRM]] : (GetEA(ModRM), ReadWord(ea)))

#define PutbackRMWord(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.w[Mod_RM.RM.w[ModRM]] = val; \
	} else { \
		WriteWord(ea, val); \
	} \
}

#define GetNextRMWord ( \
	ReadWord(ea + 2) \
)

#define GetRMWordOffset(offs) ( \
	ReadWord(ea - eo + (uint16_t)(eo + offs)) \
)

#define GetRMByteOffset(offs) ( \
	ReadByte(ea - eo + (uint16_t)(eo + offs)) \
)

#define PutRMWord(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.w[Mod_RM.RM.w[ModRM]] = val; \
	} else { \
		GetEA(ModRM); \
		WriteWord(ea, val); \
	} \
}

#define PutRMWordOffset(offs, val) \
	WriteWord(ea - eo + (uint16_t)(eo + offs), val)

#define PutRMByteOffset(offs, val) \
	WriteByte(ea - eo + (uint16_t)(eo + offs), val)

#define PutImmRMWord(ModRM) { \
	uint16_t val; \
	if (ModRM >= 0xc0) { \
		FETCHWORD(regs.w[Mod_RM.RM.w[ModRM]]) \
	} else { \
		GetEA(ModRM); \
		FETCHWORD(val) \
		WriteWord(ea, val); \
	} \
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? regs.b[Mod_RM.RM.b[ModRM]] : (GetEA(ModRM), ReadByte(ea)))

#define PutRMByte(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = val; \
	} else { \
		GetEA(ModRM); \
		WriteByte(ea, val); \
	} \
}

#define PutImmRMByte(ModRM) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = FETCH; \
	} else { \
		GetEA(ModRM); \
		WriteByte(ea, FETCH); \
	} \
}

#define PutbackRMByte(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = val; \
	} else { \
		WriteByte(ea, val); \
	} \
}

#define DEF_br8(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned src = RegByte(ModRM); \
	unsigned dst = GetRMByte(ModRM)

#define DEF_wr16(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned src = RegWord(ModRM); \
	unsigned dst = GetRMWord(ModRM)

#define DEF_r8b(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned dst = RegByte(ModRM); \
	unsigned src = GetRMByte(ModRM)

#define DEF_r16w(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned dst = RegWord(ModRM); \
	unsigned src = GetRMWord(ModRM)

#define DEF_ald8(dst, src) \
	unsigned src = FETCHOP; \
	unsigned dst = regs.b[AL]

#define DEF_axd16(dst, src) \
	unsigned src = FETCHOP; \
	unsigned dst = regs.w[AX]; \
	src += (FETCH << 8)

/************************************************************************/

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

