#pragma once

#include "./basic_types.h"

typedef union pair32_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h3, h2, h, l;
#else
		uint8_t l, h, h2, h3;
#endif
	} b;
	uint8_t barray[4];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h3, h2, h, l;
#else
		int8_t l, h, h2, h3;
#endif
	} sb;
	int8_t sbarray[4];
	struct {
#ifdef __BIG_ENDIAN__
		uint16_t h, l;
#else
		uint16_t l, h;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16_t h, l;
#else
		int16_t l, h;
#endif
	} sw;
	struct {
#ifdef __BIG_ENDIAN__
		pair16_t h, l;
#else
		pair16_t l, h;
#endif
	} p16;
	uint32_t d;
	int32_t sd;
	float f; // single float
  
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
	inline uint32_t __FASTCALL read_raw_4bytes_from_ptr(uint8_t *t)
	{
	#if defined(__NEED_ALIGNED_UINT32)
		union {
			uint32_t dd;
			uint8_t  b[4];
		} tmp;
		
		tmp.b[0] = t[0];
		tmp.b[1] = t[1];
		tmp.b[2] = t[2];
		tmp.b[3] = t[3];
		return tmp.dd;
	#else
		return *((uint32_t*)t);
	#endif
	}
	inline void __FASTCALL store_raw_4bytes_to_ptr(uint8_t *t, uint32_t nn)
	{
	#if defined(__NEED_ALIGNED_UINT32)
		union {
			uint32_t dd;
			uint8_t  b[4];
		} tmp;
		tmp.dd = nn;
		t[0] = tmp.b[0];
		t[1] = tmp.b[1];
		t[2] = tmp.b[2];
		t[3] = tmp.b[3];
	#else
		*((uint32_t*)t) = nn;
	#endif
	}
	inline uint32_t __FASTCALL swap_4bytes(uint32_t nn)
	{
	#if defined(__has_builtin) && (__has_builtin(__builtin_bswap32))
		uint32_t tmp;
		tmp = __builtin_bswap32(nn);
		return tmp;
	#else
		union {
			uint32_t dd;
			uint8_t  b[4];
		} tmp;
		tmp.dd = nn;
		uint8_t tt0 = tmp.b[0];
		uint8_t tt1 = tmp.b[1];
		tmp.b[0] = tmp.b[3];
		tmp.b[1] = tmp.b[2];
		tmp.b[2] = tt1;
		tmp.b[3] = tt0;
		return tmp.dd;
	#endif
	}
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		d = 0;
		uint16_t tmp = read_raw_2bytes_from_ptr(t);
	#ifdef __BIG_ENDIAN__
		w.l = swap_2bytes(tmp);
	#else
		w.l = tmp;
	#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		uint16_t tmp;
	#ifdef __BIG_ENDIAN__
		tmp = swap_2bytes(w.l);
	#else
		tmp = w.l;
	#endif
		store_raw_2bytes_to_ptr(t, tmp);		
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		d = 0;
		uint16_t tmp = read_raw_2bytes_from_ptr(t);
	#ifndef __BIG_ENDIAN__
		w.l = swap_2bytes(tmp);
	#else
		w.l = tmp;
	#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		uint16_t tmp;
	#ifndef __BIG_ENDIAN__
		tmp = swap_2bytes(w.l);
	#else
		tmp = w.l;
	#endif
		store_raw_2bytes_to_ptr(t, tmp);		
	}
	inline void __FASTCALL read_4bytes_le_from(uint8_t *t)
	{
		uint32_t tmp = read_raw_4bytes_from_ptr(t);
	#ifdef __BIG_ENDIAN__
		d = swap_4bytes(tmp);
	#else
		d = tmp;
	#endif
	}
	inline void __FASTCALL write_4bytes_le_to(uint8_t *t)
	{
		uint32_t tmp;
	#ifdef __BIG_ENDIAN__
		tmp = swap_4bytes(d);
	#else
		tmp = d;
	#endif
		store_raw_4bytes_to_ptr(t, tmp);		
	}
	inline void __FASTCALL read_4bytes_be_from(uint8_t *t)
	{
		uint32_t tmp = read_raw_4bytes_from_ptr(t);
	#ifndef __BIG_ENDIAN__
		d = swap_4bytes(tmp);
	#else
		d = tmp;
	#endif
	}
	inline void __FASTCALL write_4bytes_be_to(uint8_t *t)
	{
		uint32_t tmp;
	#ifndef __BIG_ENDIAN__
		tmp = swap_4bytes(d);
	#else
		tmp = d;
	#endif
		store_raw_4bytes_to_ptr(t, tmp);		
	}

	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		d = 0;
	#ifndef __BIG_ENDIAN__
		w.l = swap_2bytes(n);
	#else
		w.l = n;
	#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		d = 0;
	#ifdef __BIG_ENDIAN__
		w.l = swap_2bytes(n);
	#else
		w.l = n;
	#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
	#ifdef __BIG_ENDIAN__
		return w.l;
	#else
		uint16_t tmp = swap_2bytes(w.l);
		return tmp;
	#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
	#ifndef __BIG_ENDIAN__
		return w.l;
	#else
		uint16_t tmp = swap_2bytes(w.l);
		return tmp;
	#endif
	}
	
	inline void __FASTCALL set_4bytes_be_from(uint32_t n)
	{
	#ifndef __BIG_ENDIAN__
		d = swap_4bytes(n);
	#else
		d = n;
	#endif
	}
	inline void __FASTCALL set_4bytes_le_from(uint32_t n)
	{
	#ifdef __BIG_ENDIAN__
		d = swap_4bytes(n);
	#else
		d = n;
	#endif
	}
	inline uint32_t __FASTCALL get_4bytes_be_to()
	{
	#ifndef __BIG_ENDIAN__
		return swap_4bytes(d);
	#else
		return d;
	#endif
	}
	inline uint32_t __FASTCALL get_4bytes_le_to()
	{
	#ifdef __BIG_ENDIAN__
		return swap_4bytes(d);
	#else
		return d;
	#endif
	}
} pair32_t;

