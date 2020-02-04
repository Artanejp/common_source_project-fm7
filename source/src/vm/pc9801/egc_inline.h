#pragma once

namespace PC9801 {
	
#if defined(SUPPORT_EGC)

inline void __FASTCALL DISPLAY::egc_shift()
{
	uint8_t src8, dst8;
	
	egc_remain = (egc_leng & 0xfff) + 1;
	egc_func = (egc_sft >> 12) & 1;
	if(!egc_func) {
		egc_inptr = egc_buf;
		egc_outptr = egc_buf;
	} else {
		egc_inptr = egc_buf + 4096 / 8 + 3;
		egc_outptr = egc_buf + 4096 / 8 + 3;
	}
	egc_srcbit = egc_sft & 0x0f;
	egc_dstbit = (egc_sft >> 4) & 0x0f;
	
	src8 = egc_srcbit & 0x07;
	dst8 = egc_dstbit & 0x07;
	if(src8 < dst8) {
		egc_func += 2;
		egc_sft8bitr = dst8 - src8;
		egc_sft8bitl = 8 - egc_sft8bitr;
	}
	else if(src8 > dst8) {
		egc_func += 4;
		egc_sft8bitl = src8 - dst8;
		egc_sft8bitr = 8 - egc_sft8bitl;
	}
	egc_stack = 0;
}


inline void __FASTCALL DISPLAY::egc_sftb_upn0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upn_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_upn0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upn_sub(0);
	if(egc_remain) {
		egc_sftb_upn_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb_dnn0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnn_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_dnn0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnn_sub(1);
	if(egc_remain) {
		egc_sftb_dnn_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb_upr0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upr_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_upr0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upr_sub(0);
	if(egc_remain) {
		egc_sftb_upr_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb_dnr0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnr_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_dnr0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnr_sub(1);
	if(egc_remain) {
		egc_sftb_dnr_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb_upl0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_upl_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_upl0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_upl_sub(0);
	if(egc_remain) {
		egc_sftb_upl_sub(1);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[1] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb_dnl0(uint32_t ext)
{
	if(egc_stack < (uint32_t)(8 - egc_dstbit)) {
		egc_srcmask.b[ext] = 0;
		return;
	}
	egc_stack -= (8 - egc_dstbit);
	egc_sftb_dnl_sub(ext);
	if(!egc_remain) {
		egc_shift();
	}
}

inline void __FASTCALL DISPLAY::egc_sftw_dnl0()
{
	if(egc_stack < (uint32_t)(16 - egc_dstbit)) {
		egc_srcmask.w = 0;
		return;
	}
	egc_stack -= (16 - egc_dstbit);
	egc_sftb_dnl_sub(1);
	if(egc_remain) {
		egc_sftb_dnl_sub(0);
		if(egc_remain) {
			return;
		}
	} else {
		egc_srcmask.b[0] = 0;
	}
	egc_shift();
}

inline void __FASTCALL DISPLAY::egc_sftb(int func, uint32_t ext)
{
	switch(func) {
	case 0: egc_sftb_upn0(ext); break;
	case 1: egc_sftb_dnn0(ext); break;
	case 2: egc_sftb_upr0(ext); break;
	case 3: egc_sftb_dnr0(ext); break;
	case 4: egc_sftb_upl0(ext); break;
	case 5: egc_sftb_dnl0(ext); break;
	}
}

inline void __FASTCALL DISPLAY::egc_sftw(int func)
{
	switch(func) {
	case 0: egc_sftw_upn0(); break;
	case 1: egc_sftw_dnn0(); break;
	case 2: egc_sftw_upr0(); break;
	case 3: egc_sftw_dnr0(); break;
	case 4: egc_sftw_upl0(); break;
	case 5: egc_sftw_dnl0(); break;
	}
}

inline void __FASTCALL DISPLAY::egc_shiftinput_byte(uint32_t ext)
{
	if(egc_stack <= 16) {
		if(egc_srcbit >= 8) {
			egc_srcbit -= 8;
		} else {
			egc_stack += (8 - egc_srcbit);
			egc_srcbit = 0;
		}
		if(!(egc_sft & 0x1000)) {
			egc_inptr++;
		} else {
			egc_inptr--;
		}
	}
	egc_srcmask.b[ext] = 0xff;
	egc_sftb(egc_func, ext);
}

inline void __FASTCALL DISPLAY::egc_shiftinput_incw()
{
	if(egc_stack <= 16) {
		egc_inptr += 2;
		if(egc_srcbit >= 8) {
			egc_outptr++;
		}
		egc_stack += (16 - egc_srcbit);
		egc_srcbit = 0;
	}
	egc_srcmask.w = 0xffff;
	egc_sftw(egc_func);
}

inline void __FASTCALL DISPLAY::egc_shiftinput_decw()
{
	if(egc_stack <= 16) {
		egc_inptr -= 2;
		if(egc_srcbit >= 8) {
			egc_outptr--;
		}
		egc_stack += (16 - egc_srcbit);
		egc_srcbit = 0;
	}
	egc_srcmask.w = 0xffff;
	egc_sftw(egc_func);
}




inline uint64_t __FASTCALL DISPLAY::egc_ope_00(uint8_t ope, uint32_t addr)
{
	return 0;
}

inline uint64_t __FASTCALL DISPLAY::egc_ope_0f(uint8_t ope, uint32_t addr)
{
	egc_vram_data.q = ~egc_vram_src.q;
//	egc_vram_data.d[0] = ~egc_vram_src.d[0];
//	egc_vram_data.d[1] = ~egc_vram_src.d[1];
	return egc_vram_data.q;
}

inline uint64_t __FASTCALL DISPLAY::egc_ope_c0(uint8_t ope, uint32_t addr)
{
	__DECL_ALIGNED(16) egcquad_t dst;
	#ifdef __BIG_ENDIAN__
		dst.w[0] = vram_draw_readw(addr | VRAM_PLANE_ADDR_0);
		dst.w[1] = vram_draw_readw(addr | VRAM_PLANE_ADDR_1);
		dst.w[2] = vram_draw_readw(addr | VRAM_PLANE_ADDR_2);
		dst.w[3] = vram_draw_readw(addr | VRAM_PLANE_ADDR_3);
	#else
		dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
		dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
		dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
		dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	#endif
//	egc_vram_data.d[0] = (egc_vram_src.d[0] & dst.d[0]);
//	egc_vram_data.d[1] = (egc_vram_src.d[1] & dst.d[1]);
	egc_vram_data.q = egc_vram_src.q & dst.q;
	return egc_vram_data.q;
}

inline uint64_t __FASTCALL DISPLAY::egc_ope_f0(uint8_t ope, uint32_t addr)
{
	return egc_vram_src.q;
}


inline uint64_t __FASTCALL DISPLAY::egc_ope_fc(uint8_t ope, uint32_t addr)
{
	__DECL_ALIGNED(16) egcquad_t dst;

	#ifdef __BIG_ENDIAN__
		dst.w[0] = vram_draw_readw(addr | VRAM_PLANE_ADDR_0);
		dst.w[1] = vram_draw_readw(addr | VRAM_PLANE_ADDR_1);
		dst.w[2] = vram_draw_readw(addr | VRAM_PLANE_ADDR_2);
		dst.w[3] = vram_draw_readw(addr | VRAM_PLANE_ADDR_3);
	#else
		dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
		dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
		dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
		dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	#endif
	egc_vram_data.q = egc_vram_src.q;
	egc_vram_data.q |= ((~egc_vram_src.q) & dst.q);
//	egc_vram_data.d[0] = egc_vram_src.d[0];
//	egc_vram_data.d[0] |= ((~egc_vram_src.d[0]) & dst.d[0]);
//	egc_vram_data.d[1] = egc_vram_src.d[1];
//	egc_vram_data.d[1] |= ((~egc_vram_src.d[1]) & dst.d[1]);
	return egc_vram_data.q;
}

inline uint64_t __FASTCALL DISPLAY::egc_ope_ff(uint8_t ope, uint32_t addr)
{
	return ~0;
}


inline uint64_t __FASTCALL DISPLAY::egc_ope_nd(uint8_t ope, uint32_t addr)
{
	__DECL_ALIGNED(16) egcquad_t pat;
	__DECL_ALIGNED(16) egcquad_t tmp;

	switch(egc_fgbg & 0x6000) {
	case 0x2000:
		pat.q = egc_bgc.q;
//		pat.d[0] = egc_bgc.d[0];
//		pat.d[1] = egc_bgc.d[1];
		break;
	case 0x4000:
		pat.q = egc_fgc.q;
//		pat.d[0] = egc_fgc.d[0];
//		pat.d[1] = egc_fgc.d[1];
		break;
//	default:
	case 0x0000:
		if((egc_ope & 0x0300) == 0x0100) {
			pat.q = egc_vram_src.q;
//			pat.d[0] = egc_vram_src.d[0];
//			pat.d[1] = egc_vram_src.d[1];
		} else if((egc_ope & 0x0300) == 0x0200) {
			pat.q = egc_lastvram.q;
		} else {
			pat.q = egc_patreg.q;
//			pat.d[0] = egc_patreg.d[0];
//			pat.d[1] = egc_patreg.d[1];
		}
		break;
	}
	tmp.q =  (ope & 0x80) ? (pat.q & egc_vram_src.q) : 0;
	tmp.q |= (ope & 0x40) ? ((~pat.q) & egc_vram_src.q) : 0;
	tmp.q |= (ope & 0x08) ? (pat.q & (~egc_vram_src.q)) : 0;
	tmp.q |= (ope & 0x04) ? ((~pat.q) & (~egc_vram_src.q)) : 0;
/*
	tmp.q = 0;
	if(ope & 0x80) {
		egc_vram_data.d[0] |= (pat.d[0] & egc_vram_src.d[0]);
		egc_vram_data.d[1] |= (pat.d[1] & egc_vram_src.d[1]);
		tmp.q |= (pat.q & egc_vram_src.q);
	}
	if(ope & 0x40) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & egc_vram_src.d[0]);
		egc_vram_data.d[1] |= ((~pat.d[1]) & egc_vram_src.d[1]);
		tmp.q |= ((~pat.q) & egc_vram_src.q);
	}
	if(ope & 0x08) {
		egc_vram_data.d[0] |= (pat.d[0] & (~egc_vram_src.d[0]));
		egc_vram_data.d[1] |= (pat.d[1] & (~egc_vram_src.d[1]));
		tmp.q |= (pat.q & (~egc_vram_src.q));
	}
	if(ope & 0x04) {
		egc_vram_data.d[0] |= ((~pat.d[0]) & (~egc_vram_src.d[0]));
		egc_vram_data.d[1] |= ((~pat.d[1]) & (~egc_vram_src.d[1]));
		tmp.q |= ((~pat.q) & (~egc_vram_src.q));
	}
*/
	egc_vram_data.q = tmp.q;
	return tmp.q;
}

inline uint64_t __FASTCALL DISPLAY::egc_ope_np(uint8_t ope, uint32_t addr)
{
	__DECL_ALIGNED(16) egcquad_t dst;
	__DECL_ALIGNED(16) egcquad_t tmp;
	
	#ifdef __BIG_ENDIAN__
		dst.w[0] = vram_draw_readw(addr | VRAM_PLANE_ADDR_0);
		dst.w[1] = vram_draw_readw(addr | VRAM_PLANE_ADDR_1);
		dst.w[2] = vram_draw_readw(addr | VRAM_PLANE_ADDR_2);
		dst.w[3] = vram_draw_readw(addr | VRAM_PLANE_ADDR_3);
	#else
		dst.w[0] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
		dst.w[1] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
		dst.w[2] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
		dst.w[3] = *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
	#endif

	tmp.q =  (ope & 0x80) ? (dst.q & egc_vram_src.q) : 0;
	tmp.q |= (ope & 0x20) ? ((~dst.q) & egc_vram_src.q) : 0;
	tmp.q |= (ope & 0x08) ? (dst.q & (~egc_vram_src.q)) : 0;
	tmp.q |= (ope & 0x02) ? ((~dst.q) & (~egc_vram_src.q)) : 0;
	/*
	tmp.q = 0;
	if(ope & 0x80) {
		tmp.q |= (egc_vram_src.q & dst.q);
	}
	if(ope & 0x20) {
		tmp.q |= (egc_vram_src.q & (~dst.q));
	}
	if(ope & 0x08) {
		tmp.q |= ((~egc_vram_src.q) & dst.q);
	}
	if(ope & 0x02) {
		tmp.q |= ((~egc_vram_src.q) & (~dst.q));
	}
	*/
	egc_vram_data.q = tmp.q;
	return egc_vram_data.q;
}
#endif	

}
