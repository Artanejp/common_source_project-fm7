/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


//#include "api_draw.h"
#include "api_vram.h"
//#include "sdl_cpuid.h"
#include "cache_wrapper.h"
extern Uint32 *rgbTTLGDI;


void CalcPalette_8colors(uint32 *p, uint32 index, uint8 r, uint8 g, uint8 b, uint8 a)
{
     Uint32 ds;

#ifdef AG_LITTLE_ENDIAN
	ds = r | (g << 8) | (b << 16) | 0xff000000;
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    _prefetch_data_write_permanent(rgbTTLGDI, sizeof(Uint32) * 8);
    p[index] = ds;
}

#if (__GNUC__ >= 4)
static void getvram_8_vec(uint8 *vram, Uint32 addr, v8hi_t *cbuf, uint32 offset, uint8 mask)
{
  uint8 r = 0;
  uint8 g = 0;
  uint8 b = 0

  g = vram[addr + offset * 2];
  r = vram[addr + offset];
  b = vram[addr];
  cbuf->v = aPlanes[B0 + b] |
    aPlanes[B1 + r] |
    aPlanes[B2 + g];
  return;
}

static inline scrntype putword8_vec_r(uint32 dat, uint8 *pal_r)
{
  scrntype d = 0;

  d = pal_r[dat];
  return d;
}

static inline scrntype putword8_vec_b(uint32 dat, uint8 *pal_b)
{
  scrntype d = 0;

  d = pal_b[dat];
  return d;
}

static inline scrntype putword8_vec_g(uint32 dat, uint8 *pal_g)
{
  scrntype d = 0;

  d = pal_g[dat];
  return d;
}


static inline scrntype  *putword8_vec(scrntype *disp, v8hi_t *c, Uint8 *pal_r, Uint8 *pal_g, Uint8 *pal_b, Uint8 *pal_l, uint8 mask)
{
  scrntype r[8], g[8], b[8];
  int i;
  uint32 *dat = (uint32 *)c;

  *c = *c & (v8hi_t) {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
  
  if((mask & 0x01) != 0) {
    for(i = 0; i < 8; i++) b[i] = 0x00;
  } else {
    for(i = 0; i < 8; i++) b[i] = putword_vec_b(dat[i], pal_b);
  }
  
  if((mask & 0x02) != 0) {
    for(i = 0; i < 8; i++) r[i] = 0x00;
  } else {
    for(i = 0; i < 8; i++) r[i] = putword_vec_r(dat[i], pal_r);
  }
  
  if((mask & 0x04) != 0) {
    for(i = 0; i < 8; i++) g[i] = 0x00;
  } else {
    for(i = 0; i < 8; i++) g[i] = putword_vec_g(dat[i], pal_g);
  }
  
  for(i = 0; i < 8; i++) disp[i] = RGB_COLOR(r[i], g[i], b[i]);
  
  return disp;
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

static void getvram_8(uint32 addr, uint32 *cbuf, uint32 offset, uint8 mask)
{
  uint8 r = 0;
  uint8 g = 0;
  uint8 b = 0;
   
  if((mask & 0x04) == 0) g = vram[addr + offset * 2];
  if((mask & 0x02) == 0) r = vram[addr + offset];
  if((mask & 0x01) == 0) b = vram[addr];
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
 * ybegin - yendの行を変換する
 */
void CreateVirtualVram8_Line(uint8 *src, scrntype *p, int ybegin, uint32 offset, uint8 mask, uint8 *pal_r, uint8 *pal_g, uint8 *pal_b, uint8 *pal_l)
{
    v8hi_t c;
    uint8 *disp = (uint8 *)p;
    Uint32 addr;
    int pitch;
    int xx;
    int yy = ybegin;
   
    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(scrntype) * 8;

    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
//       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 80;
//	   disp = (Uint8 *)(&p[yy * 640]);
	   for(xx = 0; xx < (80 / 8); xx ++) { 
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      disp += pitch;
	   }
//       }
       return;
     } else {
//       for(yy = ybegin; yy < yend; yy++) { 
           addr = yy * 80;
//	   disp = (Uint8 *)(&p[yy * 640]);
	   for(xx = 0; xx < (80 / 8); xx++) { 
	     getvram_8_vec(src, addr, &c, offset, mask);
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	     addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr , &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;

	      getvram_8_vec(src, addr, &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr , &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);	
	      addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr, &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr, &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr, &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;
	      
	      getvram_8_vec(src, addr, &c, offset, mask);
	      putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	      addr++;
	      disp += pitch;
	   }
	  
//       }
	return;
     }
}

/*
 * ybegin - yendの行を変換する
 */
void CreateVirtualVram8_WindowedLine(uint8 *vram_1, uint8 *vram_w, scrntype *p, int ybegin, int xbegin, int xend, uint32 offset, uint8 mask, uint8 *pal_r, uint8 *pal_g, uint8 *pal_b, uint8 *pal_l))
{
    v8hi_t c;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;
    int pitch;
    int xx;
    int yy = ybegin;
    
    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(scrntype) * 8;
    xbegin = xbegin % 80;
    xend = xend % 80;
    ybegin = ybegin % 400;
   
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       addr = yy * 80 + xbegin;
       disp = (Uint8 *)(&p[xbegin * 8]);
       for(xx = xbegin; xx < xend; xx ++) { 
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	  disp += pitch;
       }
       return;
     } else {

	addr = yy * 80 + xbegin;
	disp = (uint8 *)p;
	for(xx = 0; xx < xbegin; xx++) { 
	   getvram_8_vec(vram_1, addr, &c, offset, mask);
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	   addr++;
	   disp += pitch;
	}

	disp = (uint8 *)(&p[xbegin * 8]);
	for(xx = xbegin; xx < xend; xx++) { 
	   getvram_8_vec(vram_w, addr, &c, offset, mask);
	     putword8_vec((scrntype *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	   addr++;
	   disp += pitch;
	}
	
	disp = (uint8 *)(&p[xend * 8]);
	for(xx = xend; xx < 80; xx++) { 
	   getvram_8_vec(vram_1, addr, &c, offset, mask);
	     putword8_vec((Uint32 *)disp, &c, pal_r, pal_g, pal_b, pal_l);
	   addr++;
	   disp += pitch;
	}
	return;
     }
}

