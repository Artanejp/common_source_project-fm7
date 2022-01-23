// ---------------------------------------------------------------------------
//	misc.h
//	Copyright (C) cisc 1998, 1999.
// ---------------------------------------------------------------------------
//	$Id: misc.h,v 1.5 2002/05/31 09:45:20 cisc Exp $

#ifndef __FMGEN_MISC_H
#define __FMGEN_MISC_H
#include <algorithm>
#include <cstdint>

inline int Max(int x, int y) { return std::max(x, y); }
inline int Min(int x, int y) { return std::min(x, y); }

inline int Abs(int x) { return x >= 0 ? x : -x; }

//#if defined(__cplusplus) && (__cplusplus >= 201703L)
//#define Limit(foo, max, min) std::clamp((int)foo, (int)min, (int)max)
//#else
inline int Limit(int v, int max, int min) 
{
	return v > max ? max : (v < min ? min : v); 
}
//#endif

#if defined(__has_builtin)
	#if (__has_builtin(__builtin_bswap32))
inline unsigned int BSwap(unsigned int a)
{
	return (unsigned int)__builtin_bswap32((uint32_t)a);
}
	#else
inline unsigned int BSwap(unsigned int a)
{
	return (a >> 24) | ((a >> 8) & 0xff00) | ((a << 8) & 0xff0000) | (a << 24);
}
	#endif
#else
inline unsigned int BSwap(unsigned int a)
{
	return (a >> 24) | ((a >> 8) & 0xff00) | ((a << 8) & 0xff0000) | (a << 24);
}
#endif

inline unsigned int NtoBCD(unsigned int a)
{
	return ((a / 10) << 4) + (a % 10);
}

inline unsigned int BCDtoN(unsigned int v)
{
	return (v >> 4) * 10 + (v & 15);
}


#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <numeric>
using std::gcd;
#else
template<class T>
inline T gcd(T x, T y)
{
	T t;
	while (y)
	{
		t = x % y;
		x = y;
		y = t;
	}
	return x;
}

#endif

template<class T>
T bessel0(T x)
{
	T p, r, s;

	r = 1.0;
	s = 1.0;
	p = (x / 2.0) / s;

	while (p > 1.0E-10)
	{
		r += p * p;
		s += 1.0;
		p *= (x / 2.0) / s;
	}
	return r;
}


#endif // MISC_H

