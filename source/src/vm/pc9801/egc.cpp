/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.06.24-
	History: 20190624: Split from display.cpp.

	[ DISPLAY/EGC ]
*/
#include "display.h"
#include "../i8259.h"
#include "../i8255.h"
#include "../upd7220.h"
#include "../../config.h"
#include "./egc_inline.h"

namespace PC9801 {
// EGC based on Neko Project 2 and QEMU/9821

#if defined(SUPPORT_EGC)

__DECL_ALIGNED(16) static const uint8_t DISPLAY::egc_bytemask_u0[64] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
	0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01,
	0xe0, 0x70, 0x38, 0x1c, 0x0e, 0x07, 0x03, 0x01,
	0xf0, 0x78, 0x3c, 0x1e, 0x0f, 0x07, 0x03, 0x01,
	0xf8, 0x7c, 0x3e, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xfc, 0x7e, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xfe, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01,
};


__DECL_ALIGNED(16) static const uint8_t DISPLAY::egc_bytemask_u1[8] =  {
	0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
};

__DECL_ALIGNED(16) static const uint8_t DISPLAY::egc_bytemask_d0[64] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80,
	0x07, 0x0e, 0x1c, 0x38, 0x70, 0xe0, 0xc0, 0x80,
	0x0f, 0x1e, 0x3c, 0x78, 0xf0, 0xe0, 0xc0, 0x80,
	0x1f, 0x3e, 0x7c, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x3f, 0x7e, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0x7f, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
	0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
};

__DECL_ALIGNED(16) static const uint8_t DISPLAY::egc_bytemask_d1[8] = {
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};

__DECL_ALIGNED(16) static const uint16_t DISPLAY::egc_maskword[16][4] = {
	{0x0000, 0x0000, 0x0000, 0x0000}, {0xffff, 0x0000, 0x0000, 0x0000},
	{0x0000, 0xffff, 0x0000, 0x0000}, {0xffff, 0xffff, 0x0000, 0x0000},
	{0x0000, 0x0000, 0xffff, 0x0000}, {0xffff, 0x0000, 0xffff, 0x0000},
	{0x0000, 0xffff, 0xffff, 0x0000}, {0xffff, 0xffff, 0xffff, 0x0000},
	{0x0000, 0x0000, 0x0000, 0xffff}, {0xffff, 0x0000, 0x0000, 0xffff},
	{0x0000, 0xffff, 0x0000, 0xffff}, {0xffff, 0xffff, 0x0000, 0xffff},
	{0x0000, 0x0000, 0xffff, 0xffff}, {0xffff, 0x0000, 0xffff, 0xffff},
	{0x0000, 0xffff, 0xffff, 0xffff}, {0xffff, 0xffff, 0xffff, 0xffff}
};

// SUBROUTINES are moved to display,h due to making inline. 20190514 K.O
void __FASTCALL DISPLAY::egc_sftb_upn_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
	}
//	__DECL_ALIGNED(16) uint8_t tmp[4];
//__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			tmp[i] = egc_outptr[i << 2];
//		}
//	if(ext == 0) {
//	__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			egc_vram_src.b[i][0] = tmp[i];
//		}
//	} else {
//	__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			egc_vram_src.b[i][1] = tmp[i];
//		}
//	}
	egc_vram_src.b[0][ext] = egc_outptr[0];
	egc_vram_src.b[1][ext] = egc_outptr[4];
	egc_vram_src.b[2][ext] = egc_outptr[8];
	egc_vram_src.b[3][ext] = egc_outptr[12];
	egc_outptr++;
}

void __FASTCALL DISPLAY::egc_sftb_dnn_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
	}
//	__DECL_ALIGNED(16) uint8_t tmp[4];
//__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			tmp[i] = egc_outptr[i << 2];
//		}
//	if(ext == 0) {
//	__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			egc_vram_src.b[i][0] = tmp[i];
//		}
//	} else {
//	__DECL_VECTORIZED_LOOP
//		for(int i = 0; i < 4; i++) {
//			egc_vram_src.b[i][1] = tmp[i];
//		}
//	}
	egc_vram_src.b[0][ext] = egc_outptr[0];
	egc_vram_src.b[1][ext] = egc_outptr[4];
	egc_vram_src.b[2][ext] = egc_outptr[8];
	egc_vram_src.b[3][ext] = egc_outptr[12];
	egc_outptr--;
}

void __FASTCALL DISPLAY::egc_sftb_upr_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
		}
		egc_dstbit = 0;
#if 1
	__DECL_ALIGNED(4) uint8_t tmp[4];
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[i] = egc_outptr[i << 2];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[i] >>= egc_sft8bitr;
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_vram_src.b[i][ext] = tmp[i];
		}
#else
		egc_vram_src.b[0][ext] = (egc_outptr[0] >> egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] >> egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] >> egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] >> egc_sft8bitr);
#endif
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
//		egc_outptr++;
#if 1
		__DECL_ALIGNED(16) uint8_t tmp[8];
		__DECL_ALIGNED(4) uint8_t tmp2[4];
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[(i << 1) + 0] = egc_outptr[(i << 2) + 0];
			tmp[(i << 1) + 1] = egc_outptr[(i << 2) + 1];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[(i << 1) + 0] <<= egc_sft8bitl;
			tmp[(i << 1) + 1] >>= egc_sft8bitr;
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp2[i] = tmp[(i << 1) + 0] | tmp[(i << 1) + 1];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_vram_src.b[i][ext] = tmp2[i];
		}
#else
		egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitl) | (egc_outptr[1] >> egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitl) | (egc_outptr[5] >> egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitl) | (egc_outptr[9] >> egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitl) | (egc_outptr[13] >> egc_sft8bitr);
#endif
		egc_outptr++;
	}
}

void __FASTCALL DISPLAY::egc_sftb_dnr_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
		}
		egc_dstbit = 0;
#if 1
		__DECL_ALIGNED(4) uint8_t tmp[4];
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[i] = egc_outptr[i << 2];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[i] <<= egc_sft8bitr;
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_vram_src.b[i][ext] = tmp[i];
		}
#else
		egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitr);
#endif
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
//		egc_outptr--;
#if 1		
		__DECL_ALIGNED(16) uint8_t tmp[8];
		__DECL_ALIGNED(4) uint8_t tmp2[4];
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[(i << 1) + 0] = egc_outptr[(i << 2) + 0];
			tmp[(i << 1) + 1] = egc_outptr[(i << 2) + 1];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp[(i << 1) + 0] <<= egc_sft8bitr;
			tmp[(i << 1) + 1] >>= egc_sft8bitl;
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			tmp2[i] = tmp[(i << 1) + 0] | tmp[(i << 1) + 1];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_vram_src.b[i][ext] = tmp2[i];
		}
#else
		egc_vram_src.b[0][ext] = (egc_outptr[1] >> egc_sft8bitl) | (egc_outptr[0] << egc_sft8bitr);
		egc_vram_src.b[1][ext] = (egc_outptr[5] >> egc_sft8bitl) | (egc_outptr[4] << egc_sft8bitr);
		egc_vram_src.b[2][ext] = (egc_outptr[9] >> egc_sft8bitl) | (egc_outptr[8] << egc_sft8bitr);
		egc_vram_src.b[3][ext] = (egc_outptr[13] >> egc_sft8bitl) | (egc_outptr[12] << egc_sft8bitr);
#endif
		egc_outptr--;
	}
}

void __FASTCALL DISPLAY::egc_sftb_upl_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_u1[egc_remain - 1];
			egc_remain = 0;
		}
	}
//	egc_outptr++;
#if 1
	__DECL_ALIGNED(16) uint8_t tmp[16];
	__DECL_ALIGNED(4) uint8_t tmp2[4];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp[(i << 1) + 0] = egc_outptr[(i << 2) + 0];
		tmp[(i << 1) + 1] = egc_outptr[(i << 2) + 1];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp[(i << 1) + 0] <<= egc_sft8bitl;
		tmp[(i << 1) + 1] >>= egc_sft8bitr;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp2[i] = tmp[(i << 1) + 0] | tmp[(i << 1) + 1];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		egc_vram_src.b[i][ext] = tmp2[i];
	}
#else
	egc_vram_src.b[0][ext] = (egc_outptr[0] << egc_sft8bitl) | (egc_outptr[1] >> egc_sft8bitr);
	egc_vram_src.b[1][ext] = (egc_outptr[4] << egc_sft8bitl) | (egc_outptr[5] >> egc_sft8bitr);
	egc_vram_src.b[2][ext] = (egc_outptr[8] << egc_sft8bitl) | (egc_outptr[9] >> egc_sft8bitr);
	egc_vram_src.b[3][ext] = (egc_outptr[12] << egc_sft8bitl) | (egc_outptr[13] >> egc_sft8bitr);
#endif
	egc_outptr++;
}

void __FASTCALL DISPLAY::egc_sftb_dnl_sub(uint32_t ext)
{
	if(egc_dstbit >= 8) {
		egc_dstbit -= 8;
		egc_srcmask.b[ext] = 0;
		return;
	}
	if(egc_dstbit) {
		if((egc_dstbit + egc_remain) >= 8) {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (7 * 8)];
			egc_remain -= (8 - egc_dstbit);
			egc_dstbit = 0;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d0[egc_dstbit + (egc_remain - 1) * 8];
			egc_remain = 0;
			egc_dstbit = 0;
		}
	} else {
		if(egc_remain >= 8) {
			egc_remain -= 8;
		} else {
			egc_srcmask.b[ext] = egc_bytemask_d1[egc_remain - 1];
			egc_remain = 0;
		}
	}
//	egc_outptr--;
#if 1
	__DECL_ALIGNED(4) uint8_t tmp0[4];
	__DECL_ALIGNED(4) uint8_t tmp1[4];
	__DECL_ALIGNED(4) uint8_t tmp2[4];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp0[i] = egc_outptr[(i << 2) + 0];
		tmp1[i] = egc_outptr[(i << 2) + 1];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp0[i] <<= egc_sft8bitr;
		tmp1[i] >>= egc_sft8bitl;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		tmp2[i] = tmp0[i] | tmp1[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		egc_vram_src.b[i][ext] = tmp2[i];
	}
#else
	egc_vram_src.b[0][ext] = (egc_outptr[1] >> egc_sft8bitl) | (egc_outptr[0] << egc_sft8bitr);
	egc_vram_src.b[1][ext] = (egc_outptr[5] >> egc_sft8bitl) | (egc_outptr[4] << egc_sft8bitr);
	egc_vram_src.b[2][ext] = (egc_outptr[9] >> egc_sft8bitl) | (egc_outptr[8] << egc_sft8bitr);
	egc_vram_src.b[3][ext] = (egc_outptr[13] >> egc_sft8bitl) | (egc_outptr[12] << egc_sft8bitr);
#endif
	egc_outptr--;
}

#if 1
#define EGC_OPE_SHIFTB(addr, value)					\
	{												\
		register uint8_t* __p = &(egc_inptr[0]);	\
		register uint8_t __tmp = (uint8_t)value;	\
		if(egc_ope & 0x400) {						\
			__p[ 0] = __tmp;						\
			__p[ 4] = __tmp;						\
			__p[ 8] = __tmp;						\
			__p[12] = __tmp;						\
			egc_shiftinput_byte(addr & 1);			\
		}											\
	}

#define EGC_OPE_SHIFTW(value)							\
	{													\
		pair16_t  __tmp;								\
		uint8_t* __tmpp;								\
		__tmp.b.l = value;								\
		__tmp.b.h = value >> 8;							\
		if(egc_ope & 0x400) {							\
			if(!(egc_sft & 0x1000)) {					\
				__tmpp = &(egc_inptr[0]);					\
				__tmpp[0] = __tmp.b.l;						\
				__tmpp[1] = __tmp.b.h;						\
				__tmpp[4] = __tmp.b.l;						\
				__tmpp[5] = __tmp.b.h;						\
				__tmpp[8] = __tmp.b.l;						\
				__tmpp[9] = __tmp.b.h;						\
				__tmpp[12] = __tmp.b.l;						\
				__tmpp[13] = __tmp.b.h;						\
				egc_shiftinput_incw();					\
			} else {									\
				__tmpp = (&(egc_inptr[-1]));	\
				__tmpp[0] = __tmp.b.l;						\
				__tmpp[1] = __tmp.b.h;						\
				__tmpp[4] = __tmp.b.l;						\
				__tmpp[5] = __tmp.b.h;						\
				__tmpp[8] = __tmp.b.l;						\
				__tmpp[9] = __tmp.b.h;						\
				__tmpp[12] = __tmp.b.l;						\
				__tmpp[13] = __tmp.b.h;						\
				egc_shiftinput_decw(); \
			}						   \
		}							   \
	}

#else

#define EGC_OPE_SHIFTB(addr, value) \
	__DECL_VECTORIZED_LOOP			\
	do {						  \
		if(egc_ope & 0x400) { \
			egc_inptr[ 0] = (uint8_t)value; \
			egc_inptr[ 4] = (uint8_t)value; \
			egc_inptr[ 8] = (uint8_t)value; \
			egc_inptr[12] = (uint8_t)value; \
			egc_shiftinput_byte(addr & 1); \
		} \
	} while(0)

#define EGC_OPE_SHIFTW(value) \
	__DECL_VECTORIZED_LOOP			\
	do { \
		if(egc_ope & 0x400) { \
			if(!(egc_sft & 0x1000)) { \
				egc_inptr[ 0] = (uint8_t)value; \
				egc_inptr[ 1] = (uint8_t)(value >> 8); \
				egc_inptr[ 4] = (uint8_t)value; \
				egc_inptr[ 5] = (uint8_t)(value >> 8); \
				egc_inptr[ 8] = (uint8_t)value; \
				egc_inptr[ 9] = (uint8_t)(value >> 8); \
				egc_inptr[12] = (uint8_t)value; \
				egc_inptr[13] = (uint8_t)(value >> 8); \
				egc_shiftinput_incw(); \
			} else { \
				egc_inptr[-1] = (uint8_t)value; \
				egc_inptr[ 0] = (uint8_t)(value >> 8); \
				egc_inptr[ 3] = (uint8_t)value; \
				egc_inptr[ 4] = (uint8_t)(value >> 8); \
				egc_inptr[ 7] = (uint8_t)value; \
				egc_inptr[ 8] = (uint8_t)(value >> 8); \
				egc_inptr[11] = (uint8_t)value; \
				egc_inptr[12] = (uint8_t)(value >> 8); \
				egc_shiftinput_decw(); \
			}  \
		} \
	} while(0)
#endif

uint64_t __FASTCALL DISPLAY::egc_ope_xx(uint8_t ope, uint32_t addr)
{
	__DECL_ALIGNED(16) egcquad_t pat;
	__DECL_ALIGNED(16) egcquad_t dst;
	__DECL_ALIGNED(16) egcquad_t tmp;
	
	switch(egc_fgbg & 0x6000) {
	case 0x2000:
		pat.q = egc_bgc.q;
		break;
	case 0x4000:
		pat.q = egc_fgc.q;
		break;
//	default:
	case 0x0000:
		if((egc_ope & 0x0300) == 0x0100) {
			pat.q = egc_vram_src.q;
		} else if((egc_ope & 0x0300) == 0x0200) {
			pat.q = egc_lastvram.q;
		} else {
			pat.q = egc_patreg.q;
		}
		break;
	}
	dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
	dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
	dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
	dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	
	//egc_vram_data.d[0] = 0;
	//egc_vram_data.d[1] = 0;
	tmp.q = 0;
//	tmp.q =  (ope & 0x80) ? (pat.q & egc_vram_src.q & dst.q) : 0;
//	tmp.q |= (ope & 0x40) ? ((~pat.q) & egc_vram_src.q & dst.q) : 0;
//	tmp.q |= (ope & 0x20) ? (pat.q & egc_vram_src.q & (~dst.q)) : 0;
//	tmp.q |= (ope & 0x10) ? ((~pat.q) & egc_vram_src.q & (~dst.q)) : 0;
//	tmp.q |= (ope & 0x08) ? (pat.q & (~egc_vram_src.q) & dst.q) : 0;
//	tmp.q |= (ope & 0x04) ? ((~pat.q) & (~egc_vram_src.q) & dst.q) : 0;
//	tmp.q |= (ope & 0x02) ? (pat.q & (~egc_vram_src.q) & (~dst.q)) : 0;
//	tmp.q |= (ope & 0x01) ? ((~pat.q) & (~egc_vram_src.q) & (~dst.q)) : 0;
	
	if(ope & 0x80) {
		tmp.q |= (pat.q & egc_vram_src.q & dst.q);
	}
	if(ope & 0x40) {
		tmp.q |= ((~pat.q) & egc_vram_src.q & dst.q);
	}
	if(ope & 0x20) {
		tmp.q |= (pat.q & egc_vram_src.q & (~dst.q));
	}
	if(ope & 0x10) {
		tmp.q |= ((~pat.q) & egc_vram_src.q & (~dst.q));
	}
	if(ope & 0x08) {
		tmp.q |= (pat.q & (~egc_vram_src.q) & dst.q);
	}
	if(ope & 0x04) {
		tmp.q |= ((~pat.q) & (~egc_vram_src.q) & dst.q);
	}
	if(ope & 0x02) {
		tmp.q |= (pat.q & (~egc_vram_src.q) & (~dst.q));
	}
	if(ope & 0x01) {
		tmp.q |= ((~pat.q) & (~egc_vram_src.q) & (~dst.q));
	}
	egc_vram_data.q = tmp.q;
	return tmp.q;
}

uint64_t __FASTCALL DISPLAY::egc_opefn(uint32_t func, uint8_t ope, uint32_t addr)
{
	switch(func & 0xff) {
	case 0x00: return egc_ope_00(ope, addr);
	case 0x01: return egc_ope_xx(ope, addr);
	case 0x02: return egc_ope_xx(ope, addr);
	case 0x03: return egc_ope_np(ope, addr);
	case 0x04: return egc_ope_xx(ope, addr);
	case 0x05: return egc_ope_nd(ope, addr);
	case 0x06: return egc_ope_xx(ope, addr);
	case 0x07: return egc_ope_xx(ope, addr);
	case 0x08: return egc_ope_xx(ope, addr);
	case 0x09: return egc_ope_xx(ope, addr);
	case 0x0a: return egc_ope_nd(ope, addr);
	case 0x0b: return egc_ope_xx(ope, addr);
	case 0x0c: return egc_ope_np(ope, addr);
	case 0x0d: return egc_ope_xx(ope, addr);
	case 0x0e: return egc_ope_xx(ope, addr);
	case 0x0f: return egc_ope_0f(ope, addr);
	case 0x10: return egc_ope_xx(ope, addr);
	case 0x11: return egc_ope_xx(ope, addr);
	case 0x12: return egc_ope_xx(ope, addr);
	case 0x13: return egc_ope_xx(ope, addr);
	case 0x14: return egc_ope_xx(ope, addr);
	case 0x15: return egc_ope_xx(ope, addr);
	case 0x16: return egc_ope_xx(ope, addr);
	case 0x17: return egc_ope_xx(ope, addr);
	case 0x18: return egc_ope_xx(ope, addr);
	case 0x19: return egc_ope_xx(ope, addr);
	case 0x1a: return egc_ope_xx(ope, addr);
	case 0x1b: return egc_ope_xx(ope, addr);
	case 0x1c: return egc_ope_xx(ope, addr);
	case 0x1d: return egc_ope_xx(ope, addr);
	case 0x1e: return egc_ope_xx(ope, addr);
	case 0x1f: return egc_ope_xx(ope, addr);
	case 0x20: return egc_ope_xx(ope, addr);
	case 0x21: return egc_ope_xx(ope, addr);
	case 0x22: return egc_ope_xx(ope, addr);
	case 0x23: return egc_ope_xx(ope, addr);
	case 0x24: return egc_ope_xx(ope, addr);
	case 0x25: return egc_ope_xx(ope, addr);
	case 0x26: return egc_ope_xx(ope, addr);
	case 0x27: return egc_ope_xx(ope, addr);
	case 0x28: return egc_ope_xx(ope, addr);
	case 0x29: return egc_ope_xx(ope, addr);
	case 0x2a: return egc_ope_xx(ope, addr);
	case 0x2b: return egc_ope_xx(ope, addr);
	case 0x2c: return egc_ope_xx(ope, addr);
	case 0x2d: return egc_ope_xx(ope, addr);
	case 0x2e: return egc_ope_xx(ope, addr);
	case 0x2f: return egc_ope_xx(ope, addr);
	case 0x30: return egc_ope_np(ope, addr);
	case 0x31: return egc_ope_xx(ope, addr);
	case 0x32: return egc_ope_xx(ope, addr);
	case 0x33: return egc_ope_np(ope, addr);
	case 0x34: return egc_ope_xx(ope, addr);
	case 0x35: return egc_ope_xx(ope, addr);
	case 0x36: return egc_ope_xx(ope, addr);
	case 0x37: return egc_ope_xx(ope, addr);
	case 0x38: return egc_ope_xx(ope, addr);
	case 0x39: return egc_ope_xx(ope, addr);
	case 0x3a: return egc_ope_xx(ope, addr);
	case 0x3b: return egc_ope_xx(ope, addr);
	case 0x3c: return egc_ope_np(ope, addr);
	case 0x3d: return egc_ope_xx(ope, addr);
	case 0x3e: return egc_ope_xx(ope, addr);
	case 0x3f: return egc_ope_np(ope, addr);
	case 0x40: return egc_ope_xx(ope, addr);
	case 0x41: return egc_ope_xx(ope, addr);
	case 0x42: return egc_ope_xx(ope, addr);
	case 0x43: return egc_ope_xx(ope, addr);
	case 0x44: return egc_ope_xx(ope, addr);
	case 0x45: return egc_ope_xx(ope, addr);
	case 0x46: return egc_ope_xx(ope, addr);
	case 0x47: return egc_ope_xx(ope, addr);
	case 0x48: return egc_ope_xx(ope, addr);
	case 0x49: return egc_ope_xx(ope, addr);
	case 0x4a: return egc_ope_xx(ope, addr);
	case 0x4b: return egc_ope_xx(ope, addr);
	case 0x4c: return egc_ope_xx(ope, addr);
	case 0x4d: return egc_ope_xx(ope, addr);
	case 0x4e: return egc_ope_xx(ope, addr);
	case 0x4f: return egc_ope_xx(ope, addr);
	case 0x50: return egc_ope_nd(ope, addr);
	case 0x51: return egc_ope_xx(ope, addr);
	case 0x52: return egc_ope_xx(ope, addr);
	case 0x53: return egc_ope_xx(ope, addr);
	case 0x54: return egc_ope_xx(ope, addr);
	case 0x55: return egc_ope_nd(ope, addr);
	case 0x56: return egc_ope_xx(ope, addr);
	case 0x57: return egc_ope_xx(ope, addr);
	case 0x58: return egc_ope_xx(ope, addr);
	case 0x59: return egc_ope_xx(ope, addr);
	case 0x5a: return egc_ope_nd(ope, addr);
	case 0x5b: return egc_ope_xx(ope, addr);
	case 0x5c: return egc_ope_xx(ope, addr);
	case 0x5d: return egc_ope_xx(ope, addr);
	case 0x5e: return egc_ope_xx(ope, addr);
	case 0x5f: return egc_ope_nd(ope, addr);
	case 0x60: return egc_ope_xx(ope, addr);
	case 0x61: return egc_ope_xx(ope, addr);
	case 0x62: return egc_ope_xx(ope, addr);
	case 0x63: return egc_ope_xx(ope, addr);
	case 0x64: return egc_ope_xx(ope, addr);
	case 0x65: return egc_ope_xx(ope, addr);
	case 0x66: return egc_ope_xx(ope, addr);
	case 0x67: return egc_ope_xx(ope, addr);
	case 0x68: return egc_ope_xx(ope, addr);
	case 0x69: return egc_ope_xx(ope, addr);
	case 0x6a: return egc_ope_xx(ope, addr);
	case 0x6b: return egc_ope_xx(ope, addr);
	case 0x6c: return egc_ope_xx(ope, addr);
	case 0x6d: return egc_ope_xx(ope, addr);
	case 0x6e: return egc_ope_xx(ope, addr);
	case 0x6f: return egc_ope_xx(ope, addr);
	case 0x70: return egc_ope_xx(ope, addr);
	case 0x71: return egc_ope_xx(ope, addr);
	case 0x72: return egc_ope_xx(ope, addr);
	case 0x73: return egc_ope_xx(ope, addr);
	case 0x74: return egc_ope_xx(ope, addr);
	case 0x75: return egc_ope_xx(ope, addr);
	case 0x76: return egc_ope_xx(ope, addr);
	case 0x77: return egc_ope_xx(ope, addr);
	case 0x78: return egc_ope_xx(ope, addr);
	case 0x79: return egc_ope_xx(ope, addr);
	case 0x7a: return egc_ope_xx(ope, addr);
	case 0x7b: return egc_ope_xx(ope, addr);
	case 0x7c: return egc_ope_xx(ope, addr);
	case 0x7d: return egc_ope_xx(ope, addr);
	case 0x7e: return egc_ope_xx(ope, addr);
	case 0x7f: return egc_ope_xx(ope, addr);
	case 0x80: return egc_ope_xx(ope, addr);
	case 0x81: return egc_ope_xx(ope, addr);
	case 0x82: return egc_ope_xx(ope, addr);
	case 0x83: return egc_ope_xx(ope, addr);
	case 0x84: return egc_ope_xx(ope, addr);
	case 0x85: return egc_ope_xx(ope, addr);
	case 0x86: return egc_ope_xx(ope, addr);
	case 0x87: return egc_ope_xx(ope, addr);
	case 0x88: return egc_ope_xx(ope, addr);
	case 0x89: return egc_ope_xx(ope, addr);
	case 0x8a: return egc_ope_xx(ope, addr);
	case 0x8b: return egc_ope_xx(ope, addr);
	case 0x8c: return egc_ope_xx(ope, addr);
	case 0x8d: return egc_ope_xx(ope, addr);
	case 0x8e: return egc_ope_xx(ope, addr);
	case 0x8f: return egc_ope_xx(ope, addr);
	case 0x90: return egc_ope_xx(ope, addr);
	case 0x91: return egc_ope_xx(ope, addr);
	case 0x92: return egc_ope_xx(ope, addr);
	case 0x93: return egc_ope_xx(ope, addr);
	case 0x94: return egc_ope_xx(ope, addr);
	case 0x95: return egc_ope_xx(ope, addr);
	case 0x96: return egc_ope_xx(ope, addr);
	case 0x97: return egc_ope_xx(ope, addr);
	case 0x98: return egc_ope_xx(ope, addr);
	case 0x99: return egc_ope_xx(ope, addr);
	case 0x9a: return egc_ope_xx(ope, addr);
	case 0x9b: return egc_ope_xx(ope, addr);
	case 0x9c: return egc_ope_xx(ope, addr);
	case 0x9d: return egc_ope_xx(ope, addr);
	case 0x9e: return egc_ope_xx(ope, addr);
	case 0x9f: return egc_ope_xx(ope, addr);
	case 0xa0: return egc_ope_nd(ope, addr);
	case 0xa1: return egc_ope_xx(ope, addr);
	case 0xa2: return egc_ope_xx(ope, addr);
	case 0xa3: return egc_ope_xx(ope, addr);
	case 0xa4: return egc_ope_xx(ope, addr);
	case 0xa5: return egc_ope_nd(ope, addr);
	case 0xa6: return egc_ope_xx(ope, addr);
	case 0xa7: return egc_ope_xx(ope, addr);
	case 0xa8: return egc_ope_xx(ope, addr);
	case 0xa9: return egc_ope_xx(ope, addr);
	case 0xaa: return egc_ope_nd(ope, addr);
	case 0xab: return egc_ope_xx(ope, addr);
	case 0xac: return egc_ope_xx(ope, addr);
	case 0xad: return egc_ope_xx(ope, addr);
	case 0xae: return egc_ope_xx(ope, addr);
	case 0xaf: return egc_ope_nd(ope, addr);
	case 0xb0: return egc_ope_xx(ope, addr);
	case 0xb1: return egc_ope_xx(ope, addr);
	case 0xb2: return egc_ope_xx(ope, addr);
	case 0xb3: return egc_ope_xx(ope, addr);
	case 0xb4: return egc_ope_xx(ope, addr);
	case 0xb5: return egc_ope_xx(ope, addr);
	case 0xb6: return egc_ope_xx(ope, addr);
	case 0xb7: return egc_ope_xx(ope, addr);
	case 0xb8: return egc_ope_xx(ope, addr);
	case 0xb9: return egc_ope_xx(ope, addr);
	case 0xba: return egc_ope_xx(ope, addr);
	case 0xbb: return egc_ope_xx(ope, addr);
	case 0xbc: return egc_ope_xx(ope, addr);
	case 0xbd: return egc_ope_xx(ope, addr);
	case 0xbe: return egc_ope_xx(ope, addr);
	case 0xbf: return egc_ope_xx(ope, addr);
	case 0xc0: return egc_ope_c0(ope, addr);
	case 0xc1: return egc_ope_xx(ope, addr);
	case 0xc2: return egc_ope_xx(ope, addr);
	case 0xc3: return egc_ope_np(ope, addr);
	case 0xc4: return egc_ope_xx(ope, addr);
	case 0xc5: return egc_ope_xx(ope, addr);
	case 0xc6: return egc_ope_xx(ope, addr);
	case 0xc7: return egc_ope_xx(ope, addr);
	case 0xc8: return egc_ope_xx(ope, addr);
	case 0xc9: return egc_ope_xx(ope, addr);
	case 0xca: return egc_ope_xx(ope, addr);
	case 0xcb: return egc_ope_xx(ope, addr);
	case 0xcc: return egc_ope_np(ope, addr);
	case 0xcd: return egc_ope_xx(ope, addr);
	case 0xce: return egc_ope_xx(ope, addr);
	case 0xcf: return egc_ope_np(ope, addr);
	case 0xd0: return egc_ope_xx(ope, addr);
	case 0xd1: return egc_ope_xx(ope, addr);
	case 0xd2: return egc_ope_xx(ope, addr);
	case 0xd3: return egc_ope_xx(ope, addr);
	case 0xd4: return egc_ope_xx(ope, addr);
	case 0xd5: return egc_ope_xx(ope, addr);
	case 0xd6: return egc_ope_xx(ope, addr);
	case 0xd7: return egc_ope_xx(ope, addr);
	case 0xd8: return egc_ope_xx(ope, addr);
	case 0xd9: return egc_ope_xx(ope, addr);
	case 0xda: return egc_ope_xx(ope, addr);
	case 0xdb: return egc_ope_xx(ope, addr);
	case 0xdc: return egc_ope_xx(ope, addr);
	case 0xdd: return egc_ope_xx(ope, addr);
	case 0xde: return egc_ope_xx(ope, addr);
	case 0xdf: return egc_ope_xx(ope, addr);
	case 0xe0: return egc_ope_xx(ope, addr);
	case 0xe1: return egc_ope_xx(ope, addr);
	case 0xe2: return egc_ope_xx(ope, addr);
	case 0xe3: return egc_ope_xx(ope, addr);
	case 0xe4: return egc_ope_xx(ope, addr);
	case 0xe5: return egc_ope_xx(ope, addr);
	case 0xe6: return egc_ope_xx(ope, addr);
	case 0xe7: return egc_ope_xx(ope, addr);
	case 0xe8: return egc_ope_xx(ope, addr);
	case 0xe9: return egc_ope_xx(ope, addr);
	case 0xea: return egc_ope_xx(ope, addr);
	case 0xeb: return egc_ope_xx(ope, addr);
	case 0xec: return egc_ope_xx(ope, addr);
	case 0xed: return egc_ope_xx(ope, addr);
	case 0xee: return egc_ope_xx(ope, addr);
	case 0xef: return egc_ope_xx(ope, addr);
	case 0xf0: return egc_ope_f0(ope, addr);
	case 0xf1: return egc_ope_xx(ope, addr);
	case 0xf2: return egc_ope_xx(ope, addr);
	case 0xf3: return egc_ope_np(ope, addr);
	case 0xf4: return egc_ope_xx(ope, addr);
	case 0xf5: return egc_ope_nd(ope, addr);
	case 0xf6: return egc_ope_xx(ope, addr);
	case 0xf7: return egc_ope_xx(ope, addr);
	case 0xf8: return egc_ope_xx(ope, addr);
	case 0xf9: return egc_ope_xx(ope, addr);
	case 0xfa: return egc_ope_nd(ope, addr);
	case 0xfb: return egc_ope_xx(ope, addr);
	case 0xfc: return egc_ope_fc(ope, addr);
	case 0xfd: return egc_ope_xx(ope, addr);
	case 0xfe: return egc_ope_xx(ope, addr);
	case 0xff: return egc_ope_ff(ope, addr);
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
	return 0;
}

uint64_t __FASTCALL DISPLAY::egc_opeb(uint32_t addr, uint8_t value)
{
	uint32_t tmp;
	if(!(enable_egc)) return 0;
	
	egc_mask2.w = egc_mask.w;
	switch(egc_ope & 0x1800) {
	case 0x0800:
		EGC_OPE_SHIFTB(addr, value);
		egc_mask2.w &= egc_srcmask.w;
		tmp = egc_ope & 0xff;
		return egc_opefn(tmp, (uint8_t)tmp, addr & (~1));
	case 0x1000:
		switch(egc_fgbg & 0x6000) {
		case 0x2000:
			return egc_bgc.q;
		case 0x4000:
			return egc_fgc.q;
//		default: // 0x0000, 0x6000
		case 0x0000:
			EGC_OPE_SHIFTB(addr, value);
			egc_mask2.w &= egc_srcmask.w;
			return egc_vram_src.q;
		default:
			return egc_vram_src.q;
		}
		break;
	default:
		tmp = value & 0xff;
		tmp = tmp | (tmp << 8);
		egc_vram_data.w[0] = (uint16_t)tmp;
		egc_vram_data.w[1] = (uint16_t)tmp;
		egc_vram_data.w[2] = (uint16_t)tmp;
		egc_vram_data.w[3] = (uint16_t)tmp;
		return egc_vram_data.q;
	}
}

uint64_t __FASTCALL DISPLAY::egc_opew(uint32_t addr, uint16_t value)
{
	uint32_t tmp;
	if(!(enable_egc)) return 0;

	egc_mask2.w = egc_mask.w;
	switch(egc_ope & 0x1800) {
	case 0x0800:
		EGC_OPE_SHIFTW(value);
		egc_mask2.w &= egc_srcmask.w;
		tmp = egc_ope & 0xff;
		return egc_opefn(tmp, (uint8_t)tmp, addr);
	case 0x1000:
		switch(egc_fgbg & 0x6000) {
		case 0x2000:
			return egc_bgc.q;
		case 0x4000:
			return egc_fgc.q;
//		default:
		case 0x0000:
			EGC_OPE_SHIFTW(value);
			egc_mask2.w &= egc_srcmask.w;
			return egc_vram_src.q;
		default:
			return egc_vram_src.q;
		}
		break;
	default:
#ifdef __BIG_ENDIAN__
		value = ((value >> 8) & 0xff) | ((value & 0xff) << 8);
#endif
		egc_vram_data.w[0] = (uint16_t)value;
		egc_vram_data.w[1] = (uint16_t)value;
		egc_vram_data.w[2] = (uint16_t)value;
		egc_vram_data.w[3] = (uint16_t)value;
		return egc_vram_data.q;
	}
}

uint32_t __FASTCALL DISPLAY::egc_readb(uint32_t addr1)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	uint32_t ext = addr1 & 1;
	static const uint32_t vram_base[4] = {VRAM_PLANE_ADDR_0,
										  VRAM_PLANE_ADDR_1,
										  VRAM_PLANE_ADDR_2,
										  VRAM_PLANE_ADDR_3};
	if(!(enable_egc)) return 0;
	__DECL_ALIGNED(16) uint32_t realaddr[4];
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		realaddr[i] = addr | vram_base[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		egc_lastvram.b[i][ext] = vram_draw[realaddr[i]];
	}
//	egc_lastvram.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
//	egc_lastvram.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
//	egc_lastvram.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
//	egc_lastvram.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	
	if(!(egc_ope & 0x400)) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_inptr[i << 2] = egc_lastvram.b[i][ext];
		}
//		egc_inptr[0] = egc_lastvram.b[0][ext];
//		egc_inptr[4] = egc_lastvram.b[1][ext];
//		egc_inptr[8] = egc_lastvram.b[2][ext];
//		egc_inptr[12] = egc_lastvram.b[3][ext];
		egc_shiftinput_byte(ext);
	}
	if((egc_ope & 0x0300) == 0x0100) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_patreg.b[i][ext] = vram_draw[realaddr[i]];
		}	
//		egc_patreg.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
//		egc_patreg.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
//		egc_patreg.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
//		egc_patreg.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	}
	if(!(egc_ope & 0x2000)) {
		int pl = (egc_fgbg >> 8) & 3;
		if(!(egc_ope & 0x400)) {
			return egc_vram_src.b[pl][ext];
		} else {
			return vram_draw[addr | (VRAM_PLANE_SIZE * pl)];
//			return vram_draw[];
		}
	}
	return vram_draw[addr1];
}

uint32_t __FASTCALL DISPLAY::egc_readw(uint32_t addr1)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	static const uint32_t vram_base[4] = {VRAM_PLANE_ADDR_0,
										  VRAM_PLANE_ADDR_1,
										  VRAM_PLANE_ADDR_2,
										  VRAM_PLANE_ADDR_3};
	if(!(enable_egc)) return 0;
	__DECL_ALIGNED(16) uint32_t realaddr[4];
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		realaddr[i] = addr | vram_base[i];
	}
	if(!(addr & 1)) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_lastvram.w[i] = *(uint16_t *)(&vram_draw[realaddr[i]]);
		}
//		egc_lastvram.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
//		egc_lastvram.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
//		egc_lastvram.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
//		egc_lastvram.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
		
		if(!(egc_ope & 0x400)) {
			if(!(egc_sft & 0x1000)) {
				uint8_t *pp = (uint8_t*)(&(egc_lastvram.b[0][0]));
			__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 4; i++) {
					egc_inptr[(i << 2) + 0] = pp[(i << 1) + 0];
					egc_inptr[(i << 2) + 1] = pp[(i << 1) + 1];
				}					
//				egc_inptr[ 0] = egc_lastvram.b[0][0];
//				egc_inptr[ 1] = egc_lastvram.b[0][1];
//				egc_inptr[ 4] = egc_lastvram.b[1][0];
//				egc_inptr[ 5] = egc_lastvram.b[1][1];
//				egc_inptr[ 8] = egc_lastvram.b[2][0];
//				egc_inptr[ 9] = egc_lastvram.b[2][1];
//				egc_inptr[12] = egc_lastvram.b[3][0];
//				egc_inptr[13] = egc_lastvram.b[3][1];
				egc_shiftinput_incw();
			} else {
				uint8_t *pp = (uint8_t*)(&(egc_lastvram.b[0][0]));
			__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 4; i++) {
					egc_inptr[(i << 2) - 1] = pp[(i << 1) + 0];
					egc_inptr[(i << 2) + 0] = pp[(i << 1) + 1];
				}					
//				egc_inptr[-1] = egc_lastvram.b[0][0];
//				egc_inptr[ 0] = egc_lastvram.b[0][1];
//				egc_inptr[ 3] = egc_lastvram.b[1][0];
//				egc_inptr[ 4] = egc_lastvram.b[1][1];
//				egc_inptr[ 7] = egc_lastvram.b[2][0];
//				egc_inptr[ 8] = egc_lastvram.b[2][1];
//				egc_inptr[11] = egc_lastvram.b[3][0];
//				egc_inptr[12] = egc_lastvram.b[3][1];
				egc_shiftinput_decw();
			}
		}
		if((egc_ope & 0x0300) == 0x0100) {
			egc_patreg.d[0] = egc_lastvram.d[0];
			egc_patreg.d[1] = egc_lastvram.d[1];
		}
		if(!(egc_ope & 0x2000)) {
			int pl = (egc_fgbg >> 8) & 3;
			if(!(egc_ope & 0x400)) {
				return egc_vram_src.w[pl];
			} else {
				return *(uint16_t *)(&vram_draw[addr | (VRAM_PLANE_SIZE * pl)]);
			}
		}
		return *(uint16_t *)(&vram_draw[addr1]);
	} else if(!(egc_sft & 0x1000)) {
		uint16_t value = egc_readb(addr1);
		value |= egc_readb(addr1 + 1) << 8;
		return value;
	} else {
		uint16_t value = egc_readb(addr1) << 8;
		value |= egc_readb(addr1 + 1);
		return value;
	}
}

void __FASTCALL DISPLAY::egc_writeb(uint32_t addr1, uint8_t value)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	uint32_t ext = addr1 & 1;
	static const uint32_t vram_base[4] = {VRAM_PLANE_ADDR_0,
										  VRAM_PLANE_ADDR_1,
										  VRAM_PLANE_ADDR_2,
										  VRAM_PLANE_ADDR_3};
	__DECL_ALIGNED(16) uint32_t realaddr[4];
	__DECL_ALIGNED(16) egcquad_t data;
	if(!(enable_egc)) return 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		realaddr[i] = addr | vram_base[i];
	}
	if((egc_ope & 0x0300) == 0x0200) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			egc_patreg.b[i][ext] = vram_draw[realaddr[i]];
		}
//		egc_patreg.b[0][ext] = vram_draw[addr | VRAM_PLANE_ADDR_0];
//		egc_patreg.b[1][ext] = vram_draw[addr | VRAM_PLANE_ADDR_1];
//		egc_patreg.b[2][ext] = vram_draw[addr | VRAM_PLANE_ADDR_2];
//		egc_patreg.b[3][ext] = vram_draw[addr | VRAM_PLANE_ADDR_3];
	}
	data.q = egc_opeb(addr, value);
	uint32_t bit;
	uint8_t mask = egc_mask2.b[ext];
	__DECL_ALIGNED(4) uint8_t n[4] = {0};
	__DECL_ALIGNED(4) uint8_t m[4] = {0};
	if(egc_mask2.b[ext]) {
		bit = 1;
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			if((bit & egc_access) == 0) { 
				m[i] = data.b[i][ext];
				n[i] = vram_draw[realaddr[i]];
			}
			bit <<= 1;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			n[i] &= ~mask;
			m[i] &= mask;
			n[i] |= m[i];
		}
		bit = 1;
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 4; i++) {
			if((bit & egc_access) == 0) { 
				vram_draw[realaddr[i]] = n[i];
			}
			bit <<= 1;
		}
		
//			vram_draw[realaddr[i]] &= ~egc_mask2.b[ext];
//			vram_draw[realaddr[i]] |= data.b[3][ext] & egc_mask2.b[ext];

//		if(!(egc_access & 1)) {
//			vram_draw[addr | VRAM_PLANE_ADDR_0] &= ~egc_mask2.b[ext];
//			vram_draw[addr | VRAM_PLANE_ADDR_0] |= data.b[0][ext] & egc_mask2.b[ext];
//		}
//		if(!(egc_access & 2)) {
//			vram_draw[addr | VRAM_PLANE_ADDR_1] &= ~egc_mask2.b[ext];
//			vram_draw[addr | VRAM_PLANE_ADDR_1] |= data.b[1][ext] & egc_mask2.b[ext];
//		}
//		if(!(egc_access & 4)) {
//			vram_draw[addr | VRAM_PLANE_ADDR_2] &= ~egc_mask2.b[ext];
//			vram_draw[addr | VRAM_PLANE_ADDR_2] |= data.b[2][ext] & egc_mask2.b[ext];
//		}
//		if(!(egc_access & 8)) {
//			vram_draw[addr | VRAM_PLANE_ADDR_3] &= ~egc_mask2.b[ext];
//			vram_draw[addr | VRAM_PLANE_ADDR_3] |= data.b[3][ext] & egc_mask2.b[ext];
//		}
	}
}

void __FASTCALL DISPLAY::egc_writew(uint32_t addr1, uint16_t value)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	static const uint32_t vram_base[4] = {VRAM_PLANE_ADDR_0,
										  VRAM_PLANE_ADDR_1,
										  VRAM_PLANE_ADDR_2,
										  VRAM_PLANE_ADDR_3};
	__DECL_ALIGNED(16) uint32_t realaddr[4];
	__DECL_ALIGNED(16) egcquad_t data;
	if(!(enable_egc)) return;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		realaddr[i] = addr | vram_base[i];
	}
	
	if(!(addr & 1)) {
		if((egc_ope & 0x0300) == 0x0200) {
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 4; i++) {
				egc_patreg.w[i] = *(uint16_t *)(&vram_draw[realaddr[i]]);
			}
			//egc_patreg.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
			//egc_patreg.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
			//egc_patreg.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
			//egc_patreg.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
		}
		data.q = egc_opew(addr, value);
		uint32_t bit;
		uint16_t mask = egc_mask2.w;
		__DECL_ALIGNED(8) uint16_t n[4] = {0};
		__DECL_ALIGNED(8) uint16_t m[4] = {0};
		if(egc_mask2.w) {
			bit = 1;
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 4; i++) {
				if((bit & egc_access) == 0) { 
					m[i] = data.w[i];
					n[i] = *((uint16_t*)(&(vram_draw[realaddr[i]])));
				}
				bit <<= 1;
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 4; i++) {
				n[i] &= ~mask;
				m[i] &= mask;
				n[i] |= m[i];
			}
			bit = 1;
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 4; i++) {
				if((bit & egc_access) == 0) { 
					*((uint16_t*)(&(vram_draw[realaddr[i]]))) = n[i];
				}
				bit <<= 1;
			}
			
//			if(!(egc_access & 1)) {
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) &= ~egc_mask2.w;
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) |= data.w[0] & egc_mask2.w;
//			}
//			if(!(egc_access & 2)) {
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) &= ~egc_mask2.w;
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) |= data.w[1] & egc_mask2.w;//
//			}
//			if(!(egc_access & 4)) {
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) &= ~egc_mask2.w;
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) |= data.w[2] & egc_mask2.w;
//			}
//			if(!(egc_access & 8)) {
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) &= ~egc_mask2.w;
//				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) |= data.w[3] & egc_mask2.w;
//			}
		}
	} else if(!(egc_sft & 0x1000)) {
		egc_writeb(addr1, (uint8_t)value);
		egc_writeb(addr1 + 1, (uint8_t)(value >> 8));
	} else {
		egc_writeb(addr1, (uint8_t)(value >> 8));
		egc_writeb(addr1 + 1, (uint8_t)value);
	}
}
#endif
}
