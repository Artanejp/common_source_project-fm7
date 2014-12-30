/*
 * Zoom x1.25x2 i.e. 800x480.
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

extern void pVram2RGB_x125(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

#if defined(__SSE2__)

static void Scaler_DrawLine(Uint32 *dst, Uint32 *src, int ww, int repeat, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   Uint32 *b2p;
   v4hi r1, r2;
   v4hi *d0;
   v4hi *b;
   int pitch2;
#if AG_BIG_ENDIAN != 1
   const Uint32 bb = 0xff000000;
#else
   const Uint32 bb = 0x000000ff;
#endif
     
   if(repeat <= 0) return;
   b = (v4hi *)src;
   b2p = (Uint32 *)dst;
   pitch2 = pitch / sizeof(Uint32);
   if((bFullScan) || (repeat < 2)) {
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = (Uint32 *)dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 7654432100
	 for(yy = 0; yy < repeat; yy++) {
	       b2p[0] = b2p[1] = r1.i[0];
	       b2p[2] = r1.i[1];
	       b2p[3] = r1.i[2];
	       b2p[4] = r1.i[3];
	       b2p[5] = b2p[6] = r2.i[0];
	       b2p[7] = r2.i[1];
	       b2p[8] = r2.i[2];
	       b2p[9] = r2.i[3];
	       b2p = b2p + pitch2;
	 }
	 dst = dst + 10;
//	 b += 2;
      }
   } else {
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = (Uint32 *)dst;
	 r1 = *b++;
	 r2 = *b++;
	 // 76543210 -> 776655444332211000
	 // 76543210 -> 7654432100
	 for(yy = 0; yy < repeat - 1; yy++) {
	       b2p[0] = b2p[1] = r1.i[0];
	       b2p[2] = r1.i[1];
	       b2p[3] = r1.i[2];
	       b2p[4] = r1.i[3];
	       b2p[5] = b2p[6] = r2.i[0];
	       b2p[7] = r2.i[1];
	       b2p[8] = r2.i[2];
	       b2p[9] = r2.i[3];
	       b2p = b2p + pitch2;
	 }
	 b2p[0] = b2p[1] = b2p[2] = b2p[3] =
	 b2p[4] = b2p[5] = b2p[6] = b2p[7] =
	 b2p[8] = b2p[9] =
	   bb;
	 dst = dst + 10;
//	 b += 2;
      }
   }
   
}



void pVram2RGB_x125_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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

   d1 = (Uint32 *)((Uint8 *)dst + ((x * 10) / 8) * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   Scaler_DrawLine(d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}


#else 

void pVram2RGB_x125_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x125_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__