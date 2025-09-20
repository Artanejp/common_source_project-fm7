/*!
  @todo will move to another directory.
*/

#pragma once

#include "../util_rgbconvert.h"
#include "../types/types_video.h"

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


// Note: table strongly recommend to be aligned by sizeof(uint16_vec8_t).
// This is sizeof(uint16) * 8, some compilers may require to align 16bytes(128)
// when using SIMD128 -- 20181105 K.O
template <typename _TBL_T, typename _VAL_T>
	void PrepareBitTransTable(_TBL_T *tbl, _VAL_T on_val, _VAL_T off_val)
{
	__UNLIKELY_IF(tbl == NULL) return;
	for(_VAL_T i = 0; i < 256; i++) {
		_VAL_T n = i;
__DECL_VECTORIZED_LOOP
		for(size_t j = 0; j < 8; j++) {
			tbl->plane_table[i].w[j] = ((n & ((_VAL_T)0x80)) == 0) ? off_val : on_val;
			n <<= 1;
		}
	}
}

// Prepare reverse byte-order table(s).
template <typename _TBL_T, typename _VAL_T>
	void PrepareReverseBitTransTableUint16(_TBL_T *tbl, _VAL_T on_val, _VAL_T off_val)
{
	__UNLIKELY_IF(tbl == NULL) return;
	for(_TBL_T i = 0; i < 256; i++) {
		_VAL_T n = i;
__DECL_VECTORIZED_LOOP
		for(size_t j = 0; j < 8; j++) {
			tbl->plane_table[i].w[j] = ((n & 0x01) == 0) ? off_val : on_val;
			n >>= 1;
		}
	}
}


void DLL_PREFIX Render8Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);

void DLL_PREFIX Render16Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);
void DLL_PREFIX Render2NColors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line, int planes);

void DLL_PREFIX Convert8ColorsToByte_Line(_render_command_data_t *src, uint8_t *dst);
void DLL_PREFIX Convert2NColorsToByte_Line(_render_command_data_t *src, uint8_t *dst, int planes);
void DLL_PREFIX Convert2NColorsToByte_LineZoom2(_render_command_data_t *src, uint8_t *dst, int planes);

