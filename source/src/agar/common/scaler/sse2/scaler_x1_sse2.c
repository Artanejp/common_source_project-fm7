/*
 * Zoom x1x1
 * (C) 2014 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;


static void Scaler_DrawLine(v4hi *dst, Uint32 *src, int ww, int repeat, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   register v4hi *b2p;
   register v4hi r1, r2;
   v4hi *d0;
   register v4hi *b;
   register v4hi bb2;
   register int pitch2;
#if AG_BIG_ENDIAN != 1
   const v4ui bb = {0xff000000, 0xff000000, 0xff000000, 0xff000000};
#else
   const v4ui bb = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};
#endif
     
   if(__builtin_expect((repeat < 0), 0)) return;
   b = (v4hi *)src;
   bb2.uv = bb;
   b2p = dst;
   pitch2 = pitch / sizeof(v4hi);
   if(bFullScan || (repeat < 2)) {
      if(__builtin_expect((repeat >= 2), 1)) {
	 for(xx = 0; xx < ww; xx += 8) {
	    b2p = dst;
	    r1 = b[0];
	    r2 = b[1];
	    for(yy = 0; yy < repeat; yy++) {
	       b2p[0] = r1;
	       b2p[1] = r2;
	       b2p = b2p + pitch2;
	    }
	 dst += 2;
	 b += 2;
	 }
      } else { // repeat == 1
	 for(xx = 0; xx < ww; xx += 8) {
	    b2p = dst;
	    b2p[0] = b[0];
	    b2p[1] = b[1];
	    dst += 2;
	    b += 2;
	 }
      }
   } else {
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = b[0];
	 r2 = b[1];
	 for(yy = 0; yy < repeat - 1; yy++) {
	    b2p[0] = r1;
	    b2p[1] = r2;
	    b2p = b2p + pitch2;
	 }
	 b2p[0] = bb2;
	 b2p[1] = bb2;
	 dst += 2;
	 b += 2;
      }
   }
   
}



      
void pVram2RGB_x1_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
{
   register v8hi_t *b;
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
   unsigned  pitch;
   int yrep2;
   int yrep3;
   if(Surface == NULL) return;

   w = Surface->w;
   h = Surface->h;
   
   ww = xend - xbegin;
   if(ww <= 0) return;
   yrep2 = yrep;
   d1 = (Uint32 *)(dst + x * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   
   Scaler_DrawLine((v4hi *)d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}

