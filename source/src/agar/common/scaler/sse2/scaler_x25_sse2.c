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

#if defined(__SSE2__)
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
   int pitch2;
   register int ip;
   v4hi r3v[5 * 80];
#if AG_BIG_ENDIAN != 1
   const v4ui bb = {0xff000000, 0xff000000, 0xff000000, 0xff000000};
   const v4ui order3 = (v4ui){3, 3, 4, 4};
#else
   const v4ui bb = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};
   const v4ui order3 = (v4ui){3, 3, 4, 4};
#endif
     
   if(repeat <= 0) return;
   b = (v4hi *)src;
   b2p = dst;
   pitch2 = pitch / sizeof(v4hi);
//   _prefetch_data_read_l2((void *)src, sizeof(Uint32) * ww);
   _prefetch_data_write_l1((void *)r3v, sizeof(r3v));
   if(__builtin_expect(((bFullScan) || (repeat < 2)), 1)) {
      ip = 0;
      for(xx = 0; xx < ww; xx += 8) {
	 b2p = dst;
	 r1 = b[0];
	 r2 = b[1];
	 // 76543210 -> 77666554443322211000
	 r3v[ip + 0].uv = __builtin_ia32_pshufd(r1.uv, 0b01000000);
	 r3v[ip + 1].uv = __builtin_ia32_pshufd(r1.uv, 0b10101001);
	 r3v[ip + 2] = (v4hi)__builtin_shuffle(r1.uv, r2.uv, order3);
	 r3v[ip + 3].uv = __builtin_ia32_pshufd(r2.uv, 0b10010100);
	 r3v[ip + 4].uv = __builtin_ia32_pshufd(r2.uv, 0b11111010);
	 ip += 5;
	 b += 2;
      }
      
      for(yy = 0; yy < repeat; yy++) {
	 //	    _prefetch_data_write_l2((void *)b2p, sizeof(v4hi) * 5);
	 memcpy((void *)b2p, (void *)r3v, sizeof(v4hi) * ip);
	 b2p = b2p + pitch2;
      }
   } else {
      ip = 0;
      for(xx = 0; xx < ww; xx += 8) {
	 yy = 0;
//	 b2p = dst;
	 r1 = b[0];
	 r2 = b[1];
	 // 76543210 -> 77666554443322211000
	 r3v[ip + 0].uv = __builtin_ia32_pshufd(r1.uv, 0b01000000);
	 r3v[ip + 1].uv = __builtin_ia32_pshufd(r1.uv, 0b10101001);
	 r3v[ip + 2] = (v4hi)__builtin_shuffle(r1.uv, r2.uv, order3);
	 r3v[ip + 3].uv = __builtin_ia32_pshufd(r2.uv, 0b10010100);
	 r3v[ip + 4].uv = __builtin_ia32_pshufd(r2.uv, 0b11111010);
	 ip += 5;
	 b += 2;
      }
      b2p = dst;
      for(yy = 0; yy < repeat - 1; yy++) {
//	    _prefetch_data_write_l2((void *)b2p, sizeof(v4hi) * 5);
	    memcpy((void *)b2p, (void *)r3v, ip * sizeof(v4hi));
	    b2p = b2p + pitch2;
      }
//	 _prefetch_data_write_l2((void *)b2p, sizeof(v4hi) * 5);
      for(xx = 0; xx < ip; xx++) b2p[xx].uv = bb;
   }
   
}



void pVram2RGB_x25_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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


#else 

void pVram2RGB_x25_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x25_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__