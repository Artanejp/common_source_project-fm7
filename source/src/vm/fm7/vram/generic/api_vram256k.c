/*
 * api_vram256k.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "api_draw.h"
//#include "api_scaler.h"
#include "api_vram.h"


static void putword(Uint32 *disp, Uint32 *cx)
{
    disp[0] = cx[0];
    disp[1] = cx[1];
    disp[2] = cx[2];
    disp[3] = cx[3];
    disp[4] = cx[4];
    disp[5] = cx[5];
    disp[6] = cx[6];
    disp[7] = cx[7];
}



static v8hi_t gpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v8hi_t v;
   v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x40)){
        v.b[5] = vram_p[addr + 0x10000]; 
        v.b[4] = vram_p[addr + 0x12000]; 
        v.b[3] = vram_p[addr + 0x14000]; 
        v.b[2] = vram_p[addr + 0x16000]; 
        v.b[1] = vram_p[addr + 0x28000]; 
        v.b[0] = vram_p[addr + 0x2a000]; 
        v1 = lshift_6bit8v(&v);
        return v1;
    
    } else {
       v8hi_t r;
       r.v = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
   

}

static v8hi_t rpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v8hi_t v;
   v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x20)){
        v.b[5] = vram_p[addr + 0x08000]; 
        v.b[4] = vram_p[addr + 0x0a000]; 
        v.b[3] = vram_p[addr + 0x0c000]; 
        v.b[2] = vram_p[addr + 0x0e000]; 
        v.b[1] = vram_p[addr + 0x20000]; 
        v.b[0] = vram_p[addr + 0x22000]; 
        v1 = lshift_6bit8v(&v);
        return v1;
   } else {
       v8hi_t r;
       r.v = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}

static v8hi_t bpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v8hi_t v;
   v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x10)){
        v.b[5] = vram_p[addr + 0x00000]; 
        v.b[4] = vram_p[addr + 0x02000]; 
        v.b[3] = vram_p[addr + 0x04000]; 
        v.b[2] = vram_p[addr + 0x06000]; 
        v.b[1] = vram_p[addr + 0x18000]; 
        v.b[0] = vram_p[addr + 0x1a000]; 
        
        v1 = lshift_6bit8v(&v);
//        v1.v <<= 16;
        return v1;
   } else {
       v8hi_t r;
       r.v = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}




static void getvram_256k(Uint32 addr, Uint32 mpage, Uint32 *cbuf)
{
   v8hi_t r, g, b;
   /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */
   
   b = bpixel2cbuf(addr, mpage);
   r = rpixel2cbuf(addr, mpage);
   g = gpixel2cbuf(addr, mpage);
#ifdef AG_LITTLE_ENDIAN   
   cbuf[0] = (b.i[0] << 16) | (g.i[0] << 8) | r.i[0] | 0xff000000;
   cbuf[1] = (b.i[1] << 16) | (g.i[1] << 8) | r.i[1] | 0xff000000;
   cbuf[2] = (b.i[2] << 16) | (g.i[2] << 8) | r.i[2] | 0xff000000;
   cbuf[3] = (b.i[3] << 16) | (g.i[3] << 8) | r.i[3] | 0xff000000;
   cbuf[4] = (b.i[4] << 16) | (g.i[4] << 8) | r.i[4] | 0xff000000;
   cbuf[5] = (b.i[5] << 16) | (g.i[5] << 8) | r.i[5] | 0xff000000;
   cbuf[6] = (b.i[6] << 16) | (g.i[6] << 8) | r.i[6] | 0xff000000;
   cbuf[7] = (b.i[7] << 16) | (g.i[7] << 8) | r.i[7] | 0xff000000;
#else   
#endif
   return ;
}



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram256k_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage)
{
    Uint32 c[8];
    Uint32 *disp = p;
    Uint32 addr;
   
    addr = y * 40 + x;
    // Loop廃止(高速化)

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);

}

void CreateVirtualVram256k_WindowedLine(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mpage)
{
    Uint32 c[8];
    Uint8 *disp;
    Uint32 addr;
    int pitch = sizeof(Uint32) * 8;
    int xx;
    int yy;
   
    for(yy = ybegin ; yy < yend; yy++) {
       addr = yy * 40 + xbegin;
       disp = (Uint8 *)p + (pitch * addr);
       for(xx = xbegin; xx < xend; xx++) {

	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  addr++;
	  disp += pitch;
	  
	  getvram_256k(addr, mpage, (Uint32 *)&c);
	  putword((Uint32 *)disp, (Uint32 *)&c);
	  
       }
    }
}

void CreateVirtualVram256k_Line(Uint32 *p, int ybegin, int yend, int mpage)
{
   CreateVirtualVram256k_WindowedLine(p, ybegin, yend, 0, 40, mpage);
}

Api_Vram_FuncList api_vram256k_generic = {
   CreateVirtualVram256k_1Pcs,
   CreateVirtualVram256k_Line,
   CreateVirtualVram256k_WindowedLine
};
