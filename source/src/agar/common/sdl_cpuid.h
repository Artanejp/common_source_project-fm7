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
   BOOL use_mmx;
   BOOL use_mmxext;
   BOOL use_sse;
   BOOL use_sse2;
   BOOL use_sse3;
   BOOL use_ssse3;
   BOOL use_sse41;
   BOOL use_sse42;
   BOOL use_sse4a;
   BOOL use_3dnow;
   BOOL use_3dnowp;
   BOOL use_abm;
   BOOL use_avx;
   BOOL use_cmov;
};
#else // 他のアーキテクチャは#ifdefネストで新しく作ること
struct AGAR_CPUID {
   BOOL dummy;
};

#endif

void getCpuID(struct AGAR_CPUID *p);
struct AGAR_CPUID *initCpuID(void);
void detachCpuID(struct AGAR_CPUID *p);
   
#ifdef __cplusplus
}
#endif

#endif