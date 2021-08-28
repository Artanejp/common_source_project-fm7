#pragma once

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
  
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		d = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1];
		#else
		// ToDo: Support for unaligned.
		w.l = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		// ToDo: Support for unaligned.
		*((uint16_t*)t) = w.l;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		d = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		w.l = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		*((uint16_t*)t) = w.l;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_le_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
		#else
		// ToDo: Support for unaligned.
		d = *((uint32_t*)t);
		#endif
	}
	inline void __FASTCALL write_4bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
		#else
		// ToDo: Support for unaligned.
		*((uint32_t*)t) = d;
		#endif
	}
	inline void __FASTCALL read_4bytes_be_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		d = *((uint32_t*)t);
		#else
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
		#endif
	}
	inline void __FASTCALL write_4bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		*((uint32_t*)t) = d;
		#else
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
		#endif
	}

	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		w.h = 0;
		w.l = n;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.w = n;
		b.l = bigv.b.l; b.h = bigv.b.h;
		w.h = 0;
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
		w.h = 0;
		#else
		w.h = 0;
		w.l = n;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return w.l;
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
		return w.l;
		#endif
	}
	
	inline void __FASTCALL set_4bytes_be_from(uint32_t n)
	{
		#ifdef __BIG_ENDIAN__
		d = n;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.dw = n;
		b.l = bigv.b.l; b.h = bigv.b.h; b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
		#endif
	}
	inline void __FASTCALL set_4bytes_le_from(uint32_t n)
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.dw = n;
		b.l = littlev.b.l; b.h = littlev.b.h; b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
		#else
		d = n;
		#endif
	}
	inline uint32_t __FASTCALL get_4bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return d;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h; bigv.b.h2 = b.h2; bigv.b.h3 = b.h3;
		return bigv.dw;
		#endif
	}
	inline uint32_t __FASTCALL get_4bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h; littlev.b.h2 = b.h2; littlev.b.h3 = b.h3;
		return littlev.dw;
		#else
		return d;
		#endif
	}
} pair32_t;

