/*
 * Zoom x2x2
 * (C) 2013 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;

void pVram2RGB_x2_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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
   int wodd;
   int i;
   int x = xbegin;
   int yrep2;
   unsigned  pitch;
   Uint32 black;
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   
   ww = xend - xbegin;
   if((ww * 2) > w) ww = w / 2;
   if(ww <= 0) return;
   wodd = ww % 8;
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   yrep2 = yrep;
   d1 = (Uint32 *)(dst + xbegin * 2 * Surface->format->BytesPerPixel);
   d2 = &src[xbegin + y * 640];

   pitch = Surface->pitch / sizeof(Uint32);
   { // Not thinking align ;-(
	
    int j;
    v4hi b2;
    v4hi b3;
    v4hi b4;
    v4hi b5;
    register v4hi bb;
    v4hi *b2p;
    Uint32 *d0;
      
    b = (v4hi *)d2;
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] = black;
       switch(yrep2) {
	case 0:
	case 1:
//	case 2:
	  d0 = d1;
	  for(xx = 0; xx < (ww - 1); xx += 8) {
	     d1 = d0;
	     b2p = (v4hi *)d1;
	     b2.i[0] = b2.i[1] = b[0].i[0];
	     b2.i[2] = b2.i[3] = b[0].i[1];
	     b3.i[0] = b3.i[1] = b[0].i[2];
	     b3.i[2] = b3.i[3] = b[0].i[3];

	     b4.i[0] = b4.i[1] = b[1].i[0];
	     b4.i[2] = b4.i[3] = b[1].i[1];
	     b5.i[0] = b5.i[1] = b[1].i[2];
	     b5.i[2] = b5.i[3] = b[1].i[3];
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     b2p[3] = b5;
	     d0 += 16;
	     b += 2;
	  }
	  if(wodd != 0) {
	     Uint32 *bp = (Uint32 *)b;
	     for(i = 0; i < wodd; i++) {
		*d0 = *bp;
		d0[1] = *bp;
		d0++;
		bp++;
	     }
	  }
	  break;
	default:
	  d0 = d1;
	  for(xx = 0; xx < (ww - 1); xx += 8){
	     d1 = d0;
	     b2.i[0] = b2.i[1] = b[0].i[0];
	     b2.i[2] = b2.i[3] = b[0].i[1];
	     b3.i[0] = b3.i[1] = b[0].i[2];
	     b3.i[2] = b3.i[3] = b[0].i[3];
	     
	     b4.i[0] = b4.i[1] = b[1].i[0];
	     b4.i[2] = b4.i[3] = b[1].i[1];
	     b5.i[0] = b5.i[1] = b[1].i[2];
	     b5.i[2] = b5.i[3] = b[1].i[3];

	     for(j = 0; j < yrep2; j++) {
		b2p = (v4hi *)d1;
		if(!bFullScan && (j >= (yrep2 >> 1))) {
		   b2p[0] = 
		   b2p[1] = 
		   b2p[2] = 
		   b2p[3] = bb;
		 } else {
		   b2p[0] = b2;
		   b2p[1] = b3;
		   b2p[2] = b4;
		   b2p[3] = b5;
		}
		d1 += pitch;
	     }
	     d0 += 16;
	     b += 2;
	  }
	  if(wodd != 0) {
	     Uint32 *bp = (Uint32 *)b;
	     for(i = 0; i < wodd; i++) {
		*d0 = *bp;
		d0[1] = *bp;
		d0[pitch] = *bp;
		d0[pitch + 1] = *bp;
		d0++;
		bp++;
	     }
	  }
	  break;
       }

   }
}

