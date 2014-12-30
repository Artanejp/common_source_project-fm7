/*
 * XM7: Cache Wrapper
 *  (C) Jun 01,2014 K.Ohta <whatisthis.sowhat@gmail.com>
 */
#ifndef CACHE_WRAPPER_H_INCLUDED
#define CACHE_WRAPPER_H_INCLUDED


#include "xm7.h"
#include "xm7_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef _CACHE_FOR_PREFETCH
#if defined(__x86_64)
   /*
    * x86_64
    */
#define _CACHE_FOR_PREFETCH 1
#define _CACHE_LINE_SIZE 64 // AMD64 has 64 bytes cache line.
   
#elif defined(__i386)
   /*
    * IA32
    */
#define _CACHE_FOR_PREFETCH 1

 #if defined(__SSE__) || defined(__SSE2__) || defined(__MMX__)
  #define _CACHE_LINE_SIZE 64 // If cache >= pentiumpro, cache line is assumed to 64.
 #else
  #define _CACHE_LINE_SIZE 32 // Mostly safety?
 #endif
   
#else 
   /*
    * Other Archtecture(s)
    */
#define _CACHE_LINE_SIZE 16 // Mostly safety?
#endif


static __inline__ void _prefetch_data_read_l1(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 0, 1);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   
   
static __inline__ void _prefetch_data_write_l1(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 1, 1);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   

static __inline__ void _prefetch_data_read_l2(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 0, 2);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   
   
static __inline__ void _prefetch_data_write_l2(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 1, 2);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   
   
static __inline__ void _prefetch_data_read_permanent(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 0, 0);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   
   
static __inline__ void _prefetch_data_write_permanent(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 1, 0);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   

static __inline__ void _prefetch_data_read_temporal(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 0, 2);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   
   
static __inline__ void _prefetch_data_write_temporal(void *p, int size) //__attribute__((always_inline))
#if defined(_CACHE_FOR_PREFETCH)
{
        int i;
        Uint8 *pp = (Uint8 *)p;
   
        for(i = 0; i < size; i += _CACHE_LINE_SIZE) __builtin_prefetch(&pp[i], 1, 2);
}
#else // oops, this archtecture does't have prefetch    
     {
     }
#endif   

#ifdef __cplusplus
}
#endif


#endif // CACHE_WRAPPER_H_INCLUDED
