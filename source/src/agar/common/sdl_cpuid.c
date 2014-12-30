/*
 * Check CPU Features(for GCC)
 * (C)2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * 1,May 2012 Initial
 * License: CC-BY-SA
 */

#include "xm7.h"
#include "xm7_types.h"
#include "sdl_cpuid.h"
#if defined(__x86_64__) || defined(__i386__)

void getCpuID(struct XM7_CPUID *p)
{
   Uint32 a,b,c,d;
   a = b = c = d = 0;
   if(__get_cpuid(0x80000001 , &a, &b, &c, &d) == 0) return; // Get Features
   p->use_mmx = ((d & bit_MMX) != 0)?TRUE:FALSE;
   p->use_sse = ((d & bit_SSE) != 0)?TRUE:FALSE;
   p->use_sse2 = ((d & bit_SSE2) != 0)?TRUE:FALSE;
   p->use_cmov = ((d & bit_CMOV) != 0)?TRUE:FALSE;
   p->use_sse3 = ((c & bit_SSE3) != 0)?TRUE:FALSE;
   p->use_ssse3 = ((c & bit_SSSE3) != 0)?TRUE:FALSE;
   p->use_sse41 = ((c & bit_SSE4_1) != 0)?TRUE:FALSE;
   p->use_sse42 = ((c & bit_SSE4_2) != 0)?TRUE:FALSE;
   p->use_sse4a = ((c & bit_SSE4a) != 0)?TRUE:FALSE;
   p->use_abm = ((c & bit_ABM) != 0)?TRUE:FALSE;
   p->use_avx = ((c & bit_AVX) != 0)?TRUE:FALSE;
   p->use_mmxext = ((d & bit_MMXEXT) != 0)?TRUE:FALSE;
   p->use_3dnow = ((d & bit_3DNOW) != 0)?TRUE:FALSE;
   p->use_3dnowp = ((d & bit_3DNOWP) != 0)?TRUE:FALSE;
}

struct XM7_CPUID *initCpuID(void)
{
   struct XM7_CPUID *p;
   p = (struct XM7_CPUID *)malloc(sizeof(struct XM7_CPUID));
   if(p == NULL) return NULL;
   memset(p, 0x00, sizeof(struct XM7_CPUID));
   getCpuID(p);
   return p;
}


#else
// 未設定のアーキテクチャはここにかく
void getCpuID(struct XM7_CPUID *p)
{
}

struct XM7_CPUID *initCpuID(void)
{
   return NULL;
}

#endif

void detachCpuID(struct XM7_CPUID *p)
{
   if(p == NULL) return;
   free(p);
}

