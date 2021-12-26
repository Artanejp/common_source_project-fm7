/*!
  @todo will move to another directory.
*/

#pragma once

#include "../types/types_video.h"

#if defined(_RGB555) || defined(_RGB565)
	scrntype_t DLL_PREFIX  __FASTCALL RGB_COLOR(uint32_t r, uint32_t g, uint32_t b);
	scrntype_t DLL_PREFIX  __FASTCALL RGBA_COLOR(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	uint8_t DLL_PREFIX  __FASTCALL R_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL G_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL B_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL A_OF_COLOR(scrntype_t c);
	#if defined(_RGB565)
inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype r;
	r = n & 0x7c00; // r
	r = r | (n & 0x03e0); // g
	r <<= 1;
	r = r | (n & 0x001f); // b
	return r;
}
	#else // RGB555
inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	return n;
}
	#endif

inline scrntype_t __FASTCALL msb_to_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype_t _n = ((n & 0x8000) != 0) ? 0x0000 : 0xffff;
	return _n;
}

inline scrntype_t __FASTCALL msb_to_alpha_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype_t _n = ((n & 0x8000) != 0) ? 0x0000 : 0xffff;
	return _n; // Not ALPHA
}

#elif defined(_RGB888)
#if defined(__LITTLE_ENDIAN__)
	#define RGB_COLOR(r, g, b)	(((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(r) << 0) | (0xff << 24))
	#define RGBA_COLOR(r, g, b, a)	(((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(r) << 0) | ((uint32_t)(a) << 24))
	#define R_OF_COLOR(c)		(((c)      ) & 0xff)
	#define G_OF_COLOR(c)		(((c) >>  8) & 0xff)
	#define B_OF_COLOR(c)		(((c) >> 16) & 0xff)
	#define A_OF_COLOR(c)		(((c) >> 24) & 0xff)
#else
	#define RGB_COLOR(r, g, b)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0) | (0xff << 24))
	#define RGBA_COLOR(r, g, b, a)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0) | ((uint32_t)(a) << 24))
	#define R_OF_COLOR(c)		(((c) >> 16) & 0xff)
	#define G_OF_COLOR(c)		(((c) >>  8) & 0xff)
	#define B_OF_COLOR(c)		(((c)      ) & 0xff)
	#define A_OF_COLOR(c)		(((c) >> 24) & 0xff)
#endif

inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	scrntype_t r, g, b;
	#if defined(__LITTLE_ENDIAN__)
	r = (n & 0x7c00) << (16 + 1);
	g = (n & 0x03e0) << (8 + 4 + 2);
	b = (n & 0x001f) << (8 + 3);
	return (r | g | b | 0x000000ff);
	#else
	scrntype_t g2;
	r = (n & 0x007c) << (16 + 1 + 8);
	g = (n & 0x0e00) << (8 + 2)
	g2= (n & 0x0030) << (8 + 4 + 2);
	b = (n & 0x1f00) << 3;
	return (r | g | g2 | b | 0x000000ff);
	#endif
}

inline scrntype_t __FASTCALL msb_to_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	scrntype_t _n;
	#if defined(__LITTLE_ENDIAN__)
	_n = ((n & 0x8000) != 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
	#else
	_n = ((n & 0x0080) != 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
	#endif
	return _n;
}

inline scrntype_t __FASTCALL msb_to_alpha_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	scrntype_t _n;
	#if defined(__LITTLE_ENDIAN__)
	_n = ((n & 0x8000) != 0) ? RGBA_COLOR(255, 255, 255, 0) : RGBA_COLOR(255, 255, 255, 255);
	#else
	_n = ((n & 0x0080) != 0) ? RGBA_COLOR(255, 255, 255, 0) : RGBA_COLOR(255, 255, 255, 255);
	#endif
	return _n;
}
#endif

inline scrntype_vec8_t ConvertByteToMonochromePackedPixel(uint8_t src, _bit_trans_table_t *tbl,scrntype_t on_val, scrntype_t off_val)
{
	__DECL_ALIGNED(16) uint16_vec8_t  tmpd;
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;

	uint16_t* vt = (uint16_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t));
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd.w[i] = vt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.w[i] = (tmpd.w[i] == 0) ? off_val: on_val;
	}
	return tmpdd;
}

// Note: Pls. read Note(s) of common.cpp -- 20181105 K.Ohta.
// Tables for below functions must be aligned by 16 (_bit_trans_table_t) or 32(_bit_trans_table_scrn_t).  
void DLL_PREFIX ConvertByteToPackedPixelByColorTable(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table);
void DLL_PREFIX ConvertByteToPackedPixelByColorTable2(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_scrn_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table);
void DLL_PREFIX ConvertByteToSparceUint16(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask);
void DLL_PREFIX ConvertByteToSparceUint8(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask);

// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
inline scrntype_vec8_t ConvertByteToPackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
{
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;
	scrntype_t* vt = (scrntype_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(scrntype_vec8_t)); 
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.w[i] = vt[i];
	}
	return tmpdd;
}

// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
inline scrntype_vec16_t ConvertByteToDoublePackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
{
	__DECL_ALIGNED(32) scrntype_vec16_t tmpdd;
	__DECL_ALIGNED(32) scrntype_vec8_t tmpd;

	scrntype_t* vt = (scrntype_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(scrntype_vec8_t)); 
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd.w[i] = vt[i];
	}
	int j = 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		tmpdd.w[i]     = tmpd.w[j];
		tmpdd.w[i + 1] = tmpd.w[j];
		j++;
	}
	return tmpdd;
}

// Table must be initialize ON_COLOR : OFF_COLOR
inline void ConvertByteToDoubleMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_t* vt = (uint16_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t)); 

	__DECL_ALIGNED(16) uint8_t d[16];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd.w[i] = vt[i];
	}
	int j = 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		d[i]     = (uint8_t)(tmpd.w[j]);
		d[i + 1] = (uint8_t)(tmpd.w[j]);
		j++;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		dst[i] = d[i];
	}
}

inline void ConvertByteToMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_t* vt = (uint16_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t)); 

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd.w[i] = vt[i];
	}

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i] = (uint8_t)(tmpd.w[i]);
	}
}

inline void ConvertRGBTo8ColorsUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_t*  rvt = (uint16_t*)___assume_aligned(&(rtbl->plane_table[r]), sizeof(uint16_vec8_t));
	uint16_t*  gvt = (uint16_t*)___assume_aligned(&(gtbl->plane_table[g]), sizeof(uint16_vec8_t));
	uint16_t*  bvt = (uint16_t*)___assume_aligned(&(btbl->plane_table[b]), sizeof(uint16_vec8_t));
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = rvt[i];
	}
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpg(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpb(8);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpg[i] = gvt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpb[i] = bvt[i];
	}
//	tmpd.v = rvt[r].v;
	tmpd = tmpd | tmpg;
	tmpd = tmpd | tmpb;
	__LIKELY_IF(shift >= 0) {
		tmpd = tmpd >> (uint16_t)shift;
	} else {
		tmpd = tmpd << (uint16_t)(-shift);
	}

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i] = (uint8_t)(tmpd[i]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Left(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_t*  rvt = (uint16_t*)___assume_aligned(&(rtbl->plane_table[r]), sizeof(uint16_vec8_t));
	uint16_t*  gvt = (uint16_t*)___assume_aligned(&(gtbl->plane_table[g]), sizeof(uint16_vec8_t));
	uint16_t*  bvt = (uint16_t*)___assume_aligned(&(btbl->plane_table[b]), sizeof(uint16_vec8_t));
	
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = rvt[i];
	}
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpg(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpb(8);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpg[i] = gvt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpb[i] = bvt[i];
	}

	tmpd = tmpd | tmpg;
	tmpd = tmpd | tmpb;
	__LIKELY_IF(shift >= 0) {
		tmpd = tmpd >> (uint16_t)shift;
	} else {
		tmpd = tmpd << (uint16_t)(-shift);
	}

__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 8; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd[j]);
		dst[i + 1] = (uint8_t)(tmpd[j]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Right(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_t*  rvt = (uint16_t*)___assume_aligned(&(rtbl->plane_table[r]), sizeof(uint16_vec8_t));
	uint16_t*  gvt = (uint16_t*)___assume_aligned(&(gtbl->plane_table[g]), sizeof(uint16_vec8_t));
	uint16_t*  bvt = (uint16_t*)___assume_aligned(&(btbl->plane_table[b]), sizeof(uint16_vec8_t));

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = rvt[i];
	}
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpg(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpb(8);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpg[i] = gvt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpb[i] = bvt[i];
	}

	tmpd = tmpd | tmpg;
	tmpd = tmpd | tmpb;
	__LIKELY_IF(shift >= 0) {
		tmpd = tmpd >> (uint16_t)shift;
	} else {
		tmpd = tmpd << (uint16_t)(-shift);
	}
	
__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 4; i < 8; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd[j]);
		dst[i + 1] = (uint8_t)(tmpd[j]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Double(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_t*  rvt = (uint16_t*)___assume_aligned(&(rtbl->plane_table[r]), sizeof(uint16_vec8_t));
	uint16_t*  gvt = (uint16_t*)___assume_aligned(&(gtbl->plane_table[g]), sizeof(uint16_vec8_t));
	uint16_t*  bvt = (uint16_t*)___assume_aligned(&(btbl->plane_table[b]), sizeof(uint16_vec8_t));

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = rvt[i];
	}
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpg(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpb(8);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpg[i] = gvt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpb[i] = bvt[i];
	}

	tmpd = tmpd | tmpg;
	tmpd = tmpd | tmpb;
	__LIKELY_IF(shift >= 0) {
		tmpd = tmpd >> (uint16_t)shift;
	} else {
		tmpd = tmpd << (uint16_t)(-shift);
	}

__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 16; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd[j]);
		dst[i + 1] = (uint8_t)(tmpd[j]);
	}
}

inline void ConvertByteToMonochromeUint8Cond_Zoom2(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_t*  vt = (uint16_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t));

	__DECL_ALIGNED(16) uint8_t d[16];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = vt[i];
	}

	int j = 0;
	__DECL_ALIGNED(16) std::valarray<bool> tmpdet(8);
	tmpdet = (tmpd == (uint16_t)0) ;
	__DECL_ALIGNED(16) std::valarray<uint8_t> dd(8);
	for(int i = 0; i < 8; i++) {
		dd[i] = (tmpdet[i]) ? off_color : on_color;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		d[i    ] = dd[j];
		d[i + 1] = dd[j];
		j++;
	}
//		d[i]     = (tmpd[j] == 0) ? off_color : on_color;
//		d[i + 1] = (tmpd[j] == 0) ? off_color : on_color;
//		j++;
//	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		dst[i] = d[i];
	}
}

inline void ConvertByteToMonochromeUint8Cond(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
{
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t));

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd[i] = vt->w[i];
	}
	__DECL_ALIGNED(16) std::valarray<bool> tmpdet(8);
	tmpdet = (tmpd == (uint16_t)0) ;
	__DECL_ALIGNED(16) std::valarray<uint8_t> dd(8);
	for(int i = 0; i < 8; i++) {
		dd[i] = (tmpdet[i]) ? off_color : on_color;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i]  = dd[i];
	}
}

void DLL_PREFIX PrepareBitTransTableUint16(_bit_trans_table_t *tbl, uint16_t on_val, uint16_t off_val);
void DLL_PREFIX PrepareBitTransTableScrnType(_bit_trans_table_scrn_t *tbl, scrntype_t on_val, scrntype_t off_val);
void DLL_PREFIX PrepareReverseBitTransTableUint16(_bit_trans_table_t *tbl, uint16_t on_val, uint16_t off_val);
void DLL_PREFIX PrepareReverseBitTransTableScrnType(_bit_trans_table_scrn_t *tbl, scrntype_t on_val, scrntype_t off_val);

void DLL_PREFIX Render8Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);

void DLL_PREFIX Render16Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);
void DLL_PREFIX Render2NColors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line, int planes);

void DLL_PREFIX Convert8ColorsToByte_Line(_render_command_data_t *src, uint8_t *dst);
void DLL_PREFIX Convert2NColorsToByte_Line(_render_command_data_t *src, uint8_t *dst, int planes);
void DLL_PREFIX Convert2NColorsToByte_LineZoom2(_render_command_data_t *src, uint8_t *dst, int planes);

