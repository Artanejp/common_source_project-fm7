/*
* FM-7 Emulator "XM7" -> CommonSourceProjedct
* Virtual Vram Display(Agar widget version)
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* Jan 18,2012 From demos/customwidget/mywidget.[c|h]
* Jan 20,2012 Separete subroutines.
* Dec 30,2014 Move from XM7/SDL, 100% my original file.
*/

#include "agar_sdlview.h"
#include "agar_cfg.h"
#include "api_vram.h"
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_kbd.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern "C" {
extern struct AGAR_CPUID *pCpuID;
extern BOOL bUseSIMD;
}

extern "C" { // Define Headers
   // scaler/generic
   extern void pVram2RGB_x05_Line(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x05.c , raster render
   extern void pVram2RGB_x1_Line(Uint32 *src,  Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x1.c , raster render
   extern void pVram2RGB_x125_Line(Uint32 *src,  Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x125.c , raster render
   extern void pVram2RGB_x15_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x15.c , raster render.
   extern void pVram2RGB_x2_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x225_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x225.c , raster render.
   extern void pVram2RGB_x25_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x25.c , raster render.
   extern void pVram2RGB_x3_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x3.c , raster render.
   extern void pVram2RGB_x4_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x4.c , raster render.
   extern void pVram2RGB_x45_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x45.c , raster render.
   extern void pVram2RGB_x5_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x5.c , raster render.
   extern void pVram2RGB_x6_Line(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x6.c , raster render.
#if defined(USE_SSE2) // scaler/sse2/
   extern void pVram2RGB_x1_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x1_sse2.c , raster render
   extern void pVram2RGB_x125_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x125_sse2.c , raster render
   extern void pVram2RGB_x15_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x15_sse2.c , raster render
   extern void pVram2RGB_x2_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2_sse2.c , raster render.
   extern void pVram2RGB_x225_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x225_sse2.c , raster render.
   extern void pVram2RGB_x25_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x25_sse2.c , raster render.
   extern void pVram2RGB_x3_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x3_sse2.c , raster render.
   extern void pVram2RGB_x4_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x4_sse2.c , raster render.
   extern void pVram2RGB_x45_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x45_sse2.c , raster render.
   extern void pVram2RGB_x5_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x5_sse2.c , raster render.
   extern void pVram2RGB_x6_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x6_sse2.c , raster render.
#endif
}

static int iScaleFactor = 1;
static void *pDrawFn = NULL;
static void *pDrawFn2 = NULL;
static int iOldW = 0;
static int iOldH = 0;


static inline Uint32 pVram_XtoHalf(Uint32 d1, Uint32 d2)
{
   Uint32 d0;
   Uint16 r,g,b,a;
#if AG_BIG_ENDIAN
   r = (d1 & 0x000000ff) + (d2 & 0x000000ff);
   g = ((d1 & 0x0000ff00) >> 8) + ((d2 & 0x0000ff00) >> 8);
   b = ((d1 & 0x00ff0000) >> 16) + ((d2 & 0x00ff0000) >> 16);
   d0 = 0xff000000 | (r >> 1) | ((b << 15) & 0x00ff0000) | ((g << 7) & 0x0000ff00);
#else
   r = ((d1 & 0xff000000) >> 24) + ((d2 & 0xff000000) >> 24);
   g = ((d1 & 0x00ff0000) >> 16) + ((d2 & 0x00ff0000) >> 16);
   b = ((d1 & 0x0000ff00) >> 8) + ((d2 & 0x0000ff00) >> 8);
   d0 = 0x000000ff | ((r << 23) & 0xff000000) | ((g << 15) & 0x00ff0000) | ((b << 7) & 0x0000ff00);
#endif
   return d0;
}


#if defined(USE_SSE2)
// w0, h0 = Console
// w1, h1 = DrawMode
static void *AGAR_SDLViewSelectScaler_Line_SSE2(int w0 ,int h0, int w1, int h1)
{
    int wx0 = w0 >> 1; // w1/4
    int hy0 = h0 >> 1;
    int xfactor;
    int yfactor;
    int xth;
    void (*DrawFn)(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep);
    DrawFn = NULL;
   
    xfactor = w1 % wx0;
    yfactor = h1 % hy0;
    xth = wx0 >> 1;
    if(__builtin_expect((iScaleFactor == (w1 / w0) && (pDrawFn2 != NULL)
      && (w1 == iOldW) && (h1 == iOldH)), 1))  return (void *)pDrawFn2;
    iScaleFactor = w1 / w0;
    iOldW = w1;
    iOldH = h1;
    switch(iScaleFactor){
     case 0:
            if(w0 > 480){
	        if((w1 < 480) || (h1 < 150)){
		   DrawFn = pVram2RGB_x05_Line;
		} else {
		   DrawFn = pVram2RGB_x1_Line_SSE2;
		}
            } else {
                DrawFn = pVram2RGB_x1_Line_SSE2;
            }
            break;

     case 1:
	       if(w1 > 900) {
		  DrawFn = pVram2RGB_x15_Line_SSE2; // 1.5?
	       } else if(w1 > 720) {
		  DrawFn = pVram2RGB_x125_Line_SSE2; // 1.25
	       } else {
		  DrawFn = pVram2RGB_x1_Line_SSE2; // 1.0
	       }
            break;
     case 2:
//            if(xfactor < xth){
	      if((w1 > 720) && (w0 <= 480)) {
		 DrawFn = pVram2RGB_x25_Line_SSE2;  // x2.5
	      } else if((w1 > 1360) && (w1 <= 1520)){
		 DrawFn = pVram2RGB_x225_Line_SSE2; // x2.25
	      } else if(w1 > 1700){
		 DrawFn = pVram2RGB_x3_Line_SSE2; // x3
	      } else if(w1 > 1520){
		 DrawFn = pVram2RGB_x25_Line_SSE2; // x2.5@1600
	      } else {
		 DrawFn = pVram2RGB_x2_Line_SSE2; // x2
	      }
            break;
     case 3:
            DrawFn = pVram2RGB_x3_Line_SSE2; // x3
            break;
     case 4:
       if((w1 > 1360) && (w1 < 1760) && (w0 <= 480)) { // 4.5
            DrawFn = pVram2RGB_x45_Line_SSE2; // 4.5
       } else {
            DrawFn = pVram2RGB_x4_Line_SSE2; // 4.0
       }
       break;
     case 5:
            DrawFn = pVram2RGB_x5_Line_SSE2;
            break;
     case 6:
     case 7:
     case 8:
            DrawFn = pVram2RGB_x6_Line_SSE2;
            break;
     default:
	      DrawFn = pVram2RGB_x1_Line_SSE2;
            break;
        }
        pDrawFn2 = (void *)DrawFn;
        return (void *)DrawFn;
}
#endif // USE_SSE2


// w0, h0 = Console
// w1, h1 = DrawMode
static void *AGAR_SDLViewSelectScaler_Line(int w0 ,int h0, int w1, int h1)
{
    int wx0 = w0 >> 1; // w1/4
    int hy0 = h0 >> 1;
    int xfactor;
    int yfactor;
    int xth;
    void (*DrawFn)(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep);
    DrawFn = NULL;

#if defined(USE_SSE2)
   if(pCpuID != NULL){
      if(pCpuID->use_sse2) {
	 return AGAR_SDLViewSelectScaler_Line_SSE2(w0, h0, w1, h1);
      }
   }
#endif
   
    xfactor = w1 % wx0;
    yfactor = h1 % hy0;
    xth = wx0 >> 1;
    if(__builtin_expect((iScaleFactor == (w1 / w0) && (pDrawFn2 != NULL)
      && (w1 == iOldW) && (h1 == iOldH)), 1))  return (void *)pDrawFn2;
    iScaleFactor = w1 / w0;
    iOldW = w1;
    iOldH = h1;
    switch(iScaleFactor){
     case 0:
            if(w0 > 480){
	        if((w1 < 480) || (h1 < 150)){
		   DrawFn = pVram2RGB_x05_Line;
		} else {
		   DrawFn = pVram2RGB_x1_Line;
		}
            } else {
                DrawFn = pVram2RGB_x1_Line;
            }
            break;

     case 1:
       if(w1 > 900) {
	  DrawFn = pVram2RGB_x15_Line; // 1.5?
       } else if(w1 > 720) {
	  DrawFn = pVram2RGB_x125_Line; // 1.25
       } else {
	  DrawFn = pVram2RGB_x1_Line; // 1.0
       }
       break;
     case 2:
//            if(xfactor < xth){
	      if((w1 > 720) && (w0 <= 480)) {
		 DrawFn = pVram2RGB_x25_Line;  // x2.5
	      } else if((w1 > 1360) && (w1 <= 1520)){
		 DrawFn = pVram2RGB_x225_Line; // x2.25
	      }else if(w1 > 1700){
		 DrawFn = pVram2RGB_x3_Line; // x3
	      }else if(w1 > 1520){
		 DrawFn = pVram2RGB_x25_Line; // x2.5
	      } else {
		 DrawFn = pVram2RGB_x2_Line;
	      }
            break;
     case 3:
           DrawFn = pVram2RGB_x3_Line; // x3
           break;
     case 4:
       if((w1 > 1360) && (w1 < 1760) && (w0 <= 480)) { // 4.5
            DrawFn = pVram2RGB_x45_Line; // 4.5
       } else {
            DrawFn = pVram2RGB_x4_Line; // 4.0
       }
       break;
     case 5:
            DrawFn = pVram2RGB_x5_Line;
            break;
     case 6:
     case 7:
     case 8:
            DrawFn = pVram2RGB_x6_Line;
            break;
     default:
	      DrawFn = pVram2RGB_x1_Line;
            break;
        }
        pDrawFn2 = (void *)DrawFn;
        return (void *)DrawFn;
}



void AGAR_SDLViewUpdateSrc(AG_Event *event)
{
   AGAR_SDLView *my = (AGAR_SDLView *)AG_SELF();
   void *Fn = NULL;
   void (*DrawFn2)(Uint32 *, Uint8 *, int , int , int, int);
   AG_Surface *Surface;
   
   Uint8 *pb;
   Uint32 *disp;
   Uint32 *src;
   Uint8 *dst;
   int yrep2;
   int y2, y3;
   int w;
   int h;
   int ww;
   int hh;
   int xx;
   int yy;
   int pitch;
   int bpp;
   int of;
   int yrep;
   int ymod;
   int yfact;
   int lcount;
   int xcache;
   BOOL flag = FALSE;

   Fn = AG_PTR(1);
   if(my == NULL) return;
   Surface = AGAR_SDLViewGetSrcSurface(my);
   
   if(Surface == NULL) return;
   DrawSurface = Surface;
   w = Surface->w;
   h = Surface->h;
   pb = (Uint8 *)(Surface->pixels);
   pitch = Surface->pitch;
   bpp = Surface->format->BytesPerPixel;
   

   if(pVram2 == NULL) return;
   if(__builtin_expect((crt_flag == FALSE), 0)) {
      AG_Rect rr;
      AG_Color cc;
      
      cc.r = 0x00;
      cc.g = 0x00;
      cc.b = 0x00;
      cc.a = 0xff;
      
      LockVram();
      //AG_ObjectLock(AGOBJECT(my));
      AG_SurfaceLock(Surface);
      AG_FillRect(Surface, NULL, cc);
      //AG_ObjectUnlock(AGOBJECT(my));
      AGAR_SDLViewSetDirty(my);
      UnlockVram();
      return;
   }
   
   switch(bMode){
    case SCR_200LINE:
        ww = 640;
        hh = 200;
        break;
    case SCR_400LINE:
        ww = 640;
        hh = 400;
        break;
    default:
        ww = 320;
        hh = 200;
        break;
   }
   Fn = XM7_SDLViewSelectScaler_Line(ww , hh, w, h);
   if(__builtin_expect((Fn != NULL), 1)) {
      DrawFn2 = (void (*)(Uint32 *, Uint8 *, int , int , int, int))Fn;
   } else {
     return;
   }
   

   
   if(h > hh) {
      ymod = h % hh;
      yrep = h / hh;
   } else {
      ymod = h % hh;
      yrep = 1;
   }
   
   if(Fn == NULL) return; 
    src = pVram2;
    LockVram();
    AG_ObjectLock(AGOBJECT(my));

   if(nRenderMethod == RENDERING_RASTER) {
      if(my->forceredraw != 0){
	  for(yy = 0; yy < hh; yy++) {
	     bDrawLine[yy] = TRUE;
	  }
	  my->forceredraw = 0;
       }
       Surface = GetDrawSurface();
       if(Surface == NULL)       goto _end1;
       AG_SurfaceLock(Surface);
       dst = (Uint8 *)(Surface->pixels);
#ifdef _OPENMP
#pragma omp parallel for shared(hh, bDrawLine, yrep, ww, src, Surface, flag) private(dst, y2, y3)
#endif
      for(yy = 0 ; yy < hh; yy++) {
/*
*  Virtual VRAM -> Real Surface:
*/
	 if(__builtin_expect((bDrawLine[yy] == TRUE), 0)) {
//	    _prefetch_data_read_l2(&src[yy * 80], ww * sizeof(Uint32));
	    y2 = (h * yy ) / hh;
	    y3 = (h * (yy + 1)) / hh;
	    dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
	    yrep2 = y3 - y2;
	    if(__builtin_expect((yrep2 < 1), 0)) yrep2 = 1;
	    DrawFn2(src, dst, 0, ww, yy, yrep2);
	    bDrawLine[yy] = FALSE;
	    flag = TRUE;
	 }
	 dst = dst + (yrep2 * Surface->pitch);
      }
      AG_SurfaceUnlock(Surface);
      // BREAK.
      goto _end1;
   } else { // Block
      if(my->forceredraw != 0){
	 for(yy = 0; yy < (hh >> 3); yy++) {
            for(xx = 0; xx < (ww >> 3); xx++ ){
	       SDLDrawFlag.write[xx][yy] = TRUE;
            }
	 }
      }
   }
   
/*
 * Below is BLOCK or FULL.
 * Not use from line-rendering.
 */

   Surface = GetDrawSurface();
   if(Surface == NULL) goto _end1;
   AG_SurfaceLock(Surface);

#ifdef _OPENMP
# pragma omp parallel for shared(pb, SDLDrawFlag, ww, hh, src, flag) private(disp, of, xx, lcount, xcache, y2, y3, dst)
#endif
    for(yy = 0 ; yy < hh; yy += 8) {
       lcount = 0;
       xcache = 0;
//       dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
       for(xx = 0; xx < ww; xx += 8) {
/*
*  Virtual VRAM -> Real Surface:
*                disp = (Uint32 *)(pb + xx  * bpp + yy * pitch);
*                of = (xx % 8) + (xx / 8) * (8 * 8)
*                    + (yy % 8) * 8 + (yy / 8) * 640 * 8;
*                *disp = src[of];
** // xx,yy = 1scale(not 8)
*/
//            if(xx >= w) continue;
	   if(__builtin_expect((SDLDrawFlag.write[xx >> 3][yy >> 3] != FALSE), 1)) {
	      lcount += 8;
	      SDLDrawFlag.write[xx >> 3][yy >> 3] = FALSE;
	   } else {
	      if(__builtin_expect((lcount != 0), 1)) {
		 int yy2;
		 //	      disp = (Uint32 *)pb;
		 //	      of = (xx *8) + yy * ww;
		 //	      DrawFn(&src[of], disp, xx, yy, yrep);
		 for(yy2 = 0; yy2 < 8; yy2++) {
		    y2 = (h * (yy + yy2)) / hh;
		    y3 = (h * (yy + yy2 + 1)) / hh;
		    dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
		    yrep2 = y3 - y2;
		    if(__builtin_expect((yrep2 < 1), 0)) yrep2 = 1;
		    DrawFn2(src, dst, xcache, xcache + lcount, yy + yy2 , yrep2);
		    flag = TRUE;
		 }
	      }
	      
	      xcache = xx + 8;
	      lcount = 0;
	   }
       }
       
       
       if(__builtin_expect((lcount != 0), 1)) {
	  int yy2;
	  //	      disp = (Uint32 *)pb;
	  //	      of = (xx *8) + yy * ww;
	  //	      DrawFn(&src[of], disp, xx, yy, yrep);
	  for(yy2 = 0; yy2 < 8; yy2++) {
	     y2 = (h * (yy + yy2)) / hh;
	     y3 = (h * (yy + yy2 + 1)) / hh;
	     dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
	     yrep2 = y3 - y2;
	     if(__builtin_expect((yrep2 < 1), 0)) yrep2 = 1;
	     DrawFn2(src, dst, xcache, xcache + lcount, yy + yy2 , yrep2);
	     flag = TRUE;
	  }
       }
//			if(yy >= h) continue;
    }
   AG_SurfaceUnlock(Surface);
      
_end1:   
   AG_ObjectUnlock(AGOBJECT(my));
   if(flag != FALSE) XM7_SDLViewSetDirty(my);
   UnlockVram();
   return;
}
