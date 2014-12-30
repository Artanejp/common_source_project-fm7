/*
 * Zoom x4.5
 * (C) 2014 K.Ohta
 * 
 * History:
 *  2013-04-02 Move from scaler_x2.c
 */
#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;

extern void pVram2RGB_x45_Line(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

#if defined(__SSE2__)
static void Scaler_DrawLine(v4hi *dst, Uint32 *src, int ww, int repeat, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   if(repeat <= 0) return;

# ifndef __x86_64__     
   v4hi *b2p;
   v4hi r1, r2;
   v4hi *d0;
   v4hi *b;
   int pitch2;
#if AG_BIG_ENDIAN != 1
   const v4ui bb = {0xff000000, 0xff000000, 0xff000000, 0xff000000};
#else
   const v4ui bb = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};
#endif
   b = (v4hi *)src;
   b2p = dst;
   pitch2 = pitch / sizeof(v4hi);
   if((bFullScan) || (repeat < 2)) {
      v4hi r3, r4, r5, r6, r7;
      v4hi r8, r9, r10, r11;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 777766666555544444333322222111100000
	 r3.uv  = (v4ui){r1.i[0], r1.i[0], r1.i[0], r1.i[0]};  
	 r4.uv  = (v4ui){r1.i[0], r1.i[1], r1.i[1], r1.i[1]};  
	 r5.uv  = (v4ui){r1.i[1], r1.i[2], r1.i[2], r1.i[2]};  
	 r6.uv  = (v4ui){r1.i[2], r1.i[2], r1.i[3], r1.i[3]};  
	 r7.uv  = (v4ui){r1.i[3], r1.i[3], r2.i[0], r2.i[0]};  

	 r8.uv  = (v4ui){r2.i[0], r2.i[0], r2.i[0], r2.i[1]};  
	 r9.uv  = (v4ui){r2.i[1], r2.i[1], r2.i[1], r2.i[2]};  
	 r10.uv = (v4ui){r2.i[2], r2.i[2], r2.i[2], r2.i[2]};  
	 r11.uv = (v4ui){r2.i[3], r2.i[3], r2.i[3], r2.i[3]};  
	 for(yy = 0; yy < repeat; yy++) {
	    b2p[0] = r3;
	    b2p[1] = r4;
	    b2p[2] = r5;
	    b2p[3] = r6;
	    b2p[4] = r7;
	    b2p[5] = r8;
	    b2p[6] = r9;
	    b2p[7] = r10;
	    b2p[8] = r11;
	    b2p = b2p + pitch2;
	 }
	 dst += 9;
//	 b += 2;
      }
   } else {
      v4hi r3, r4, r5, r6, r7;
      v4hi r8, r9, r10, r11;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 777766666555544444333322222111100000
	 r3.uv  = (v4ui){r1.i[0], r1.i[0], r1.i[0], r1.i[0]};  
	 r4.uv  = (v4ui){r1.i[0], r1.i[1], r1.i[1], r1.i[1]};  
	 r5.uv  = (v4ui){r1.i[1], r1.i[2], r1.i[2], r1.i[2]};  
	 r6.uv  = (v4ui){r1.i[2], r1.i[2], r1.i[3], r1.i[3]};  
	 r7.uv  = (v4ui){r1.i[3], r1.i[3], r2.i[0], r2.i[0]};  

	 r8.uv  = (v4ui){r2.i[0], r2.i[0], r2.i[0], r2.i[1]};  
	 r9.uv  = (v4ui){r2.i[1], r2.i[1], r2.i[1], r2.i[2]};  
	 r10.uv = (v4ui){r2.i[2], r2.i[2], r2.i[2], r2.i[2]};  
	 r11.uv = (v4ui){r2.i[3], r2.i[3], r2.i[3], r2.i[3]};  
	 for(yy = 0; yy < repeat - 1; yy++) {
	    b2p[0] = r3;
	    b2p[1] = r4;
	    b2p[2] = r5;
	    b2p[3] = r6;
	    b2p[4] = r7;
	    b2p[5] = r8;
	    b2p[6] = r9;
	    b2p[7] = r10;
	    b2p[8] = r11;
	    b2p = b2p + pitch2;
	 }
	 b2p[0].uv = 
	 b2p[1].uv = 
	 b2p[2].uv = 
	 b2p[3].uv = 
	 b2p[4].uv = 
	 b2p[5].uv = 
	 b2p[6].uv = 
	 b2p[7].uv = 
	 b2p[8].uv = bb;
	 dst += 9;
//	 b += 2;
      }
   }
#else // __x86_64__
   
   if((bFullScan) || (repeat < 2)) {
      yrep2 = repeat;
      if(yrep2 < 1) yrep2 = 1;
      yrep3 = 0;
   } else {
      yrep2 = repeat - 1;
      yrep3 = 1;
   }
   // 76543210 -> 7777 6666 6555 5444 4433 3322 2221 1110 0000
   asm (
		"subq $64, %%rsp /* Allocate local value */\n\t"
		"movq %[src], %%rsi\n\t"
		"movq %[dst], %%rdi\n\t"
		"movl %[pitch], %%eax\n\t"
		"movq %%rax, %%r10 /* pitch */\n\t"
		"movq %%rdi, 40(%%rsp)  /* dst */\n\t"
		"movl %[ww], %%ecx\n\t"
		"shr  $3, %%ecx\n\t"
		"movl %%ecx, 32(%%rsp) /* r10 / 16(rsp) = count(ww) */\n\t"
		"movl %%ecx, 24(%%rsp) /* r10 / 16(rsp) = count(ww) */\n\t"
		"movl %%ecx, 16(%%rsp) /* r10 / 16(rsp) = count(ww) */\n\t"
		"movl %[rep2], %%ecx \n\t"
		"movl %%ecx, 8(%%rsp) /*  r9 / 8(rsp) = yrep2 */\n\t"
		"movl %[rep3], %%ecx \n\t"
		"movl %%ecx, 0(%%rsp) /*  r8 / 0(rsp) = yrep3 */\n\t"
		"movq $0, %%r11 /* Set offset counter of source */\n\t"
		"movl 32(%%rsp), %%ecx \n\t"
		"_l0:\n\t"
		"movl %%ecx, 16(%%rsp) \n\t"
		"/* Get upper to xmm0 */\n\t"
		"movdqu 0(%%rsi),  %%xmm0 /* Get Upper */\n\t"
		"movdqu 16(%%rsi), %%xmm5 /* Get Lower */\n\t"
		"/* 76543210 -> 7777 6666 6555 5444 4433 3322 2221 1110 0000 */"
		"pshufd $0b11111111, %%xmm0, %%xmm1 /* 7777 -> xmm1 */\n\t"
		"pshufd $0b11101010, %%xmm0, %%xmm2 /* 6666 -> xmm2 */\n\t"
		"pshufd $0b10100101, %%xmm0, %%xmm3 /* 6555 -> xmm3 */\n\t"
		"pshufd $0b01010000, %%xmm0, %%xmm4 /* 5444 -> xmm4 */\n\t"
		"addq $32, %%rsi\n\t"
		"movd %%xmm1, %%eax /* $00,$00,0,4 */\n\t"
		"movd %%eax, %%xmm6\n\t"
		"pshufd $0b11110000, %%xmm6, %%xmm6\n\t"
		"/* Store higher */\n\t"
		"movq $0, %%r13 /* r13 -> offset */\n\t"
		"movl 8(%%rsp), %%ecx /* yrep2 */\n\t"
		"pushq %%rdi\n\t"
		"_l1a:\n\t"
		"movdqu %%xmm4, 0(%%rdi) /* store 6666 */\n\t"
		"movdqu %%xmm3, 16(%%rdi) /* store 6555 */\n\t"
		"movdqu %%xmm2, 32(%%rdi) /* store 5444 */\n\t"
		"movdqu %%xmm1, 48(%%rdi) /* store 5444 */\n\t"
		"addq %%r10, %%rdi\n\t"
		"dec %%ecx\n\t"
		"jnz _l1a\n\t"
		"popq %%rdi\n\t"
		"pshufd $0b11111111, %%xmm5, %%xmm1 /* 3322 */\n\t"
		"pshufd $0b11101010, %%xmm5, %%xmm2 /* 2221 */\n\t"
		"pshufd $0b10100101, %%xmm5, %%xmm3 /* 1110 */\n\t"
		"pshufd $0b01010000, %%xmm5, %%xmm4 /* 0000 */\n\t"
		"movd %%xmm4, %%eax\n\t"
		"movd %%eax, %%xmm0\n\t"
		"pshufd $0b00001111, %%xmm0, %%xmm0\n\t"
		"por %%xmm0, %%xmm6\n\t"
		"movl 8(%%rsp), %%ecx\n\t"
		"movq $0, %%r13 /* r13 -> offset */\n\t"
		"pushq %%rdi\n\t"
		"_l1b:\n\t"
		"movdqu %%xmm6, 64(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm4, 80(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm3, 96(%%rdi) /* store 2221 */\n\t"
		"movdqu %%xmm2, 112(%%rdi) /* store 1110 */\n\t"
		"movdqu %%xmm1, 128(%%rdi) /* store 0000 */\n\t"
		"addq %%r10, %%rdi\n\t"
		"dec %%ecx\n\t"
		"jnz _l1b\n\t"
		"popq %%rdi\n\t"
		"addq $144, %%rdi\n\t"
		"addq $4, %%r11\n\t"
		"movl 16(%%rsp), %%ecx\n\t"
		"dec %%ecx\n\t"
		"jnz _l0\n\t"
		
		"movl 0(%%rsp), %%ebx\n\t"
		"cmpl $0, %%ebx /* cmp yrep3, 0 */\n\t"
		"jz _l2c\n\t"
		
		"/* clear */"
		"movl $0xff000000, %%eax\n\t"
		"movd %%eax, %%xmm0\n\t"
		"pshufd $0b00000000, %%xmm0, %%xmm0\n\t"
		"_l2a:\n\t"
		"movq 40(%%rsp), %%rdi\n\t"
		"movl 32(%%rsp), %%ecx\n\t"
		"pushq %%rdi\n\t"
		"_l2b:\n\t"
		"movdqu %%xmm0,  0(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 16(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 32(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 48(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 64(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 80(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 96(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 112(%%rdi) /* store 3322 */\n\t"
		"movdqu %%xmm0, 128(%%rdi) /* store 3322 */\n\t"
		"addq $144, %%rdi\n\t"
		"dec %%ecx\n\t"
		"jnz _l2b\n\t"
		"popq %%rdi\n\t"
		"addq %%r10, %%r13\n\t"
		"dec %%ebx\n\t"
		"jnz _l2a\n\t"
		"_l2c:\n\t"
		"addq $64, %%rsp"
		:
		: [src] "rm" (src), [dst] "rm" (dst), [pitch] "rm" (pitch),
		   [ww] "rm" (ww), [rep2] "rm" (yrep2), [rep3] "rm" (yrep3)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
		  "rax", "rbx", "rcx", "rdi", "rsi", "r10", "r11", "r12", "r13" );
#endif   
}



void pVram2RGB_x45_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
{
   register v4hi *b;
   AG_Surface *Surface = GetDrawSurface();
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w;
   int h;
   int yy;
   int xx;
   int hh;
   int ww;
   int i;
   int x = xbegin;
   int yrep2;
   unsigned  pitch;
   Uint32 black;
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;


   ww = xend - xbegin;
//   if(ww > (w / 2)) ww = w / 2;
   ww = (ww / 8) * 8;
   if(ww <= 0) return;


#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
//   yrep = yrep * 16.0f;

   yrep2 = yrep;

   d1 = (Uint32 *)((Uint8 *)dst + ((x * 18) / 4) * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   Scaler_DrawLine((v4hi *)d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}


#else 

void pVram2RGB_x45_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x45_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__