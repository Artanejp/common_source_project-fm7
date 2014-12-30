/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "api_draw.h"
#include "api_vram.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern void CreateVirtualVram8_WindowedLine(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode);


#if (__GNUC__ >= 4)

static inline v8hi_t getvram_8_vec(Uint32 addr)
{
    register uint8_t r, g, b;
    v8hi_t ret;
//    volatile v4hi cbuf __attribute__((aligned(32)));
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
    g = vram_pg[addr];
    r = vram_pr[addr];
    b = vram_pb[addr];

   ret.v   = aPlanes[B0 + b] |
              aPlanes[B1 + r] |
              aPlanes[B2 + g];
   return ret;
}

static void  putword8_vec(Uint32 *disp, v8hi_t c, Uint32 *pal)
{
   v8hi_t *p = (v8hi_t *)disp;
   register int j;

//   if(disp == NULL) return;

   // recommand -finline-loop
#ifdef __x86_64__
   if((pal == NULL) || (disp == NULL))return;
   asm ("movq %[c], %%r8\n\t"
	"movdqa  0(%%r8), %%xmm0\n\t"
	"movdqa 16(%%r8), %%xmm1\n\t"
	"movq %[pal], %%r8\n\t"
	"movq %[disp], %%rdi\n\t"
	"movl $7, %%r9d\n\t"
	"movd %%r9d, %%xmm2\n\t"
	"pshufd $0b00000000, %%xmm2, %%xmm2\n\t"
	"pand %%xmm2, %%xmm0\n\t"
	"pand %%xmm2, %%xmm1\n\t"
	"pshufd $0b00011011, %%xmm0, %%xmm0\n\t"
	"pshufd $0b00011011, %%xmm1, %%xmm1\n\t"
	
	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"movdqa %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"
	
	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"

	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"

	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"/* psrldq $4, %%xmm0 */\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"/* pslldq $4, %%xmm3 */\n\t"
	"movdqu %%xmm3, 0(%%rdi)\n\t"
	
	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"movdqa %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"
	
	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"

	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"

	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"/* psrldq $4, %%xmm1 */\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"/* pslldq $4, %%xmm5 */\n\t"
	"movdqu %%xmm5, 16(%%rdi)\n\t"
	:
	: [c] "rm" (&c), [disp] "rm" (disp), [pal] "rm" (pal)
	: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
	  "r8", "r9", "r10", "rdi");
#else
   v8hi_t tmp;
   if((pal == NULL) || (disp == NULL))return;
   c.vv &= (v8ii){7, 7, 7, 7, 7, 7, 7, 7,};
   for(j = 0; j < 8; j++) {
      tmp.i[j] = pal[c.i[j]];
   }
   *p = tmp;
#endif   
}


static  void getputvram_8_vec(Uint32 addr, Uint32 *disp, Uint32 *pal)
{
#ifdef __x86_64__
   if((pal == NULL) || (disp == NULL)) return;
   asm (
	"movq %[vram_pg], %%r9\n\t"
	"movq %[vram_pr], %%r10\n\t"
	"movq %[vram_pb], %%r11\n\t"
		 
	"movb 0(%%r11), %%r13b\n\t"
	"movb 0(%%r10), %%r14b\n\t"
	"movb 0(%%r9), %%r15b\n\t"
	"andq $0xff, %%r13\n\t"
	"andq $0xff, %%r14\n\t"
	"andq $0xff, %%r15\n\t"
	"shlq $5, %%r13\n\t"
	"shlq $5, %%r14\n\t"
	"shlq $5, %%r15\n\t"
	"addq $0x2000, %%r14 /* 256 * 32 */\n\t"
	"addq $0x4000, %%r15 /* 512 * 32 */\n\t"
	
	"movq %[pal], %%r8\n\t"
	"movq %[disp], %%rdi\n\t"
	"movq %[aPlanes], %%r12\n\t"
	
	"movdqa 0(%%r12, %%r13), %%xmm0\n\t"
	"movdqa 0(%%r12, %%r14), %%xmm1\n\t"
	"movdqa 0(%%r12, %%r15), %%xmm2\n\t"
	"por %%xmm1, %%xmm0\n\t"
	"por %%xmm2, %%xmm0\n\t"

	"movdqa 16(%%r12, %%r13), %%xmm1\n\t"
	"movdqa 16(%%r12, %%r14), %%xmm4\n\t"
	"movdqa 16(%%r12, %%r15), %%xmm5\n\t"
	"por %%xmm4, %%xmm1\n\t"
	"por %%xmm5, %%xmm1\n\t"
	
	"movl $0x07, %%eax\n\t"
	"movd %%eax, %%xmm2\n\t"
	"pshufd $0b00000000, %%xmm2, %%xmm2\n\t"
	"pand %%xmm2, %%xmm0\n\t"
	"pand %%xmm2, %%xmm1\n\t"
	"pshufd $0b00011011, %%xmm0, %%xmm0\n\t"
	"pshufd $0b00011011, %%xmm1, %%xmm1\n\t"
	"pxor %%xmm2, %%xmm2\n\t"
	"pxor %%xmm4, %%xmm4\n\t"

	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"movdqa %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"
	
	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"

	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"psrldq $4, %%xmm0\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"pslldq $4, %%xmm3\n\t"

	"movd %%xmm0, %%r9d\n\t"
	"movd 0(%%r8, %%r9, 4), %%xmm2\n\t"
	"por    %%xmm2, %%xmm3\n\t"
	"movdqu %%xmm3, 0(%%rdi)\n\t"
	
	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"movdqa %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"
	
	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"

	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"psrldq $4, %%xmm1\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"pslldq $4, %%xmm5\n\t"

	"movd %%xmm1, %%r10d\n\t"
	"movd 0(%%r8, %%r10, 4), %%xmm4\n\t"
	"por    %%xmm4, %%xmm5\n\t"
	"movdqu %%xmm5, 16(%%rdi)\n\t"
	:
	: [aPlanes] "rm" (aPlanes),
	  [disp] "rm" (disp), 	  [pal] "rm" (pal), 
	  [vram_pg] "rm" (&vram_pg[addr]), [vram_pr] "rm" (&vram_pr[addr]), [vram_pb] "rm" (&vram_pb[addr])
	: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
	  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	  "rdi");
#else
   v8hi_t c;
   register uint8_t g, r, b;
   int j;
   v8hi_t *p = (v8hi_t *)disp;
   v8hi_t tmp;
   
   if((pal == NULL) || (p == NULL))return;
   
   g = vram_pg[addr];
   r = vram_pr[addr];
   b = vram_pb[addr];

   c.v   = aPlanes[B0 + b] |
           aPlanes[B1 + r] |
           aPlanes[B2 + g];
   c.vv &= (v8ii){7, 7, 7, 7, 7, 7, 7, 7,};
   for(j = 0; j < 8; j++) {
      tmp.i[j] = pal[c.i[j]];
   }
   *p = tmp;
#endif
}




#else
static inline void planeto8(Uint32 *c, uint8_t r, unit8_t g, uint8_t b)
{
   Uint8 mask;
   
   mask = 0x80;
   c[0] = ((r & mask) >> 6) | ((g & mask) >> 5) || ((b & mask) >> 7);
   mask >>= 1;
   c[1] = ((r & mask) >> 5) | ((g & mask) >> 4) || ((b & mask) >> 6);
   mask >>= 1;
   c[2] = ((r & mask) >> 4) | ((g & mask) >> 3) || ((b & mask) >> 5);
   mask >>= 1;
   c[3] = ((r & mask) >> 3) | ((g & mask) >> 2) || ((b & mask) >> 4);
   mask >>= 1;
   c[4] = ((r & mask) >> 2) | ((g & mask) >> 1) || ((b & mask) >> 3);
   mask >>= 1;
   c[5] = ((r & mask) >> 1) | (g & mask) || ((b & mask) >> 2);
   mask >>= 1;
   c[6] = (r & mask) | ((g & mask) << 1) || ((b & mask) >> 1);
   mask >>= 1;
   c[7] = ((r & mask) << 1) | ((g & mask) << 2) || (b & mask);
   mask >>= 1;
}

static void getvram_8(Uint32 addr, Uint32 *cbuf)
{
    uint8_t r, g, b;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
   
   g = vram_pg[addr];
   r = vram_pr[addr];
   b = vram_pb[addr];
   planeto8(cbuf, r, g, b);
  
   return;
}

static inline void  putword8(Uint32 *disp, Uint32 *c, Uint32 *pal)
{

   Uint32 *r1 = disp;

   r1[0] = pal[c[0] & 7]; // ?!
   r1[1] = pal[c[1] & 7];
   r1[2] = pal[c[2] & 7];
   r1[3] = pal[c[3] & 7];
   r1[4] = pal[c[4] & 7];
   r1[5] = pal[c[5] & 7];
   r1[6] = pal[c[6] & 7];
   r1[7] = pal[c[7] & 7];
}

#endif // __GNUC__ >= 4



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mode)
{
#if (__GNUC__ >= 4)   
    register v8hi_t c;
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    register v8hi_t *disp =(v8hi_t *) p;
    register Uint32 addr;
    register int i;
    pitch = pitch / (sizeof(v8hi_t) / sizeof(Uint32));

    if((p == NULL) || (pal == NULL)) return;
//    for(i = 0; i < 8; i++) __builtin_prefetch(&pal[i], 0, 0); // パレットテーブルをキャッシュに読み込ませておく
    addr = y * 80 + x;
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;

       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
//       disp++;
       return;
     } else {
#if 0
       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp, c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
#else
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;

	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
	addr += 80;
	disp += pitch;
	
	getputvram_8_vec(addr, disp, pal);
//	addr += 80;
//	disp += pitch;
#endif
     }
#else 
    Uint32 c[8];
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    v8hi_t *disp =(V8hi_t *) p;

    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(Uint32) * 8;
    addr = y * 80 + x;

    // Loop廃止(高速化)
   getvram_8(addr, c);
   putword8((Uint32 *)disp, c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr , c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr , c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   //    addr += 80;
   //    disp++;
     
#endif   
}

/*
 * ybegin - yendの行を変換する
 */
void CreateVirtualVram8_Line_SSE2(Uint32 *p, int ybegin, int yend, int mode)
{
#if (__GNUC__ >= 4)   
    register v8hi_t c;
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    register v8hi_t *disp =(v8hi_t *) p;
    register Uint32 addr;
    const int pitch = sizeof(Uint32) * 8;
    int xx;
    int yy;
    register int i;
   
    if((p == NULL) || (pal == NULL)) return;

//    for(i = 0; i < 8; i++) __builtin_prefetch(&pal[i], 0, 0); // パレットテーブルをキャッシュに読み込ませておく
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       for(yy = ybegin; yy < yend; yy++) { 
           addr = ybegin * 80;
	   for(xx = 0; xx < (80 / 8); xx ++) { 
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	   }
       }
       return;
     } else {
       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 80;
	   for(xx = 0; xx < (80 / 8); xx++) { 
#if 1
	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;

	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;

	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;

	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;

	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;
	      
	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;
	      
	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;

	      getputvram_8_vec(addr, (Uint32 *)disp, pal);
	      addr++;
	      disp++;
#else
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;

	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp,  c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp,  c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp,  c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp,  c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp,  c, pal);
	      addr++;
	      disp++;
#endif
	   }
	  
       }
	return;
     }
 #else 
    Uint32 c[8];
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    Uint8 *disp =(Uint8 *) p;
    int xx;
    int yy;

    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(Uint32) * 8;
   for(yy = ybegin; yy < yend; yy++) {  
      addr = y * 80;
      for(xx = 0; xx < (80 / 8) ; xx++) {
	   
	 // Loop廃止(高速化)
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp, c, pal);
	 addr++;
	 disp++;
   
	 getvram_8(addr , c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr++;
	 disp++;
	 
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
   
	 getvram_8(addr , c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
   
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
   
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
   
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
   
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp,  c, pal);
	 addr += 1;
	 disp++;
      }
   }
   
     
#endif   
}
/*
 * ybegin - yendの行を変換する
 */
void CreateVirtualVram8_WindowedLine_SSE2(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode)
{
#if (__GNUC__ >= 4)   
    register v8hi_t c;
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;
    int pitch;
    int xx;
    int yy;
   
    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(Uint32) * 8;

    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.vv = (v8ii){0,0,0,0,0,0,0,0};
       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 80 + xbegin;
	   disp = (Uint8 *)(&p[yy * 640 + xbegin]);
	   for(xx = xbegin; xx < xend; xx ++) { 
	      putword8_vec((Uint32 *)disp,  c, pal);
	      disp++;
	   }
       }
       return;
     } else {
       int xs =  (xend - xbegin) / 8;
       int xs2 = (xend - xbegin) % 8;
       int xx2;
       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 80 + xbegin;
	   disp = (Uint8 *)(&p[yy * 640 + xbegin]);
	   xx = xbegin;
	   for(xx2 = 0; xx2 < xs; xx2++) {
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	      xx += 8;
	   }
	   if(xs2 <= 0) continue;
	   for(; xx < xend; xx++) { 
	      c = getvram_8_vec(addr);
	      putword8_vec((Uint32 *)disp, c, pal);
	      addr++;
	      disp++;
	   }
       }
	return;
     }
 #else 
    Uint32 c[8];
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    Uint8 *disp =(Uint8 *) p;
    int xx;
    int yy;

    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(Uint32) * 8;
    for(yy = ybegin; yy < yend; yy++) {  
      addr = y * 80 + xbegin;
      disp = (Uint8 *)(&p[yy * 640 + xbegin]);
      for(xx = xbegin; xx < xend; xx++) {
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp, c, pal);
	 addr++;
	 disp++;
      }
   }
#endif   
}

Api_Vram_FuncList api_vram8_sse2 = {
   CreateVirtualVram8_1Pcs_SSE2,
   CreateVirtualVram8_Line_SSE2,
   CreateVirtualVram8_WindowedLine
};
