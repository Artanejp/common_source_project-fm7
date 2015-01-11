/*
 * Check CPU Features (for GCC)
 * (C)2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * 1,May 2012 Initial
 * License: CC-BY-SA
 */

#ifndef _SDL_CPUID_H_
#define _SDL_CPUID_H_

#ifdef __cplusplus
extern "C"
{
#endif
   
// 以下、CPU依存なので移植の場合は同等の関数を作ること
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
struct AGAR_CPUID {
   int use_mmx;
   int use_mmxext;
   int use_sse;
   int use_sse2;
   int use_sse3;
   int use_ssse3;
   int use_sse41;
   int use_sse42;
   int use_sse4a;
   int use_3dnow;
   int use_3dnowp;
   int use_abm;
   int use_avx;
   int use_cmov;
};
#else // 他のアーキテクチャは#ifdefネストで新しく作ること
struct AGAR_CPUID {
   int dummy;
};

#endif

void getCpuID(struct AGAR_CPUID *p);
struct AGAR_CPUID *initCpuID(void);
void detachCpuID(struct AGAR_CPUID *p);
   
#ifdef __cplusplus
}
#endif

#endif