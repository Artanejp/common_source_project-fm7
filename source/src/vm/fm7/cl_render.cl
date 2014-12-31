//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: GPLv2


#if (_CL_KERNEL_LITTLE_ENDIAN==1)
#define  rmask 0x000000ff
#define  gmask 0x0000ff00
#define  bmask 0x00ff0000
#define  amask 0xff000000
#define  rshift 0
#define  gshift 8
#define  bshift 16
#define  ashift 24
#else
#define  amask 0x000000ff
#define  gmask 0x00ff0000
#define  bmask 0x0000ff00
#define  rmask 0xff000000
#define  rshift 24
#define  gshift 16
#define  bshift 8
#define  ashift 0
#endif

inline uint8 putpixel(uint8 n, uint8 abuf)
{
  uint8 ret;
  ret = n | abuf;
  return ret;
}


struct apalettetbl_t {
   uchar line_h;
   uchar line_l;
   uchar mpage;
   uchar r_4096[4096];
   uchar g_4096[4096];
   uchar b_4096[4096];
} __attribute__((packed));

struct dpalettetbl_t {
   uchar line_h;
   uchar line_l;
   uchar mpage;
   uchar tbl[8];
}__attribute__((packed));

struct palettebuf_t {
   uchar alines_h;
   uchar alines_l;
   uchar dlines_h;
   uchar dlines_l;
   struct apalettetbl_t atbls[200];
   struct dpalettetbl_t dtbls[400];
}__attribute__((packed));
   
uint4 ttlpalet2rgb(__global uchar *pal, uint index)
{
    uchar dat = pal[index];
    uint4 ret;

    ret.s2 = ((dat & 0x01) == 0)?0:255; // B
    ret.s0 = ((dat & 0x02) == 0)?0:255; // R
    ret.s1 = ((dat & 0x04) == 0)?0:255; // G
    ret.s3 = 255; // A
    return ret;
}


__kernel void CreateTable(__global ushort8 *table, int pages)
{
  int i;
  int j;
  ushort8 v;
  for(j = 0; j < 256; j++) {
     v.s0 = (j & 0x80) >> 7;
     v.s1 = (j & 0x40) >> 6;
     v.s2 = (j & 0x20) >> 5;
     v.s3 = (j & 0x10) >> 4;
     v.s4 = (j & 0x08) >> 3;
     v.s5 = (j & 0x04) >> 2;
     v.s6 = (j & 0x02) >> 1;
     v.s7 = j & 0x01;
     for(i = 0; i < pages; i++) {
       table[i * 256 + j] = v;
       v <<= 1;
     }
  }
}


__kernel void CopyVram(__global uchar *to, __global uchar *from, int size, int multithread)
{
  int t;
  int gid;
//  int lid;
  int pbegin;
  int pb1;
  int ww;
  int i;
  int l_mod, r_mod;
  int ww2, ww3;
  __global uint8 *p32, *q32;
  __global uchar *p, *q;
  
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      //lid = get_local_id(0);
      pbegin = (gid * size) / t; 
      pb1 = ((gid + 1) * size) / t;
      if(pb1 > size) {
         ww = size - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
  } else {
      pbegin = 0;
      ww = size;
      gid = 0;
  }
  p = &(from[pbegin]);
  q = &(to[pbegin]);

  l_mod = (32 - pbegin % 32) % 32;
  
  ww2 = ww - l_mod;
  ww3 = ww2 / 32;
  r_mod = ww2 - ww3 * 32;
  
  for(i = 0; i < l_mod; i++) *q++ = *p++;

  p32 = (__global uint8 *)p;
  q32 = (__global uint8 *)q;
  for(i = 0; i < ww3; i++) *q32++ = *p32++;

  p = (__global uchar *)p32;
  q = (__global uchar *)q32;
  for(i = 0; i < r_mod; i++) *q++ = *p++;
}

void setup_ttlpalette(__global uchar *pal, uint *palette, uint4 bright, uint vpage)
{
   int i;
   uint4 v;
   uint4  rgba_int;
   uint4  palette_int[8];
   uint8  *palette8;
   uint8 r8, g8, b8, a8;
   
   for(i = 0; i < 8; i++) {
      v = ttlpalet2rgb(pal, i & vpage);
      rgba_int = ((v * bright) & (uint4){0x0000ff00, 0x0000ff00, 0x0000ff00, 0x0000ff00}) >> 8;
      palette_int[i] = rgba_int;
   }
   r8 = (uint8) {palette_int[0].s0, palette_int[1].s0,
                 palette_int[2].s0, palette_int[3].s0,
		 palette_int[4].s0, palette_int[5].s0,
		 palette_int[6].s0, palette_int[7].s0} << rshift;
   g8 = (uint8) {palette_int[0].s1, palette_int[1].s1,
                 palette_int[2].s1, palette_int[3].s1,
		 palette_int[4].s1, palette_int[5].s1,
		 palette_int[6].s1, palette_int[7].s1} << gshift;
   b8 = (uint8) {palette_int[0].s2, palette_int[1].s2,
                 palette_int[2].s2, palette_int[3].s2,
		 palette_int[4].s2, palette_int[5].s2,
		 palette_int[6].s2, palette_int[7].s2} << bshift;
   a8 = (uint8) {palette_int[0].s3, palette_int[1].s3,
                 palette_int[2].s3, palette_int[3].s3,
		 palette_int[4].s3, palette_int[5].s3,
		 palette_int[6].s3, palette_int[7].s3} << ashift;
   palette8 = (uint8 *)palette;
   *palette8 = r8 | g8 | b8 | a8;
		 
}

void clearscreen(int w,  __global uint8 *out, float4 bright)
{
   int i;
   __global uint8 *p = out;
   uint a = 255 * bright.s3;
   uint8 abuf;

   a = (a & 255) << ashift;
   abuf = (uint8){a, a, a, a, a, a, a, a};

   for(i = 0; i < w; i++) {
      *p++ = abuf;
   }
}


__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out,
                       __global struct palettebuf_t *pal, __global ushort8 *table,
		       int multithread, int crtflag, float4 bright)
{
  int ofset = 0x4000;
  int x;
  int ww;
  int t, q, rr;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8   av;
  ushort  mask;
  ushort8 mask8;
  __local  ushort8 tbl8[3 * 256];
  ushort8 c8;
  __global uint8 *p8;
  __global uchar *src_r;
  __global uchar *src_g;
  __global uchar *src_b;
  uint pb1, pbegin, col;
  uint palette[8];
  int line;
  int lid = 0;
  int i;
  int oldline = 0;
  int lines;
  int nextline = h;
  int wrap;
  uchar mpage;
  __local uint4 bright4;
  int line2;
  int palette_changed = 0;
  
  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      lid = get_local_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 > col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
      t = 1;
      gid = 0;
      lid = 0;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));

  if(crtflag == 0) {
    clearscreen(ww, p8, bright);
    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
  }

  if(h > 200) ofset = 0x8000;

  src_r = (__global uchar *)&src[addr + ofset];
  src_g = (__global uchar *)&src[addr + ofset + ofset];
  src_b = (__global uchar *)&src[addr];

  q = 0;
  rr = 256 * 3;
  if(lid == 0) {
      for(i = q; i < rr; i++) tbl8[i] = table[i]; // Prefetch palette
      bright4 = convert_uint4((float4){255.0, 255.0, 255.0, 255.0} * bright);
  }
  barrier(CLK_LOCAL_MEM_FENCE);

  line = addr / 80;

  {
       lines = pal->dlines_h * 256 + pal->dlines_l;
       i = 0;
       //nextline = pal->dtbls[0].line_h * 256 + pal->dtbls[0].line_l;
       for(i = 0; i < lines; i++) {
          line2 = pal->dtbls[i].line_h * 256 +  pal->dtbls[i].line_l;
          if((line2 < 0) || (line2 >= h)) break;
          if(line2 >= line) break;
       }
       oldline = i - 1;
       if(oldline < 0) oldline = 0;
       if(oldline >= lines) oldline = lines - 1; 
       mpage = pal->dtbls[oldline].mpage;
       mask = (~(mpage >> 4)) & 0x07;
       mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       
       setup_ttlpalette(pal->dtbls[oldline].tbl, palette, bright4, mask);
       
       if(oldline < (lines - 1)) {
	     nextline = pal->dtbls[oldline + 1].line_h * 256 + pal->dtbls[oldline + 1].line_l;
	     if(nextline > h) {
	       nextline = h;
	     }
	     oldline++;
       } else {
	     nextline = h;
       }
       if(((addr + ww - 1) / 80) >= nextline) {
         palette_changed = -1;
       }
  }

  //barrier(CLK_LOCAL_MEM_FENCE);

  wrap = line;
  for(x = 0; x < ww; x++) {

     if(palette_changed != 0) {
      line = (x + addr) / 80;
      if((wrap != line) && (line >= nextline)) {
	   mpage = pal->dtbls[oldline].mpage;
	   mask = (~(mpage >> 4)) & 0x07;
	   mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
	   setup_ttlpalette(pal->dtbls[oldline].tbl, palette, bright4, mask);
	   wrap = line;
 	   if(oldline < (lines - 1)) {
	        nextline = pal->dtbls[oldline + 1].line_h * 256 + pal->dtbls[oldline + 1].line_l;
		oldline++;
	    } else {
	        nextline = h;
	   }
	   if(line <= nextline) palette_changed = 0;
       }
     }


        bc = *src_b++;
	rc = *src_r++;
	gc = *src_g++;
        c8 = tbl8[bc] | tbl8[rc + 256] | tbl8[gc + 256 * 2];
	c8 &= mask8;
	av.s0 = palette[c8.s0];
	av.s1 = palette[c8.s1];
	av.s2 = palette[c8.s2];
	av.s3 = palette[c8.s3];
	av.s4 = palette[c8.s4];
	av.s5 = palette[c8.s5];
	av.s6 = palette[c8.s6];
	av.s7 = palette[c8.s7];
        *p8++ = av;
	}
//    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
}	
	
inline uint get_apalette(__global struct apalettetbl_t *pal, uint col, uint4 bright)
{
   uint4 rgba;
   uint dat;
   rgba.s0 = (uint)(pal->r_4096[col] * bright.s0) & 0x00ff00;
   rgba.s1 = (uint)(pal->g_4096[col] * bright.s1) & 0x00ff00;
   rgba.s2 = (uint)(pal->b_4096[col] * bright.s2) & 0x00ff00;
   rgba.s3 = 0;
   rgba >>= 4;
   rgba.s0 <<= rshift;
   rgba.s1 <<= gshift;
   rgba.s2 <<= bshift;
   rgba.s3  = (uint)((255 * bright.s3) >> 8 ) << ashift;
   dat = rgba.s0 | rgba.s1 | rgba.s2 | rgba.s3;
   return dat;
}   

   

__kernel void getvram4096(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global struct palettebuf_t *pal,
			  __global ushort8 *table, 
			  uint multithread, int crtflag, float4 bright)
{
  int ofset = 0x8000;
  int x;
  int ww;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uint r0, r1, r2, r3;
  uint b0, b1, b2, b3;
  uint g0, g1, g2, g3;
  ushort8 r8; 
  ushort8 g8;
  ushort8 b8;
  __global uchar *r, *g, *b;
  uint8 av;
  ushort8 cv;
  __global uint8 *p8;
  __local  ushort8 tbl8[256 * 4];
  uint pb1, pbegin, col;
  ushort8 mask8;
  ushort mask;
  int lid = 0;
  int i;
  uint palette[4096];
  int mpage;
  int line;
  int nextline = 200;
  int oldline = 0;
  int lines;
  int wrap;
  int line2;
  int palette_changed = 0;
  __local uint4 bright4;
  
  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      lid = get_local_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 > col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
      gid = 0;
      lid = 0;
  }


  p8 = (__global uint8 *)(&(out[addr2]));
  line = addr2 / 40;
  src = &src[addr];
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }

  if(lid == 0) {
     bright4 = convert_uint4((float4){256.0, 256.0, 256.0, 256.0} * bright);
     for(i = 0; i < 1024; i++) tbl8[i] = table[i]; // prefetch palette;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  {
      lines = (pal->alines_h << 8) | pal->alines_l;
      for(i = 0; i < lines; i++) {
          line2 = pal->atbls[i].line_h * 256 + pal->atbls[i].line_l;
          if((line2 < 0) || (line2 > 199)) break;
          if(line2 >= line) break;
       }
       
       oldline = i - 1;
       if(oldline < 0) oldline = 0;
       if(oldline >= lines) oldline = lines - 1;
       mpage = pal->atbls[oldline].mpage;
       mask = 0x0000;
       if(!(mpage & 0x10)) mask |= 0x000f;
       if(!(mpage & 0x20)) mask |= 0x00f0;
       if(!(mpage & 0x40)) mask |= 0x0f00;
       mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       
       for(i = 0; i < 4096; i++) palette[i] = get_apalette(&(pal->atbls[oldline]), i & mask, bright4); // Prefetch palette
       
       if(oldline < (lines - 1)) {
           nextline = pal->atbls[oldline + 1].line_h * 256 + pal->atbls[oldline + 1].line_l;
	   if(nextline > 200) {
	        nextline = 200;
	   } 
	   oldline++;
	 } else {
	   nextline = 200;
	 }
    }
    if(((addr + ww - 1) / 40) >= nextline) {
         palette_changed = -1;
    }


    
  b = src;
  r = &src[ofset];
  g = &r[ofset];
  wrap = line;
  for(x = 0; x < ww; x++) {
    if(palette_changed != 0) {
       line = (x + addr) / 40;
       if((wrap != line) && (line >= nextline)) {
	      if(oldline < (lines - 1)) {
	        nextline = pal->atbls[oldline + 1].line_h * 256 + pal->atbls[oldline + 1].line_l;
		oldline++;
	      } else {
	        nextline = 200;
	      }
	      if(line <= nextline) palette_changed = 0;
	      wrap = line;
	      mpage = pal->atbls[oldline].mpage;
	      mask = 0x0000;
	      //mask = 0x0fff;
	      if(!(mpage & 0x10)) mask |= 0x000f;
	      if(!(mpage & 0x20)) mask |= 0x00f0;
	      if(!(mpage & 0x40)) mask |= 0x0f00;
	      for(i = 0; i < 4096; i++) palette[i] = get_apalette(&(pal->atbls[oldline]), i & mask, bright4); // Prefetch palette
	      mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       }
    }
	b3 = (uint)(b[0x0   ]) + 0x300;
	b2 = (uint)(b[0x2000]) + 0x200;
	b1 = (uint)(b[0x4000]) + 0x100;
	b0 = (uint)(b[0x6000]) + 0x000;
	
	b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3];

	r3 = (uint)(r[0x0   ]) + 0x300;
	r2 = (uint)(r[0x2000]) + 0x200;
	r1 = (uint)(r[0x4000]) + 0x100;
	r0 = (uint)(r[0x6000]) + 0x000;
	
	r8 = tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3];
	r8 = r8 * (ushort8){16, 16, 16, 16, 16, 16, 16, 16};
	
	g3 = (uint)(g[0x0   ]) + 0x300;
	g2 = (uint)(g[0x2000]) + 0x200;
	g1 = (uint)(g[0x4000]) + 0x100;
	g0 = (uint)(g[0x6000]) + 0x000;
	g8 = tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3];
	g8 = g8 * (ushort8){256, 256, 256, 256, 256, 256, 256, 256};
	
	
	cv = (b8 | r8 | g8) & mask8;
        av.s0 = palette[cv.s0];
        av.s1 = palette[cv.s1];
        av.s2 = palette[cv.s2];
        av.s3 = palette[cv.s3];
        av.s4 = palette[cv.s4];
        av.s5 = palette[cv.s5];
        av.s6 = palette[cv.s6];
        av.s7 = palette[cv.s7];
        *p8++ = av;
	b++;
	r++;
	g++;
	}
//    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
}	
	
__kernel void getvram256k(__global uchar *src, int w, int h, 
                          __global uchar4 *out,
			  __global ushort8 *table, 
			  uint mpage,
			  int multithread, int crtflag, float4 bright)
{
  int ofset = 0xc000;
  int x;
  int ww;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uint r0, r1, r2, r3, r4, r5;
  uint b0, b1, b2, b3, b4, b5;
  uint g0, g1, g2, g3, g4, g5;
  uint8 r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  uint8 g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  uint8 b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  int rdraw, gdraw, bdraw;
  __global uchar *r, *g, *b;
  uint8 cv;
  __global uint8 *p8;
  __local  ushort8 tbl8[256 * 6];
  uint col;
  int lid = 0;
  uint pbegin, pb1;
  uint bright_r, bright_g, bright_b, bright_a;
  __local uint8 bright_r8, bright_g8, bright_b8, bright_a8;
  uint8 mask8 = (uint8){0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff};
  int i;

  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      lid = get_local_id(0);
      gid = get_global_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 >= col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      if(ww <= 0) return;
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }

  if(lid == 0) {
      for(i = 0; i < (256 * 6); i++) tbl8[i] = table[i]; // Prefetch palette
      bright_r = bright.s0 * 255;
      bright_g = bright.s1 * 255;
      bright_b = bright.s2 * 255;
      bright_a = bright.s3 * 255;
      
      bright_r8 = (uint8){bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r};
      bright_g8 = (uint8){bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g};
      bright_b8 = (uint8){bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b};
      bright_a8 = (uint8){bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a};
      bright_a8 <<= ashift;
  }

  
  b = &src[addr];
  r = &src[addr + ofset];
  g = &src[addr + ofset + ofset];

  r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};

  gdraw = 0; if((mpage & 0x40) == 0) gdraw = -1; 
  rdraw = 0; if((mpage & 0x20) == 0) rdraw = -1; 
  bdraw = 0; if((mpage & 0x10) == 0) bdraw = -1; 
  for(x = 0; x < ww; x++) {
	if(bdraw) {
	    b5 = (uint)(b[0     ]) + 0x500;
	    b4 = (uint)(b[0x2000]) + 0x400;
	    b3 = (uint)(b[0x4000]) + 0x300;
	    b2 = (uint)(b[0x6000]) + 0x200;
	    b1 = (uint)(b[0x8000]) + 0x100;
	    b0 = (uint)(b[0xa000]) + 0x000;
	    b8 =  convert_uint8(tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3] | tbl8[b4] | tbl8[b5]);
	    b8 <<= 2;
	    b8 = ((b8 * bright_b8) >> 8) & mask8;
            b8 <<= bshift;
	}
	if(rdraw) {
	    r5 = (uint)(r[0     ]) + 0x500;
	    r4 = (uint)(r[0x2000]) + 0x400;
	    r3 = (uint)(r[0x4000]) + 0x300;
	    r2 = (uint)(r[0x6000]) + 0x200;
	    r1 = (uint)(r[0x8000]) + 0x100;
	    r0 = (uint)(r[0xa000]) + 0x000;
	    r8 =  convert_uint8(tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3] | tbl8[r4] | tbl8[r5]);
	    r8 <<= 2;
	    r8 = ((r8 * bright_r8) >> 8) & mask8;
            r8 <<= rshift; // 6bit -> 8bit
	}
	if(gdraw) {
	    g5 = (uint)(g[0     ]) + 0x500;
	    g4 = (uint)(g[0x2000]) + 0x400;
	    g3 = (uint)(g[0x4000]) + 0x300;
	    g2 = (uint)(g[0x6000]) + 0x200;
	    g1 = (uint)(g[0x8000]) + 0x100;
	    g0 = (uint)(g[0xa000]) + 0x000;
	    g8 =  convert_uint8(tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3] | tbl8[g4] | tbl8[g5]);
	    g8 <<= 2;
	    g8 = ((g8 * bright_g8) >> 8) & mask8;
            g8 <<= gshift; // 6bit -> 8bit
	}
	cv = b8 | r8 | g8 | bright_a8;
	*p8++ = cv;
	b++;
	g++;
	r++;
   }
//   barrier(CLK_GLOBAL_MEM_FENCE);
   return;
}	
	
