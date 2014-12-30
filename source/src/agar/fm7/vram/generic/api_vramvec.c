/*
 * api_vramvec.cpp
 * Convert VRAM -> VirtualVram(Vector Version)
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "xm7_types.h"
#include "api_draw.h"
#include "api_vram.h"
#include "agar_logger.h"
#include "cache_wrapper.h"

/*
* Definition of Convertsion Tables.
*/
// Reduce Tables 20120131

v8si *aPlanes;
static void initvramtblsub_vec(volatile unsigned char x, volatile v8hi_t *p)
{
//    p->v = (v8si){x & 0x80, x & 0x40, x & 0x20, x & 0x10, x & 0x08, x & 0x04, x & 0x02, x & 0x01};
    
    p->i[0] = (x & 0x80) >> 7;
    p->i[1] = (x & 0x40) >> 6;
    p->i[2] = (x & 0x20) >> 5;
    p->i[3] = (x & 0x10) >> 4;
    p->i[4] = (x & 0x08) >> 3;
    p->i[5] = (x & 0x04) >> 2;
    p->i[6] = (x & 0x02) >> 1;
    p->i[7] = x & 0x01;
    // 8 Colors
}

void initvramtbl_8_vec(void)
{
}

static v8si *initvramtblsub(int size)
{
   v8si *p;
#ifndef _WINDOWS
   if(posix_memalign((void **)&p, 16 * sizeof(Uint32), sizeof(v8si) * size) != 0) return NULL;
#else
   p = (v8si *)__mingw_aligned_malloc(sizeof(v8si) * size, 16 * sizeof(Uint32));
   if(p == NULL) return NULL;
#endif
   return p;
}


void initvramtbl_4096_vec(void)
{
    int i;
    volatile v8hi_t r;
    aPlanes = initvramtblsub(12 * 256);
    if(aPlanes == NULL) return;
    XM7_DebugLog(XM7_LOG_DEBUG, "Vram Table OK");
    // Init Mask Table
   for(i = 0; i <= 255; i++){
        initvramtblsub_vec(i & 255, &r);

        aPlanes[B0 + i] = r.v;
        r.v <<= 1;
        aPlanes[B1 + i] = r.v;
        r.v <<= 1;
        aPlanes[B2 + i] = r.v;
        r.v <<= 1;
        aPlanes[B3 + i] = r.v;
        r.v <<= 1;


        aPlanes[R0 + i] = r.v;
        r.v <<= 1;
        aPlanes[R1 + i] = r.v;
        r.v <<= 1;
        aPlanes[R2 + i] = r.v;
        r.v <<= 1;
        aPlanes[R3 + i] = r.v;
        r.v <<= 1;
      
        aPlanes[G0 + i] = r.v;
        r.v <<= 1;
        aPlanes[G1 + i] = r.v;
        r.v <<= 1;
        aPlanes[G2 + i] = r.v;
        r.v <<= 1;
        aPlanes[G3 + i] = r.v;
//        r.v <<= 1;
    }
    _prefetch_data_read_permanent(aPlanes, sizeof(Uint32) * 256 * 8 * 12); // 98KB (!), priority = 1.
}

void detachvramtbl_8_vec(void)
{
   
}

void detachvramtbl_4096_vec(void)
{
   if(aPlanes != NULL) {
#ifndef _WINDOWS
      free(aPlanes);
#else
      __mingw_aligned_free(aPlanes);
#endif
      aPlanes = NULL;
   }
}



v8hi_t lshift_6bit8v(v8hi_t *v)
{
   v8hi_t r;
   v8hi_t cbuf;
   v8hi_t mask;
   mask.v = (v8si){0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8};
   cbuf.v =
        aPlanes[B2 + v->b[0]] |
        aPlanes[B3 + v->b[1]] |
        aPlanes[R0 + v->b[2]] |
        aPlanes[R1 + v->b[3]] |
        aPlanes[R2 + v->b[4]] |
        aPlanes[R3 + v->b[5]];
   
   mask.v = mask.v & cbuf.v;
#if ((__GNUC__ == 4) && (__GCC_MINOR__ >= 7)) || (__GNUC__ > 4) //GCC 4.7 or later.
   r.v = mask.v != (v8si){0, 0, 0, 0, 0, 0, 0, 0};
   r.v = r.v & (v8si) {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
   cbuf.v = cbuf.v |  r.v;
#else
   if(mask.i[0] != 0) cbuf.s[0] |= 0x03;
   if(mask.i[1] != 0) cbuf.s[1] |= 0x03;
   if(mask.i[2] != 0) cbuf.s[2] |= 0x03;
   if(mask.i[3] != 0) cbuf.s[3] |= 0x03;
   if(mask.i[4] != 0) cbuf.s[4] |= 0x03;
   if(mask.i[5] != 0) cbuf.s[5] |= 0x03;
   if(mask.i[6] != 0) cbuf.s[6] |= 0x03;
   if(mask.i[7] != 0) cbuf.s[7] |= 0x03;
#endif	
  return cbuf;
}




