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

extern void pVram2RGB_x25(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

static void Scaler_DrawLine(v4hi *dst, Uint32 *src, int ww, int repeat, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   v4hi *b2p;
   v4hi r1, r2;
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
      volatile v4hi r3, r4, r5, r6, r7;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 77666554443322211000
	 r3.uv = (v4ui){r1.i[0], r1.i[0], r1.i[0], r1.i[1]};  
	 r4.uv = (v4ui){r1.i[1], r1.i[2], r1.i[2], r1.i[2]};  
	 r5.uv = (v4ui){r1.i[3], r1.i[3], r2.i[0], r2.i[0]};  
	 r6.uv = (v4ui){r2.i[0], r2.i[1], r2.i[1], r2.i[2]};  
	 r7.uv = (v4ui){r2.i[2], r2.i[2], r2.i[3], r2.i[3]};  
	 for(yy = 0; yy < repeat; yy++) {
	    *b2p++ = r3;
	    *b2p++ = r4;
	    *b2p++ = r5;
	    *b2p++ = r6;
	    *b2p++ = r7;
	    b2p = b2p + (pitch2 - 5);
	 }
	 dst += 5;
//	 b += 2;
      }
   } else {
      volatile v4hi r3, r4, r5, r6, r7;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 77666554443322211000
	 r3.uv = (v4ui){r1.i[0], r1.i[0], r1.i[0], r1.i[1]};  
	 r4.uv = (v4ui){r1.i[1], r1.i[2], r1.i[2], r1.i[2]};  
	 r5.uv = (v4ui){r1.i[3], r1.i[3], r2.i[0], r2.i[0]};  
	 r6.uv = (v4ui){r2.i[0], r2.i[1], r2.i[1], r2.i[2]};  
	 r7.uv = (v4ui){r2.i[2], r2.i[2], r2.i[3], r2.i[3]};  
	 for(yy = 0; yy < repeat - 1; yy++) {
	    *b2p++ = r3;
	    *b2p++ = r4;
	    *b2p++ = r5;
	    *b2p++ = r6;
	    *b2p++ = r7;
	    b2p = b2p + (pitch2 - 5);
	 }
	 b2p[0].uv = bb;
	 b2p[1].uv = bb;
	 b2p[2].uv = bb;
	 b2p[3].uv = bb;
	 b2p[4].uv = bb;
	 dst += 5;
//	 b += 2;
      }
   }
   
}



void pVram2RGB_x25_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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

   d1 = (Uint32 *)((Uint8 *)dst + ((x * 20) / 16) * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   Scaler_DrawLine((v4hi *)d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}
