/*
 * Zoom x0.5
 * (C) 2014 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;


void pVram2RGB_x05_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
{
   v8hi_t *b;
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w;
   int h;
   int yy;
   int yy2;
   int xx;
   int hh;
   int ww;
   int i;
   int pitch;
   int yrep2 = yrep;
   v8hi_t rmask1, gmask1, bmask1, amask1;
   v4hi rmask2, gmask2, bmask2, amask2;
   Uint32 black;
   AG_Surface *Surface = GetDrawSurface();
    
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   pitch = Surface->pitch / sizeof(Uint32);
   if(yrep2 <= 0) yrep2 = 1; // Okay?
   
#if AG_BIG_ENDIAN != 1
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0x000000ff;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x0000ff00;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x00ff0000;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0xff000000;

   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0xff000000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x00ff0000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x0000ff00;
   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0x000000ff;

#else
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0xff000000;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x00ff0000;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x0000ff00;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0x000000ff;

   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0xff000000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x00ff0000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x0000ff00;
   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0x000000ff;
#endif
   d1 = (Uint32 *)(dst + (xbegin >> 1) * Surface->format->BytesPerPixel);
   p = &src[xbegin + y * 640];
   if(((xbegin >>1) + 4) >= w) {
	Uint32 amask, rmask, gmask, bmask;
        Uint32 bd1, bd2;
        Uint32 r, g, b, a;
        int j;

#if AG_BIG_ENDIAN != 1
      amask = 0xff000000;
      bmask = 0x00ff0000;
      gmask = 0x0000ff00;
      rmask = 0x000000ff;
#else
      rmask = 0xff000000;
      gmask = 0x00ff0000;
      bmask = 0x0000ff00;
      amask = 0x000000ff;
#endif
      ww = (xend - xbegin) / 2;
      if(ww > w) ww = w;
            
      for(xx = 0; xx < ww; xx++) {
	 bd1 = p[0];
	 bd2 = p[1];
	 r = (((bd1 & rmask) >> 1) + ((bd2 & rmask) >> 1)) & rmask;
	 g = (((bd1 & gmask) >> 1) + ((bd2 & gmask) >> 1)) & gmask;
	 b = (((bd1 & bmask) >> 1) + ((bd2 & bmask) >> 1)) & bmask;
	 d2 = &d1[xx];
	 for(j = 0; j < yrep2; j++) {
	    *d2 = r | g  | b | amask;
	    d2 += pitch;
	 }
	 p += 2;
      }
      return;
   }
   

     {
      v4hi *pd;
      v4hi cr, cg, cb, cd;
      v8hi_t *b;
      v8hi_t br,bg, bb;
      Uint32 *d0;
	
      ww = (xend - xbegin) / 2;
      if(ww > w) ww = w;
      d0 = d1;
      for(xx = 0; xx < ww; xx++) {
	 d1 = d0;
 	 b = (v8hi_t *)p;
	 br.v = b->v & rmask1.v;
	 bg.v = b->v & gmask1.v;
	 bb.v = b->v & bmask1.v;
	 cr.i[0] = (br.i[0] >> 1) + (br.i[1] >> 1);
	 cr.i[1] = (br.i[2] >> 1) + (br.i[3] >> 1);
	 cr.i[2] = (br.i[4] >> 1) + (br.i[5] >> 1);
	 cr.i[3] = (br.i[6] >> 1) + (br.i[7] >> 1);

	 cb.i[0] = (bb.i[0] + bb.i[1]) >> 1;
	 cb.i[1] = (bb.i[2] + bb.i[3]) >> 1;
	 cb.i[2] = (bb.i[4] + bb.i[5]) >> 1;
	 cb.i[3] = (bb.i[6] + bb.i[7]) >> 1;

	 cg.i[0] = (bg.i[0] + bg.i[1]) >> 1;
	 cg.i[1] = (bg.i[2] + bg.i[3]) >> 1;
	 cg.i[2] = (bg.i[4] + bg.i[5]) >> 1;
	 cg.i[3] = (bg.i[6] + bg.i[7]) >> 1;
	 cr.v = cr.v & rmask2.v;
	 cg.v = cg.v & gmask2.v;
	 cb.v = cb.v & bmask2.v;
	 cd.v = cr.v | cg.v | cb.v | amask2.v;
	 for(i = 0; i < yrep2; i++) {
	    pd = (v4hi *)d1;
	    *pd = cd;
	    d1 += pitch;
	 }
	 d0 += 4;
	 p += 8;
      }
   }
}






