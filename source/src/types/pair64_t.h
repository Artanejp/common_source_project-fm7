#pragma once

typedef union pair64_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	uint8_t barray[8];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		int8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} sb;
	uint8_t sbarray[8];
	struct {
#ifdef __BIG_ENDIAN__
		uint16_t h3, h2, h, l;
#else
		uint16_t l, h, h2, h3;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16_t h3, h2, h, l;
#else
		int16_t l, h, h2, h3;
#endif
	} sw;
	struct {
#ifdef __BIG_ENDIAN__
		pair16_t h3, h2, h, l;
#else
		pair16_t l, h, h2, h3;
#endif
	} p16;
	struct {
#ifdef __BIG_ENDIAN__
		uint32_t h, l;
#else
		uint32_t l, h;
#endif
	} d;
	struct {
#ifdef __BIG_ENDIAN__
		int32_t h, l;
#else
		int32_t l, h;
#endif
	} sd;
	struct {
#ifdef __BIG_ENDIAN__
		pair32_t h, l;
#else
		pair32_t l, h;
#endif
	} p32;
	struct {
#ifdef __BIG_ENDIAN__
		float h, l;
#else
		float l, h;
#endif
	} f32;
	uint64_t q;
	int64_t sq;
	double df; // double float
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; //b.h2 = b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#else
		// ToDo: Support unaligned.
		w.l = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		// ToDo: Support unaligned.
		*((uint16_t*)t) = w.l;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		w.l = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		*((uint16_t*)t) = w.l;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_le_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
		#else
		// ToDo: Support unaligned.
		d.l = *((uint32_t*)t);
		#endif
	}
	inline void __FASTCALL write_4bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
		#else
		// ToDo: Support unaligned.
		*((uint32_t*)t) = d.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_be_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		d.l = *((uint32_t*)t);
		#else
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
		#endif
	}
	inline void __FASTCALL write_4bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		*((uint32_t*)t) = d.l;
		#else
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
		#endif
	}

	// Note: Expect to optimize by SIMD when aligned this value. 20210811 K.O
	inline void __FASTCALL read_8bytes_le_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ii];
			}
		#endif
//		b.l = t[0];  b.h = t[1];  b.h2 = t[2]; b.h3 = t[3];
//		b.h4 = t[4]; b.h5 = t[5]; b.h6 = t[6]; b.h7 = t[7];
	}
	inline void __FASTCALL write_8bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ij] = barray[ii];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ii] = barray[ii];
			}
		#endif
//		t[0] = b.l;  t[1] = b.h;  t[2] = b.h2; t[3] = b.h3;
//		t[4] = b.h4; t[5] = b.h5; t[6] = b.h6; t[7] = b.h7;
	}
	inline void __FASTCALL read_8bytes_be_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ii];
			}
		#else
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ij];
				ij--;
			}
		#endif
//		b.h7 = t[0]; b.h6 = t[1]; b.h5 = t[2]; b.h4 = t[3];
//		b.h3 = t[4]; b.h2 = t[5]; b.h = t[6];  b.l = t[7];
	}
	inline void __FASTCALL write_8bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ii] = barray[ii];
			}
		#else
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ij] = barray[ii];
				ij--;
			}
		#endif
//		t[0] = b.h7; t[1] = b.h6; t[2] = b.h5; t[3] = b.h4;
//		t[4] = b.h3; t[5] = b.h2; t[6] = b.h;  t[7] = b.l;
	}

	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
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
//		b.h2 = 0; b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.w = n;
		b.l = littlev.b.l; b.h = littlev.b.h;
//		b.h2 = 0; b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#else
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
		q = 0;
		#ifdef __BIG_ENDIAN__
		d.l = n;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.dw = n;
		b.l = bigv.b.l; b.h = bigv.b.h; b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
//		b.h4 = 0;       b.h5 = 0;       b.h6 = 0;         b.h7 = 0;
		#endif
	}
	inline void __FASTCALL set_4bytes_le_from(uint32_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.dw = n;
		b.l = littlev.b.l; b.h = littlev.b.h; b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
		//b.h4 = 0;          b.h5 = 0;          b.h6 = 0;            b.h7 = 0;
		#else
		d.l = n;
		#endif
	}
	inline uint32_t __FASTCALL  get_4bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return d.l;
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
		return d.l;
		#endif
	}

	inline void __FASTCALL set_8bytes_be_from(uint64_t n)
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t h7, h6, h5, h4, h3, h2, h, l;
			}b;
			uint8_t barray[8];
		} bigv;
		bigv.qw = n;
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = bigv.barray[ii];
			}
		#else
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = bigv.barray[ij];
				ij--;
			}
		#endif
		//b.l = bigv.b.l;   b.h = bigv.b.h;   b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
		//b.h4 = bigv.b.h4; b.h5 = bigv.b.h5; b.h6 = bigv.b.h6; b.h7 = bigv.b.h7;
	}
	inline void __FASTCALL set_8bytes_le_from(uint64_t n)
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t l, h, h2, h3, h4, h5, h6, h7;
			}b;
			uint8_t barray[8];
		} littlev;
		littlev.qw = n;
		#ifdef __BIG_ENDIAN__
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = littlev.barray[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = littlev.barray[ii];
			}
		#endif
//		b.l = littlev.b.l;   b.h = littlev.b.h;   b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
//		b.h4 = littlev.b.h4; b.h5 = littlev.b.h5; b.h6 = littlev.b.h6; b.h7 = littlev.b.h7;
	}
	inline uint64_t __FASTCALL get_8bytes_be_to()
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t h7, h6, h5, h4, h3, h2, h, l;
			}b;
			uint8_t barray[8];
		} bigv;
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				bigv.barray[ii] = barray[ii];
			}
		#else
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				bigv.barray[ii] = barray[ij];
				ij--;
			}
		#endif
		//bigv.b.l = b.l;   bigv.b.h = b.h;   bigv.b.h2 = b.h2; bigv.b.h3 = b.h3;
		//bigv.b.h4 = b.h4; bigv.b.h5 = b.h5; bigv.b.h6 = b.h6; bigv.b.h7 = b.h7;
		return bigv.qw;
	}
	inline uint64_t __FASTCALL get_8bytes_le_to()
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t l, h, h2, h3, h4, h5, h6, h7;
			}b;
			uint8_t barray[8];
		} littlev;
		#ifdef __BIG_ENDIAN__
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				littlev.barray[ii] = barray[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				littlev.barray[ii] = barray[ii];
			}
		#endif
		//littlev.b.l = b.l;   littlev.b.h = b.h;   littlev.b.h2 = b.h2; littlev.b.h3 = b.h3;
		//littlev.b.h4 = b.h4; littlev.b.h5 = b.h5; littlev.b.h6 = b.h6; littlev.b.h7 = b.h7;
		return littlev.qw;
	}

} pair64_t;
