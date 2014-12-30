/*
 * Zoom x2x2
 * (C) 2013 K.Ohta
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

extern void pVram2RGB_x2(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

#if defined(__SSE2__)
//	     b2p = d0;
//	     b2.vv = __builtin_ia32_pshufd(b[0].v, 0x50);
//	     b3.vv = __builtin_ia32_pshufd(b[0].v, 0xfa);

//	     b4.vv = __builtin_ia32_pshufd(b[1].v, 0x50);
//	     b5.vv = __builtin_ia32_pshufd(b[1].v, 0xfa);

static void Scaler_DrawLine(v4hi *dst, Uint32 *src, int ww, int repeat, int pitch)
{

#ifndef __x86_64__
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   v4hi *b2p;
   volatile v4hi r1, r2;
   v4hi *d0;
   v4hi *b;
   v4hi bb2;
   int pitch2;
#if AG_BIG_ENDIAN != 1
   const v4ui bb = {0xff000000, 0xff000000, 0xff000000, 0xff000000};
#else
   const v4ui bb = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};
#endif
   if(repeat <= 0) return;
   b = (v4hi *)src;
   bb2.uv = bb;
   b2p = dst;
   pitch2 = pitch / sizeof(v4hi);
   if((bFullScan) || (repeat < 2)) {
      volatile v4hi r3, r4, r5, r6;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
         r3.vv = __builtin_ia32_pshufd(r1.vv, 0x50);
         r4.vv = __builtin_ia32_pshufd(r1.vv, 0xfa);

         r5.vv = __builtin_ia32_pshufd(r2.vv, 0x50);
	 r6.vv = __builtin_ia32_pshufd(r2.vv, 0xfa);
	 for(yy = 0; yy < repeat; yy++) {
	       b2p[0] = r3;
	       b2p[1] = r4;
	       b2p[2] = r5;
	       b2p[3] = r6;
	       
	       b2p = b2p + pitch2;
	 }
	 dst = dst + 4;
//	 b += 2;
      }
   } else {
      volatile v4hi r3, r4, r5, r6;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
	 r3 = r1;
	 r4 = r1;
	 r5 = r2;
	 r6 = r2;
         r3.vv = __builtin_ia32_pshufd(r1.vv, 0x50);
         r4.vv = __builtin_ia32_pshufd(r1.vv, 0xfa);

         r5.vv = __builtin_ia32_pshufd(r2.vv, 0x50);
	 r6.vv = __builtin_ia32_pshufd(r2.vv, 0xfa);
	 for(yy = 0; yy < repeat - 1; yy++) {
	    *b2p++ = r3;
	    *b2p++ = r4;
	    *b2p++ = r5;
	    *b2p++ = r6;
	    b2p = b2p + (pitch2 - 4);
	 }
	 *b2p++ = bb2;
	 *b2p++ = bb2;
	 *b2p++ = bb2;
	 *b2p++ = bb2;
	 dst += 4;
//	 b += 2;
      }
   }
#else /* __x86_64__ */
   int yrep2, yrep3;
   
   if(repeat <= 0) return;
   if((bFullScan) || (repeat < 2)) {
      yrep2 = repeat;
      if(yrep2 < 1) yrep2 = 1;
      yrep3 = 0;
   } else {
      yrep2 = repeat - 1;
      yrep3 = 1;
   }
   // 7766554433221100
   asm ( "/* _dst: .equ 40 */\n\t"
	 "/*_count0: .equ 32 */\n\t"
	 "/*_count1: .equ 24 */\n\t"
	 "/*_count2: .equ 16 */\n\t"
	 "/*_yrep2:  .equ 8 */\n\t"
	 "/*_yrep3:  .equ 0 */\n\t"
	 "subq $64, %%rsp /* Allocate local value */\n\t"
	 "movq %[src], %%rsi\n\t"
	 "movq %[dst], %%rdi\n\t"
	 "movq %%rdi, 40(%%rsp) /* _dst */\n\t"
	 
	 "movl %[pitch], %%eax\n\t"
	 "movq %%rax, %%r10 /* pitch */\n\t"
	 
	 "movl %[ww], %%ecx \n\t"
	 "shrl $3, %%ecx\n\t"
	 "movl %%ecx, 32(%%rsp) /* _count0 */\n\t"
	 
	 "movl %[rep2], %%r11d\n\t"
	 "movl %[rep3], %%r12d\n\t"
	 "movl %%r12d, 0(%%rsp) /* _yrep3 */\n\t"
	 "cmpl $0, %%r11d\n\t"
	 "je _l2\n\t"
	 "movl %%r11d, 8(%%rsp) /* _yrep2 */\n\t"
	 
	 "cmpl $0, %%ecx\n\t"
	 "je _exit0\n\t"
	 
	 "_l0: \n\t"
	 "movdqu 0(%%rsi), %%xmm0 /* 0123 */\n\t"
	 "movdqu 16(%%rsi), %%xmm1 /* 4567 */\n\t"
	 "pshufd $0b01010000, %%xmm0, %%xmm2 /* 2233 */\n\t"
	 "pshufd $0b11111010, %%xmm0, %%xmm0 /* 0011 */\n\t"
	 "pshufd $0b01010000, %%xmm1, %%xmm3 /* 6677 */\n\t"
	 "pshufd $0b11111010, %%xmm1, %%xmm1 /* 4455 */\n\t"
	 "addq $32, %%rsi\n\t"
	 "movl %%r11d, %%r13d\n\t"
	 "movq %%rdi,  %%r14\n\t"
	 "_l0a: \n\t"
	 "movdqu %%xmm2, 0(%%rdi)\n\t"
	 "movdqu %%xmm0, 16(%%rdi)\n\t"
	 "movdqu %%xmm3, 32(%%rdi)\n\t"
	 "movdqu %%xmm1, 48(%%rdi)\n\t"
	 "addq %%r10, %%rdi\n\t"
	 "decl %%r13d\n\t"
	 "jnz _l0a\n\t"
	 "addq $64, %%r14\n\t"
	 "movq %%r14, %%rdi\n\t"
	 "decl %%ecx\n\t"
	 "jnz _l0\n\t"

	 "movl 0(%%rsp), %%ecx /* _yrep3 */\n\t"
	 "cmpl $1, %%ecx\n\t"
	 "jl _exit0\n\t"
	 
	 "movq 40(%%rsp), %%rdi /* _dst */\n\t"
	 "movl 8(%%rsp), %%eax /* _yrep2 */\n\t"
	 "mulq %%r10\n\t"
	 "addq %%rax, %%rdi\n\t"
	 "movq %%rdi, %%r14\n\t"
	 
	 "movl $0xff000000, %%eax /* ABGR */\n\t"
	 "movd %%eax, %%xmm0\n\t"
	 "pshufd $0b00000000, %%xmm0, %%xmm0\n\t"
	 
	 "_l2: \n\t"
	 "movl 32(%%rsp), %%r8d /* _count0 */\n\t"
	 "cmpl $1, %%r8d\n\t"
	 "jl _exit0\n\t"
	 
	 "_l2a:\n\t"
	 "movdqu %%xmm0, 0(%%rdi)\n\t"
	 "movdqu %%xmm0, 16(%%rdi)\n\t"
	 "movdqu %%xmm0, 32(%%rdi)\n\t"
	 "movdqu %%xmm0, 48(%%rdi)\n\t"
	 "addq $64, %%rdi\n\t"
	
	 "decl %%r8d\n\t"
	 "jnz _l2a\n\t"
	 
	 "movq %%r14, %%rdi\n\t"
	 "addq %%r10, %%rdi\n\t"
	 "movq %%rdi, %%r14\n\t"
	 "decl %%ecx\n\t"
	 "jnz _l2\n\t"
	
	 "_exit0:\n\t"
	 "addq $64, %%rsp /* Free local value */\n\t"
	:
	: [src] "rm" (src), [dst] "rm" (dst), [pitch] "rm" (pitch),
	[ww] "rm" (ww), [rep2] "rm" (yrep2), [rep3] "rm" (yrep3)
	: "xmm0", "xmm1", "xmm2", "xmm3",
	"rax", "rcx", "rdi", "rsi", "r10", "r11", "r12", "r13", "r14" );
   
#endif
}



void pVram2RGB_x2_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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
   if(ww > (w / 2)) ww = w / 2;
   ww = (ww / 8) * 8;
   if(ww <= 0) return;


#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
//   yrep = yrep * 16.0f;

   yrep2 = yrep;

   d1 = (Uint32 *)((Uint8 *)dst + x * 2 * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   Scaler_DrawLine((v4hi *)d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}


#else 

void pVram2RGB_x2_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x2_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__