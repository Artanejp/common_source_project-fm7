/*
 * api_vram4096.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "api_draw.h"
#include "api_vram.h"
#include "cache_wrapper.h"

//Uint8 *vram_pb;
//Uint8 *vram_pr;
//Uint8 *vram_pg;



static inline void putword2_vec(Uint32 *disp, v8hi_t cbuf)
{
   v8hi_t *dst = (v8hi_t *)disp;
   v8hi_t r1;
   register int j;
   _prefetch_data_write_l1(disp, sizeof(Uint32) * 8); // 4 * 8  = 32bytes.
   for(j = 0; j < 8; j++) dst->i[j] = rgbAnalogGDI[cbuf.i[j]];
}

static inline v8hi_t getvram_4096_vec(Uint32 addr)
{
    v8hi_t cbuf;
    uint8_t r0, r1, r2, r3;
    uint8_t g0, g1, g2, g3;
    uint8_t b0, b1, b2, b3;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
    g3 = vram_pg[addr + 0x00000];
    g2 = vram_pg[addr + 0x02000];
    g1 = vram_pg[addr + 0x04000];
    g0 = vram_pg[addr + 0x06000];
    cbuf.v = 
        aPlanes[G0 + g0] |
        aPlanes[G1 + g1] |
        aPlanes[G2 + g2] |
        aPlanes[G3 + g3] ;

   
    r3 = vram_pr[addr + 0x00000];
    r2 = vram_pr[addr + 0x02000];
    r1 = vram_pr[addr + 0x04000];
    r0 = vram_pr[addr + 0x06000];
    cbuf.v = cbuf.v |
        aPlanes[R0 + r0] |
        aPlanes[R1 + r1] |
        aPlanes[R2 + r2] |
        aPlanes[R3 + r3] ;

    b3 = vram_pb[addr + 0x00000];
    b2 = vram_pb[addr + 0x02000];
    b1 = vram_pb[addr + 0x04000];
    b0 = vram_pb[addr + 0x06000];
    cbuf.v = cbuf.v |
        aPlanes[B0 + b0] |
        aPlanes[B1 + b1] |
        aPlanes[B2 + b2] |
        aPlanes[B3 + b3] ;
   return cbuf;
}

/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram4096_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mode)
{
//    Uint32 c[8];
    register v8hi_t c;
    Uint32 *disp = p;
    Uint32 addr;
    register int i;

//   for(i = 0; i < 4096; i++) __builtin_prefetch(&rgbAnalogGDI[i], 0, 0);
    addr = y * 40 + x;
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
//       disp += pitch;
    } else {
       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       c = getvram_4096_vec(addr);
       putword2_vec((Uint32 *)disp,  c);
    }
   
}
   

/*
 * 1LineのピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram4096_Line_SSE2(Uint32 *p, int ybegin, int yend, int mode)
{
//    Uint32 c[8];
    register v8hi_t c;
    Uint8 *disp;
    Uint32 addr;
    int yy;
    int xx;
    const int pitch = sizeof(Uint32) * 8;
    int i;

//    for(i = 0; i < 4096; i++) __builtin_prefetch(&rgbAnalogGDI[i], 0, 0);
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       for(yy = ybegin; yy < yend; yy++) {
	  addr = yy * 40;
	  disp = (Uint8 *)p + (pitch * addr);
	  for(xx = 0; xx < (40 / 8); xx++) {
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	  }
       }
    } else {
       for(yy = ybegin; yy < yend; yy++) {
	  addr = yy * 40;
	  disp = (Uint8 *)p + (pitch * addr);
	  for(xx = 0; xx < (40 / 8); xx++) {
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	     c = getvram_4096_vec(addr);
	     putword2_vec((Uint32 *)disp,  c);
	     disp +=  pitch;
	     addr++;
	  }
       }
    } 
}

/*
 * ybegin - yendの行を変換する
 */
void CreateVirtualVram4096_WindowedLine_SSE2(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode)
{
#if (__GNUC__ >= 4)   
    register v8hi_t c;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;
    int pitch;
    int xx;
    int yy;
   
    if(p == NULL) return;
    pitch = sizeof(Uint32) * 8;

    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 40 + xbegin;
	   disp = (Uint8 *)(&p[yy * 320 + xbegin]);
	   for(xx = xbegin; xx < xend; xx ++) { 
	      putword2_vec((Uint32 *)disp, c);
	      disp += pitch;
	   }
       }
       return;
     } else {
       int xs =  (xend - xbegin) / 8;
       int xs2 = (xend - xbegin) % 8;
       int xx2;
       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 40 + xbegin;
	   disp = (Uint8 *)(&p[yy * 320 + xbegin]);
	   xx = xbegin;
	   for(xx2 = 0; xx2 < xs; xx2++) {
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
	      
	      xx += 8;
	   }
	   if(xs2 <= 0) continue;
	   
	   for(;xx < xend; xx++) { 
	      c = getvram_4096_vec(addr);
	      putword2_vec((Uint32 *)disp, c);
	      addr++;
	      disp += pitch;
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
      addr = y * 40 + xbegin;
      disp = (Uint8 *)(&p[yy * 320 + xbegin]);
      for(xx = xbegin; xx < xend; xx++) {
	 getvram_8(addr, c);
	 putword8((Uint32 *)disp, c, pal);
	 addr++;
	 disp += pitch;
      }
   }
#endif   
}

Api_Vram_FuncList api_vram4096_sse2 = {
   CreateVirtualVram4096_1Pcs_SSE2,
   CreateVirtualVram4096_Line_SSE2,
   CreateVirtualVram4096_WindowedLine_SSE2
};
