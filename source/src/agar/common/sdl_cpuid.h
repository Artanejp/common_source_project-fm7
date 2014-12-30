/*
 * Check CPU Features (for GCC)
 * (C)2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * 1,May 2012 Initial
 * License: CC-BY-SA
 */


#ifdef __cplusplus
extern "C"
{
#endif
   
// 以下、CPU依存なので移植の場合は同等の関数を作ること
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
struct XM7_CPUID {
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
struct XM7_CPUID {
   BOOL dummy;
};

#endif

void getCpuID(struct XM7_CPUID *p);
struct XM7_CPUID *initCpuID(void);
void detachCpuID(struct XM7_CPUID *p);
   
#ifdef __cplusplus
}
#endif
   