/*
 * xm7_types.h
 *  VarTypes for XM7/Agar
 * (C)2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * 
 */
#include <stdint.h>
#include <stddef.h>

#ifdef _WINDOWS
#include <windef.h>
#endif

#ifndef __XM7_TYPES_H
#define __XM7_TYPES_H 1
/*
 * 基本型定義 -> SDL定義にする(コンパイラ依存の吸収 20100802 α66)
 */

typedef uint64_t Uint64;
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t  Uint8;

typedef int64_t Sint64;
typedef int32_t Sint32;
typedef int16_t Sint16;
typedef int8_t  Sint8;
typedef int BOOL;
#define FALSE 0
#define TRUE  1 

// Vector
typedef short int v2si __attribute__ ((__vector_size__(8)));
typedef uint16_t v4si  __attribute__ ((__vector_size__(16)));
typedef uint16_t v8si  __attribute__ ((__vector_size__(32)));
typedef uint32_t v2ui  __attribute__ ((__vector_size__(8)));
typedef int32_t v2ii   __attribute__ ((__vector_size__(8)));
typedef int32_t v4ii   __attribute__ ((__vector_size__(16)));
typedef uint32_t v4ui  __attribute__ ((__vector_size__(16)));
typedef int32_t v8ii   __attribute__ ((__vector_size__(32)));
typedef uint32_t v8ui  __attribute__ ((__vector_size__(32)));

typedef union 
{
        v2si v;
        v2ii vv;
        v2ui uv;
        uint32_t i[2];
        uint16_t s[4];
        uint8_t  b[8];
        int32_t si[2];
        int16_t ss[4];
        int8_t  sb[8];
} v2hi;

typedef union 
{
        v4si v;
        v4ii vv;
        v4ui uv;
        uint32_t i[4];
        uint16_t s[8];
        uint8_t    b[16];
        int32_t si[4];
        int16_t ss[8];
        int8_t  sb[16];
} v4hi;

typedef union 
{
        v8si v;
        v8ii vv;
        v8ui uv;
        v4si v4[2];
        v4ii vv4[2];
       
        uint32_t i[8];
        uint16_t s[16];
        uint8_t  b[32];
        int32_t si[8];
        int16_t ss[16];
        int8_t  sb[32];
} v8hi_t;

#endif //#ifndef __XM7_TYPES_H
