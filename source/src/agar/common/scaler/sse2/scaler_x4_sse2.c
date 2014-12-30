/*
 * Zoom x4x4
 * (C) 2013 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 *  2013-09-17 Move from scaler/generic/scaler_x4.c
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;

#if defined(__SSE2__)
void pVram2RGB_x4_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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
//   AG_SurfaceLock(Surface);
   w = Surface->w;
   h = Surface->h;
   
   ww = xend - xbegin;
   if(ww > (w / 4)) ww = w / 4;
   ww = ww - 7;
   if(ww <= 0) return;
//   if(yrep < 2) {
//      if(y >= h) return;
//   } else {
//      if(y >= (h / (yrep >> 1))) return;/
//   }
   
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   yrep2 = yrep;
   d1 = (Uint32 *)(dst+ x * 4 * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   
   pitch = Surface->pitch / sizeof(Uint32);
   { // Not thinking align ;-(
	
    int j;
    register v4hi b2, b3, b4, b5, b6, b7, b8, b9;
    register v4hi bb;
    register v4hi bx0, bx1;
    v4hi *b2p;
    Uint32 *d0;
      
    b = (v4hi *)d2;
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] = black;
    //if((((y * yrep2) % 16) == 0) && ((yrep2 % 16) != 0)) yrep2 += 16;
    //yrep2 >>= 4;
       switch(yrep2) {
	case 0:
	case 1:
//	case 2:
	  _prefetch_data_write_l2(d1, sizeof(v4hi) * 8 * ww);
	  for(xx = 0; xx < ww; xx += 8) {
	     b2p = (v4hi *)d1;
	     bx0 = b[0];
	     bx1 = b[1];
	     b2.vv = __builtin_ia32_pshufd(bx0.vv, 0x00);
	     b3.vv = __builtin_ia32_pshufd(bx0.vv, 0x55);
	     b4.vv = __builtin_ia32_pshufd(bx0.vv, 0xaa);
	     b5.vv = __builtin_ia32_pshufd(bx0.vv, 0xff);

	     b6.vv = __builtin_ia32_pshufd(bx1.vv, 0x00);
	     b7.vv = __builtin_ia32_pshufd(bx1.vv, 0x55);
	     b8.vv = __builtin_ia32_pshufd(bx1.vv, 0xaa);
	     b9.vv = __builtin_ia32_pshufd(bx1.vv, 0xff);
	     
	     *b2p++ = b2;
	     *b2p++ = b3;
	     *b2p++ = b4;
	     *b2p++ = b5;
	     *b2p++ = b6;
	     *b2p++ = b7;
	     *b2p++ = b8;
	     *b2p++ = b9;
	     d1 += 32;
	     b += 2;
	  }
	  break;
	default:
	  d0 = d1;
	  _prefetch_data_write_l2(d1, sizeof(v4hi) * 8 * ww);
	  for(xx = 0; xx < ww; xx += 8){
	     d1 = d0;
	     b2p = (v4hi *)d1;
	     bx0 = b[0];
	     bx1 = b[1];
	     b2.vv = __builtin_ia32_pshufd(bx0.vv, 0x00);
	     b3.vv = __builtin_ia32_pshufd(bx0.vv, 0x55);
	     b4.vv = __builtin_ia32_pshufd(bx0.vv, 0xaa);
	     b5.vv = __builtin_ia32_pshufd(bx0.vv, 0xff);

	     b6.vv = __builtin_ia32_pshufd(bx1.vv, 0x00);
	     b7.vv = __builtin_ia32_pshufd(bx1.vv, 0x55);
	     b8.vv = __builtin_ia32_pshufd(bx1.vv, 0xaa);
	     b9.vv = __builtin_ia32_pshufd(bx1.vv, 0xff);
	     
	     for(j = 0; j < yrep2; j++) {
		b2p = (v4hi *)d1;
		_prefetch_data_write_l2(d1, sizeof(v4hi) * 8);
		if(!bFullScan && (j >= (yrep2 - 1))) {
		   b2p[0] = 
		   b2p[1] = 
		   b2p[2] = 
		   b2p[3] = 
		   b2p[4] = 
		   b2p[5] = 
		   b2p[6] = 
		   b2p[7] = bb;
		 } else {
		    b2p[0] = b2;
		    b2p[1] = b3;
		    b2p[2] = b4;
		    b2p[3] = b5;
		    b2p[4] = b6;
		    b2p[5] = b7;
		    b2p[6] = b8;
		    b2p[7] = b9;
		}
		d1 += pitch;
	     }
	     d0 += 32;
	     b += 2;
	  }
	  break;
       }
   }
//   AG_SurfaceUnlock(Surface);
}


#else // NON-SSE2
void pVram2RGB_x4_SSE2_Line(Uint32 *src, int xbegin, int xend, int y, float yrep)
{
   pVram2RGB_x4_Line(Uint32 *src, xbegin, xend, int y, yrep);
}
#endif