#pragma once

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

	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1];
		#else
		w = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		*((uint16_t*)t) = w;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		w = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		*((uint16_t*)t) = w;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	
	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		w = n;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.w = n;
		b.l = bigv.b.l; b.h = bigv.b.h;
		#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.w = n;
		b.l = littlev.b.l; b.h = littlev.b.h;
		#else
		w = n;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return w;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h;
		return bigv.w;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h;
		return littlev.w;
		#else
		return w;
		#endif
	}

} pair16_t;
