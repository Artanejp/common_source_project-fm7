// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Phil Bennett
#ifndef __LIB_I386_OPS_OPS_H__
#define __LIB_I386_OPS_OPS_H__
class I386_OPS_BASE;
typedef struct X86_OPCODE {
	UINT8 opcode;
	UINT32 flags;
	void (I386_OPS_BASE::*handler16)();
	void (I386_OPS_BASE::*handler32)();
	bool lockable;
};

#define OP_I386         0x1
#define OP_FPU          0x2
#define OP_I486         0x4
#define OP_PENTIUM      0x8
#define OP_MMX          0x10
#define OP_PPRO         0x20
#define OP_SSE          0x40
#define OP_SSE2         0x80
#define OP_SSE3         0x100
#define OP_CYRIX        0x8000
#define OP_2BYTE        0x80000000
#define OP_3BYTE66      0x40000000
#define OP_3BYTEF2      0x20000000
#define OP_3BYTEF3      0x10000000
#define OP_3BYTE38      0x08000000
#define OP_3BYTE3A      0x04000000
#define OP_4BYTE3866    0x02000000
#define OP_4BYTE3A66    0x01000000
#define OP_4BYTE38F2    0x00800000
#define OP_4BYTE3AF2    0x00400000
#define OP_4BYTE38F3    0x00200000

#endif
