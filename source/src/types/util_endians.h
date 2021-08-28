#pragma once

#include "../types/pair16_t.h"
#include "../types/pair32_t.h"
#include "../types/pair64_t.h"

inline uint32_t __FASTCALL EndianToLittle_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.d = x;
	return xx.get_4bytes_le_to();
}

inline uint16_t  __FASTCALL EndianToLittle_WORD(uint16_t x)
{
	pair16_t xx;
	xx.w = x;
	return xx.get_2bytes_le_to();
}

inline uint32_t  __FASTCALL EndianFromLittle_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.set_4bytes_le_from(x);
	return xx.d;
}

inline uint16_t  __FASTCALL EndianFromLittle_WORD(uint16_t x)
{
	pair16_t xx;
	xx.set_2bytes_le_from(x);
	return xx.w;
}


inline uint32_t  __FASTCALL  EndianToBig_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.d = x;
	return xx.get_4bytes_be_to();
}

inline uint16_t  __FASTCALL EndianToBig_WORD(uint16_t x)
{
	pair16_t xx;
	xx.w = x;
	return xx.get_2bytes_be_to();
}

inline uint32_t  __FASTCALL EndianFromBig_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.set_4bytes_be_from(x);
	return xx.d;
}

inline uint16_t  __FASTCALL EndianFromBig_WORD(uint16_t x)
{
	pair16_t xx;
	xx.set_2bytes_be_from(x);
	return xx.w;
}

inline uint64_t __FASTCALL ExchangeEndianU64(uint64_t __in)
{
	__DECL_ALIGNED(16) pair64_t __i, __o;
	__i.q = __in;
	int ij = 7;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 8; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
//	__o.b.h7  = __i.b.l;
//	__o.b.h6  = __i.b.h;
//	__o.b.h5  = __i.b.h2;
//	__o.b.h4  = __i.b.h3;
//	__o.b.h3  = __i.b.h4;
//	__o.b.h2  = __i.b.h5;
//	__o.b.h   = __i.b.h6;
//	__o.b.l   = __i.b.h7;
	return __o.q;
}

inline int64_t __FASTCALL ExchangeEndianS64(uint64_t __in)
{
	__DECL_ALIGNED(16) pair64_t __i, __o;
	__i.q = __in;
	int ij = 7;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 8; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	return __o.sq;
}
inline uint32_t __FASTCALL ExchangeEndianU32(uint32_t __in)
{
	__DECL_ALIGNED(4) pair32_t __i, __o;
	__i.d = __in;
	int ij = 3;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 4; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	
//	__o.b.h3 = __i.b.l;
//	__o.b.h2 = __i.b.h;
//	__o.b.h  = __i.b.h2;
//	__o.b.l  = __i.b.h3;
	return __o.d;
}

inline int32_t __FASTCALL ExchangeEndianS32(uint32_t __in)
{
	__DECL_ALIGNED(4) pair32_t __i, __o;
	__i.d = __in;
	int ij = 3;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 4; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	return __o.sd;
}

inline uint16_t __FASTCALL ExchangeEndianU16(uint16_t __in)
{
	pair16_t __i, __o;
	__i.u16 = __in;
	__o.b.h = __i.b.l;
	__o.b.l  = __i.b.h;
	return __o.u16;
}

inline int16_t __FASTCALL ExchangeEndianS16(uint16_t __in)
{
	pair16_t __i, __o;
	__i.u16 = __in;
	__o.b.h = __i.b.l;
	__o.b.l = __i.b.h;
	return __o.s16;
}

#define swap_endian_u16(foo) ExchangeEndianU16(foo)
