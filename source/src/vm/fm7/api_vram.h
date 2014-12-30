/*
 * XM7: Vram Emulation Routines
 *  (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */
#ifndef API_VRAM_H_INCLUDED
#define API_VRAM_H_INCLUDED

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
//#include "api_draw.h"
//#include "api_scaler.h"


//#include "agar_draw.h"
#include "common.h"
#include "emu.h"
#include "config.h"
#include "simd_types.h"
#include "agar_gldraw.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {   
	void(*vram_block)(Uint32 *, int, int, int, int);
        void(*vram_line )(Uint32 *, int, int, int);
        void(*vram_windowline)(Uint32, int, int, int, int, int);
} Api_Vram_FuncList;
extern Api_Vram_FuncList *pVirtualVramBuilder; /* 書き換え関数ポインタ */
/*
 * api_vram8.c
*/
extern void SetVram_200l(Uint8 *p);
extern void SetVram_400l(Uint8 *p);
extern void CalcPalette_8colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern Api_Vram_FuncList api_vram8_generic;
extern Api_Vram_FuncList api_vram4096_generic;
extern Api_Vram_FuncList api_vram256k_generic;

#ifdef USE_SSE2
extern void CreateVirtualVram8_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mpage);
extern void CreateVirtualVram8_WindowedLine_SSE2(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode);
extern void CreateVirtualVram8_Line_SSE2(Uint32 *p, int ybegin, int yend, int mode);
#endif
/*
 * api_vram4096.c
 */
extern void CalcPalette_4096Colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
#ifdef USE_SSE2
extern void CreateVirtualVram4096_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mpage);
extern void CreateVirtualVram4096_Line_SSE2(Uint32 *p, int ybegin, int yend, int mode);
extern void CreateVirtualVram4096_WindowedLine_SSE2(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode);
#endif
   
/*
 * api_vram256k.c
 */
#ifdef USE_SSE2
extern void CreateVirtualVram256k_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mpage);
extern void CreateVirtualVram256k_Line_SSE2(Uint32 *p, int ybegin, int yend, int mode);
#endif

#ifdef USE_SSE2
extern Api_Vram_FuncList api_vram8_sse2;
extern Api_Vram_FuncList api_vram4096_sse2;
extern Api_Vram_FuncList api_vram256k_sse2;
#endif   
   
#if (__GNUC__ >= 4)
extern void initvramtbl_8_vec(void);
extern void initvramtbl_4096_vec(void);
extern void detachvramtbl_8_vec(void);
extern void detachvramtbl_4096_vec(void);
extern v8hi_t lshift_6bit8v(v8hi_t *v);
extern v8si *aPlanes;

enum {
   B0 = 0,
   B1 = 256,
   B2 = 512,
   B3 = 768,
   R0 = 1024,
   R1 = 1280,
   R2 = 1536,
   R3 = 1792,
   G0 = 2048,
   G1 = 2304,
   G2 = 2560,
   G3 = 2816
};

   
enum {
   PLAINB0 = 0,
   PLAINB1,
   PLAINB2,
   PLAINB3,
   PLAINR0,
   PLAINR1,
   PLAINR2,
   PLAINR3,
   PLAING0,
   PLAING1,
   PLAING2,
   PLAING3
};

enum {
   PLAINB = 0,
   PLAINR,
   PLAING,
   PLAINW
};



#endif    // __GNUC__ >= 4
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;



#ifdef __cplusplus
}
#endif


#endif // API_VRAM_H_INCLUDED
