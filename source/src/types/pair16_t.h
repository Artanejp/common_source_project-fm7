#pragma once

#include "./basic_types.h"

typedef union pair16_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h, l;
#else
		uint8_t l, h;
#endif
	} b;
	uint8_t barray[2];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h, l;
#else
		int8_t l, h;
#endif
	} sb;
	int8_t sbarray[2];
	uint16_t u16; // ToDo: Remove
	int16_t s16; // ToDo: Remove
	uint16_t w;
	int16_t sw;
	inline uint16_t __FASTCALL read_raw_2bytes_from_ptr(uint8_t *t)
	{
	#if defined(__NEED_ALIGNED_UINT32)
		union {
			uint16_t ww;
			uint8_t  b[2];
		} tmp16;
		
		tmp16.b[0] = t[0];
		tmp16.b[1] = t[1];
		return tmp16.ww;
	#else
		return *((uint16_t*)t);
	#endif
	}
	inline void __FASTCALL store_raw_2bytes_to_ptr(uint8_t *t, uint16_t nn)
	{
	#if defined(__NEED_ALIGNED_UINT32)
		union {
			uint16_t ww;
			uint8_t  b[2];
		} tmp16;
		tmp16.ww = nn;
		t[0] = tmp16.b[0];
		t[1] = tmp16.b[1];
	#else
		*((uint16_t*)t) = nn;
	#endif
	}
	inline uint16_t __FASTCALL swap_2bytes(uint16_t nn)
	{
	#if defined(__has_builtin) && (__has_builtin(__builtin_bswap16))
		uint16_t tmp;
		tmp = __builtin_bswap16(nn);
		return tmp;
	#else
		union {
			uint16_t ww;
			uint8_t  b[2];
		} tmp16;
		tmp16.ww = nn;
		uint8_t tt = tmp16.b[0];
		tmp16.b[0] = tmp16.b[1];
		tmp16.b[1] = tt;
		return tmp16.ww;
	#endif
	}
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		uint16_t nn = read_raw_2bytes_from_ptr(t);
	#ifdef __BIG_ENDIAN__
		w = swap_2bytes(nn);
	#else
		w = nn;
	#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		uint16_t nn;
	#ifdef __BIG_ENDIAN__
		nn = swap_2bytes(w);
	#else
		nn = w;
	#endif
		store_raw_2bytes_to_ptr(t, nn);
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		uint16_t nn = read_raw_2bytes_from_ptr(t);
	#ifndef __BIG_ENDIAN__
		w = swap_2bytes(nn);
	#else
		w = nn;
	#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		uint16_t nn;
	#ifndef __BIG_ENDIAN__
		nn = swap_2bytes(w);
	#else
		nn = w;
	#endif
		store_raw_2bytes_to_ptr(t, nn);
	}
	
	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
	#ifdef __BIG_ENDIAN__
		w = n;
	#else
		w = swap_2bytes(n);
	#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
	#ifndef __BIG_ENDIAN__
		w = n;
	#else
		w = swap_2bytes(n);
	#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
	#ifdef __BIG_ENDIAN__
		return w;
	#else
		return swap_2bytes(w);
	#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
	#ifndef __BIG_ENDIAN__
		return w;
	#else
		return swap_2bytes(w);
	#endif
	}

} pair16_t;
