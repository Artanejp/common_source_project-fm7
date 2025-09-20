/*!
  @todo will move to another directory.
*/

#pragma once

#include "../common.h"
#include "../types/types_video.h"

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

static inline scrntype_vec8_t ConvertByteToMonochromePackedPixel(uint8_t src, _bit_trans_table_t *tbl,scrntype_t on_val, scrntype_t off_val)
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


// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
static inline scrntype_vec8_t ConvertByteToPackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
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
static inline scrntype_vec16_t ConvertByteToDoublePackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
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
static inline void ConvertByteToDoubleMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
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

static inline void ConvertByteToMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
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

static inline void ConvertRGBTo8ColorsUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
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

static inline void ConvertRGBTo8ColorsUint8_Zoom2Left(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
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

static inline void ConvertRGBTo8ColorsUint8_Zoom2Right(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
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

static inline void ConvertRGBTo8ColorsUint8_Zoom2Double(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
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

static inline void ConvertByteToMonochromeUint8Cond_Zoom2(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
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

static inline void ConvertByteToMonochromeUint8Cond(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
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



static void Render8Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line)
{
	__UNLIKELY_IF(src == NULL) return;
	__UNLIKELY_IF(dst == NULL) return;

//__DECL_VECTORIZED_LOOP
//	for(int i = 0; i < 3; i++) {
//		if(src->bit_trans_table[i] == NULL) return;
//		if(src->data[i] == NULL) return;
//	}
	__DECL_ALIGNED(32) std::valarray<scrntype_t> palette(8); // fallback
	uint16_vec8_t *vpb = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[0], sizeof(uint16_vec8_t));
	uint16_vec8_t *vpr = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[1], sizeof(uint16_vec8_t));
	uint16_vec8_t *vpg = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[2], sizeof(uint16_vec8_t));

	uint32_t x;
	__DECL_ALIGNED(16) std::valarray<uint32_t> offset(src->voffset, 4);
	const uint32_t mask = src->addrmask;
	const uint32_t offsetmask = src->addrmask2;

	__UNLIKELY_IF(src->palette == NULL) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			palette[i] = RGB_COLOR(((i & 2) << 5) | 0x1f,
										 ((i & 4) << 5) | 0x1f,
										 ((i & 1) << 5) | 0x1f);
		}
	} else {
		palette = std::valarray<scrntype_t>(src->palette, 8);
	}
	uint8_t *bp = &(src->data[0][src->baseaddress[0]]);
	uint8_t *rp = &(src->data[1][src->baseaddress[1]]);
	uint8_t *gp = &(src->data[2][src->baseaddress[2]]);

	__DECL_ALIGNED(8) std::valarray<uint8_t> brg(4);
	__DECL_ALIGNED(8) std::valarray<uint8_t> rmask((const uint8_t)0, 4);
	__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 3; ii++) {
			if(src->is_render[ii]) rmask[ii] = 0xff;
		}
	enum {
		_b = 0,
		_r,
		_g,
		_n
	};

	const int shift = src->shift;

	x = src->begin_pos;
	uint32_t n = x;
	__DECL_ALIGNED(32) std::valarray<scrntype_t> sline(RGBA_COLOR(31, 31, 31, 255), 8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> vtmpb(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> vtmpr(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> vtmpg(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> tmpdd(8);
	for(uint32_t xx = 0; xx < src->render_width; xx++) {
		brg[_b] =  bp[(offset[0] + n) & mask];
		brg[_r] =  rp[(offset[1] + n) & mask];
		brg[_g] =  gp[(offset[2] + n) & mask];

		brg &= rmask;
		// Note: Should pre-allocate valarrays to improbe speed.

		uint16_t *ppb = (uint16_t*) ___assume_aligned(vpb[brg[_b]].w, sizeof(uint16_t) * 8);
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			vtmpb[i] = ppb[i];
		}
		uint16_t *ppr = (uint16_t*) ___assume_aligned(vpr[brg[_r]].w, sizeof(uint16_t) * 8);
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			vtmpr[i] = ppr[i];
		}
		uint16_t *ppg = (uint16_t*) ___assume_aligned(vpg[brg[_g]].w, sizeof(uint16_t) * 8);
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			vtmpg[i] = ppg[i];
		}
		tmpd = vtmpb | vtmpr | vtmpg;
		__LIKELY_IF(shift >= 0) {
			tmpd = tmpd >> (uint16_t)shift;
		} else {
			tmpd = tmpd << (uint16_t)(-shift);
		}

		n = (n + 1) & offsetmask;

	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			tmpdd[i] = palette[tmpd[i]];
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			dst[i] = tmpdd[i];
		}
		dst += 8;
		__LIKELY_IF(dst2 != nullptr) {
			if(scan_line) {
#if defined(_RGB555) || defined(_RGBA565)
				tmpdd >>= (scrntype_t)2;
#else // 24bit
				tmpdd >>= (scrntype_t)3;
#endif
				tmpdd = tmpdd & sline;
			}
		__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst2[i] = tmpdd[i];
			}
			dst2 += 8;
		}
	}
}

static void Render16Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line)
{
	__UNLIKELY_IF(src == NULL) return;
	__UNLIKELY_IF(dst == NULL) return;

//__DECL_VECTORIZED_LOOP
//	for(int i = 0; i < 3; i++) {
//		if(src->bit_trans_table[i] == NULL) return;
//		if(src->data[i] == NULL) return;
//	}
	__DECL_ALIGNED(32) std::valarray<scrntype_t> palette(16); // fallback
	uint16_vec8_t *vpb = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[0], sizeof(uint16_vec8_t));
	uint16_vec8_t *vpr = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[1], sizeof(uint16_vec8_t));
	uint16_vec8_t *vpg = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[2], sizeof(uint16_vec8_t));
	uint16_vec8_t *vpn = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[3], sizeof(uint16_vec8_t));

	uint32_t x;
	__DECL_ALIGNED(16) uint32_t offset[4];
	__DECL_ALIGNED(16) uint32_t beginaddr[4];
	uint32_t mask = src->addrmask;
	uint32_t offsetmask = src->addrmask2;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		offset[i] = src->voffset[i];
	}
	__UNLIKELY_IF(src->palette == NULL) {
		// Note: Workaround for "warning: loop not distributed: failed explicitly specified loop distribution [-Wpass-failed]" with CLANG. 20241030 K.O
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			scrntype_t _rr = ((i & 2) != 0) ? 0x7f : 0;
			scrntype_t _rg = ((i & 4) != 0) ? 0x7f : 0;
			scrntype_t _rb = ((i & 1) != 0) ? 0x7f : 0;
			palette[i] = RGBA_COLOR(_rr, _rg, _rb, 0xff);
		}
__DECL_VECTORIZED_LOOP
		for(int i = 8; i < 16; i++) {
			scrntype_t _rr = ((i & 2) != 0) ? 0xff : 0;
			scrntype_t _rg = ((i & 4) != 0) ? 0xff : 0;
			scrntype_t _rb = ((i & 1) != 0) ? 0xff : 0;
			palette[i] = RGBA_COLOR(_rr, _rg, _rb, 0xff);
		}
	} else {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			palette[i] = src->palette[i];
		}
	}
	uint8_t *bp = &(src->data[0][src->baseaddress[0]]);
	uint8_t *rp = &(src->data[1][src->baseaddress[1]]);
	uint8_t *gp = &(src->data[2][src->baseaddress[2]]);
	uint8_t *np = &(src->data[3][src->baseaddress[3]]);

	__DECL_ALIGNED(8) std::valarray<uint8_t> brgn(4);
	__DECL_ALIGNED(8) std::valarray<uint8_t> rmask((const uint8_t)0, 4);
	__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 4; ii++) {
			if(src->is_render[ii]) rmask[ii] = 0xff;
		}
	enum {
		_b = 0,
		_r,
		_g,
		_n
	};
	int shift = src->shift;
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> tmp_dd(8);
	x = src->begin_pos;

	uint32_t xn = x;
	if(dst2 == NULL) {
		for(uint32_t xx = 0; xx < src->render_width; xx++) {
			brgn[_b] =  bp[(offset[0] + xn) & mask];
			brgn[_r] =  rp[(offset[1] + xn) & mask];
			brgn[_g] =  gp[(offset[2] + xn) & mask];
			brgn[_n] =  gp[(offset[3] + xn) & mask];
			brgn &= rmask;

			__DECL_ALIGNED(16) std::valarray<uint16_t> vb(vpb[brgn[_b]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vr(vpr[brgn[_r]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vg(vpg[brgn[_g]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vn(vpn[brgn[_n]].w, 8);

			tmpd = vb;
			tmpd = tmpd | vr;
			tmpd = tmpd | vg;
			tmpd = tmpd | vn;
			__LIKELY_IF(shift >= 0) {
				tmpd = tmpd >> (uint16_t)shift;
			} else {
				tmpd = tmpd << (uint16_t)(-shift);
			}
			xn = (xn + 1) & offsetmask;

	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				tmp_dd[i] = palette[tmpd[i]];
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst[i] = tmp_dd[i];
			}
			dst += 8;
		}
	} else {
#if defined(_RGB555) || defined(_RGBA565)
		static const scrntype_t shift_factor = 2;
#else // 24bit
		static const scrntype_t shift_factor = 3;
#endif
		__DECL_ALIGNED(32) std::valarray<scrntype_t> sline(RGBA_COLOR(31, 31, 31, 255), 8);
		for(uint32_t xx = 0; xx < src->render_width; xx++) {
			brgn[_b] =  bp[(offset[0] + xn) & mask];
			brgn[_r] =  rp[(offset[1] + xn) & mask];
			brgn[_g] =  gp[(offset[2] + xn) & mask];
			brgn[_n] =  gp[(offset[3] + xn) & mask];
			brgn &= rmask;

			__DECL_ALIGNED(16) std::valarray<uint16_t> vb(vpb[brgn[_b]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vr(vpr[brgn[_r]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vg(vpg[brgn[_g]].w, 8);
			__DECL_ALIGNED(16) std::valarray<uint16_t> vn(vpn[brgn[_n]].w, 8);

			tmpd = vb;
			tmpd = tmpd | vr;
			tmpd = tmpd | vg;
			tmpd = tmpd | vn;
			__LIKELY_IF(shift >= 0) {
				tmpd = tmpd >> (uint16_t)shift;
			} else {
				tmpd = tmpd << (uint16_t)(-shift);
			}
			xn = (xn + 1) & offsetmask;

	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				tmp_dd[i] = palette[tmpd[i]];
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst[i] = tmp_dd[i];
			}
			dst += 8;

			if(scan_line) {
				tmp_dd = tmp_dd >> shift_factor;
				tmp_dd = tmp_dd & sline;
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst2[i] = tmp_dd[i];
			}
			dst2 += 8;
		}
	}
}

// src->palette Must be 2^planes entries.
static void Render2NColors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line, int planes)
{
	__UNLIKELY_IF(src == NULL) return;
	__UNLIKELY_IF(dst == NULL) return;
	__UNLIKELY_IF(src->palette == NULL) return;
	__UNLIKELY_IF(planes <= 0) return;
	__UNLIKELY_IF(planes >= 16) planes = 16;
//__DECL_VECTORIZED_LOOP
//	for(int i = 0; i < 3; i++) {
//		if(src->bit_trans_table[i] == NULL) return;
//		if(src->data[i] == NULL) return;
//	}
	std::valarray<scrntype_t> palette(src->palette, 8);

	uint16_vec8_t* vp[16];
	for(int i = 0; i < planes; i++) {
		vp[i] = (uint16_vec8_t*)___assume_aligned(src->bit_trans_table[i], sizeof(uint16_vec8_t));
	}

	uint32_t x;
	__DECL_ALIGNED(16) uint32_t offset[16];
	__DECL_ALIGNED(16) uint32_t beginaddr[16];
	uint32_t mask = src->addrmask;
	uint32_t offsetmask = src->addrmask2;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < planes; i++) {
		offset[i] = src->voffset[i];
	}
	__DECL_ALIGNED(16) uint8_t *pp[16];
	for(int i = 0; i < planes; i++) {
		pp[i] = &(src->data[i][src->baseaddress[i]]);
	}

	int shift = src->shift;
	__DECL_ALIGNED(16) std::valarray<bool> is_render(src->is_render, 16);
	__DECL_ALIGNED(16) std::valarray<uint8_t> rmask((const uint8_t)0, 16);

	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> tmp_dd(8);
	for(int i = 0; i < planes; i++) {
		if(is_render[i]) rmask[i] = 0xff;
	}

	x = src->begin_pos;
	if(dst2 == NULL) {
		uint32_t n = x;

		for(uint32_t xx = 0; xx < src->render_width; xx++) {
			__DECL_ALIGNED(16) std::valarray<uint8_t> d((const uint8_t)0, 16);

			for(int i = 0; i < planes; i++) {
				d[i] = pp[i][(offset[i] + n) & mask];
			}
			d &= rmask;
			tmpd = 0;

			for(int i = 0; i < planes; i++) {
				__DECL_ALIGNED(16) std::valarray<uint16_t> tmpr(vp[i][d[i]].w, 8);
				tmpd |= tmpr;
			}

			n = (n + 1) & offsetmask;
			__LIKELY_IF(shift >= 0) {
				tmpd = tmpd >> (uint16_t)shift;
			} else {
				tmpd = tmpd << (uint16_t)(-shift);
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				tmp_dd[i] = palette[tmpd[i]];
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst[i] = tmp_dd[i];
			}
			dst += 8;
		}
	} else {
#if defined(_RGB555) || defined(_RGBA565)
		static const scrntype_t shift_factor = 2;
#else // 24bit
		static const scrntype_t shift_factor = 3;
#endif
		__DECL_ALIGNED(32) std::valarray<scrntype_t> sline(8);
		sline = (scrntype_t)RGBA_COLOR(31, 31, 31, 255);

		uint32_t n = x;

		for(uint32_t xx = 0; xx < src->render_width; xx++) {
			__DECL_ALIGNED(16) std::valarray<uint8_t> d((const uint8_t)0, 16);

			for(int i = 0; i < planes; i++) {
				d[i] = pp[i][(offset[i] + n) & mask];
			}
			d &= rmask;
			tmpd = 0;

			for(int i = 0; i < planes; i++) {
				__DECL_ALIGNED(16) std::valarray<uint16_t> tmpr(vp[i][d[i]].w, 8);
				tmpd |= tmpr;
			}
			n = (n + 1) & offsetmask;
			__LIKELY_IF(shift >= 0) {
				tmpd = tmpd >> (uint16_t)shift;
			} else {
				tmpd = tmpd << (uint16_t)(-shift);
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				tmp_dd[i] = palette[tmpd[i]];
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst[i] = tmp_dd[i];
			}
			dst += 8;
			if(scan_line) {
				tmp_dd = tmp_dd >> shift_factor;
				tmp_dd = tmp_dd & sline;
			}
	__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				dst2[i] = tmp_dd[i];
			}
			dst2 += 8;
		}
	}
}

static void Convert2NColorsToByte_Line(_render_command_data_t *src, uint8_t *dst, int planes)
{
	__UNLIKELY_IF(planes >= 8) planes = 8;
	__UNLIKELY_IF(planes <= 0) return;

	__DECL_ALIGNED(32) uint8_t* srcp[8];
	__DECL_ALIGNED(32) uint32_t offset[8] = {0};
	uint16_vec8_t* bp[8] ;


	for(int i = 0; i < planes; i++) {
		bp[i] = (uint16_vec8_t*)___assume_aligned(&(src->bit_trans_table[i]->plane_table[0]), sizeof(uint16_vec8_t));
		srcp[i] = &(src->data[i][src->baseaddress[i]]);
	}
	uint32_t addrmask = src->addrmask;
	uint32_t offsetmask = src->addrmask2;
	int shift = src->shift;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < planes; i++) {
		offset[i] = src->voffset[i];
	}

	uint32_t noffset = src->begin_pos & offsetmask;

	for(int x = 0; x < src->render_width; x++) {
		__DECL_ALIGNED(16) std::valarray<uint8_t> td((const uint8_t)0, 16);

		for(int i = 0; i < planes; i++) {
			td[i] = srcp[i][(noffset + offset[i]) & addrmask];
		}
		noffset = (noffset + 1) & offsetmask;
		__DECL_ALIGNED(16) std::valarray<uint16_t> dat((const uint16_t)0, 8);
		for(int i = 0; i < planes; i++) {
			__DECL_ALIGNED(16) std::valarray<uint16_t> _bd(bp[i][td[i]].w, 8);
			dat |= _bd;
		}
		__LIKELY_IF(shift >= 0) {
			dat = dat >> (uint16_t)shift;
		} else {
			dat = dat << (uint16_t)(-shift);
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			dst[i] = (uint8_t)(dat[i]);
		}
		dst += 8;

	}
}

static void Convert2NColorsToByte_LineZoom2(_render_command_data_t *src, uint8_t *dst, int planes)
{
	__UNLIKELY_IF(planes >= 8) planes = 8;
	__UNLIKELY_IF(planes <= 0) return;

	uint8_t* srcp[8];
	__DECL_ALIGNED(32) uint32_t offset[8] = {0};
	uint16_vec8_t* bp[8] ;


	for(int i = 0; i < planes; i++) {
		bp[i] = (uint16_vec8_t*)___assume_aligned(&(src->bit_trans_table[i]->plane_table[0]), sizeof(uint16_vec8_t));
		srcp[i] = &(src->data[i][src->baseaddress[i]]);
	}
	uint32_t addrmask = src->addrmask;
	uint32_t offsetmask = src->addrmask2;
	int shift = src->shift;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < planes; i++) {
		offset[i] = src->voffset[i];
	}

	uint32_t noffset = src->begin_pos & offsetmask;

	for(int x = 0; x < src->render_width; x++) {

		__DECL_ALIGNED(16) std::valarray<uint8_t> td((const uint8_t)0, 16);
		for(int i = 0; i < planes; i++) {
			td[i] = srcp[i][(noffset + offset[i]) & addrmask];
		}
		noffset = (noffset + 1) & offsetmask;
		__DECL_ALIGNED(16) std::valarray<uint16_t> dat((const uint16_t)0, 8);
		for(int i = 0; i < planes; i++) {
			__DECL_ALIGNED(16) std::valarray<uint16_t> _bd(bp[i][td[i]].w, 8);
			dat |= _bd;
		}
		noffset = (noffset + 1) & offsetmask;
		__LIKELY_IF(shift >= 0) {
			dat = dat >> (uint16_t)shift;
		} else {
			dat = dat << (uint16_t)(-shift);
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0, j = 0; i < 16; i +=2, j++) {
			dst[i]     = (uint8_t)(dat[j]);
			dst[i + 1] = (uint8_t)(dat[j]);
		}
		dst += 16;
	}
}

static void Convert8ColorsToByte_Line(_render_command_data_t *src, uint8_t *dst)
{
	uint8_t *bp = &(src->data[0][src->baseaddress[0]]);
	uint8_t *rp = &(src->data[1][src->baseaddress[1]]);
	uint8_t *gp = &(src->data[2][src->baseaddress[2]]);
	__DECL_ALIGNED(16) uint32_t offset[4] = {0};

	__DECL_ALIGNED(16) std::valarray<uint16_t> rdat(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> gdat(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> bdat(8);
	__DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);

	uint16_vec8_t* bpb = (uint16_vec8_t*)___assume_aligned(&(src->bit_trans_table[0]->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t* bpr = (uint16_vec8_t*)___assume_aligned(&(src->bit_trans_table[1]->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t* bpg = (uint16_vec8_t*)___assume_aligned(&(src->bit_trans_table[2]->plane_table[0]), sizeof(uint16_vec8_t));

	uint32_t addrmask = src->addrmask;
	uint32_t offsetmask = src->addrmask2;
	int shift = src->shift;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 3; i++) {
		offset[i] = src->voffset[i];
	}

	uint32_t noffset = src->begin_pos & offsetmask;
	uint8_t b, r, g;

	for(int x = 0; x < src->render_width; x++) {
		b = bp[(noffset + offset[0]) & addrmask];
		r = rp[(noffset + offset[1]) & addrmask];
		g = gp[(noffset + offset[2]) & addrmask];

		noffset = (noffset + 1) & offsetmask;
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			bdat[j] = bpb[b].w[j];
			rdat[j] = bpr[r].w[j];
			gdat[j] = bpg[g].w[j];
		}

		tmpd = bdat;
		tmpd = tmpd | rdat;
		tmpd = tmpd | gdat;
		__LIKELY_IF(shift >= 0) {
			tmpd = tmpd >> (uint16_t)shift;
		} else {
			tmpd = tmpd << (uint16_t)(-shift);
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			dst[i] = (uint8_t)(tmpd[i]);
		}
		dst += 8;
	}
}


// Convert uint8_t[] ed VRAM to uint16_t[] mono pixel pattern.
// You must set table to "ON_VALUE" : "OFF_VALUE" via PrepareBitTransTableUint16().
// -- 20181105 K.O
static void ConvertByteToSparceUint16(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask)
{

    __DECL_ALIGNED(32) std::valarray<uint16_t> tmpd(8);
    __DECL_ALIGNED(16) std::valarray<uint16_t> __masks(8);
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	__masks = mask;

	for(int i = 0; i < bytes; i++) {
	__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			tmpd[j] = vt[src[i]].w[j];
		}
		tmpd = tmpd & __masks;

__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			dst[j] = tmpd[j];
		}
		dst += 8;
	}
}

// Convert uint8_t[] ed VRAM to uint8_t[] mono pixel pattern.
// You must set table to "ON_VALUE" : "OFF_VALUE" via PrepareBitTransTableUint16().
// -- 20181105 K.O
static void ConvertByteToSparceUint8(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask)
{

    __DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
    __DECL_ALIGNED(16) std::valarray<uint16_t> __masks(8);
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	__masks = mask;
	// Sorry, not aligned.

	for(int i = 0; i < bytes; i++) {
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			tmpd[j] = vt[src[i]].w[j];
		}
		tmpd = tmpd & __masks;
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			dst[j] = (uint8_t)(tmpd[j]);
		}
		dst += 8;
	}
}


static void ConvertByteToPackedPixelByColorTable(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table)
{

    __DECL_ALIGNED(16) std::valarray<uint16_t> tmpd(8);
    __DECL_ALIGNED(32) std::valarray<scrntype_t> tmpdd(8);
    __DECL_ALIGNED(16) std::valarray<bool> tmpdet(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> on_tbl(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> off_tbl(8);
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	// Sorry, not aligned.

	for(int i = 0; i < bytes; i++) {
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			tmpd[j] = vt[src[i]].w[j];
		}
		tmpdet = (tmpd == (uint16_t)0);

__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			on_tbl[j] = on_color_table[j];
			off_tbl[j] = off_color_table[j];
		}
//		for(int j = 0; j < 8; j++) {
//			tmpdd[j] = (tmpdet) ? off_color_table[j] : on_color_table[j];
//		}
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			tmpdd[j] = (tmpdet[j]) ? off_tbl[j] : on_tbl[j];
		}
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			dst[j] = tmpdd[j];
		}
		off_color_table += 8;
		on_color_table += 8;
		dst += 8;
	}
}
// Note: Pls. read Note(s) of common.cpp -- 20181105 K.Ohta.
// Tables for below functions must be aligned by 16 (_bit_trans_table_t) or 32(_bit_trans_table_scrn_t).
// With _bit_trans_table_scrn_t.
static void ConvertByteToPackedPixelByColorTable2(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_scrn_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table)
{

    __DECL_ALIGNED(32) std::valarray<scrntype_t> tmpd(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> tmpdd(8);
	__DECL_ALIGNED(32) std::valarray<scrntype_t> colors(8);
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	for(int i = 0; i < bytes; i++) {
		scrntype_t* vtp = (scrntype_t*)___assume_aligned(&(vt[src[i]]), sizeof(scrntype_vec8_t));
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			tmpd[j] = vtp[j];
		}
		tmpdd = ~tmpd;

__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			colors[j] = on_color_table[j];
		}
		tmpd = tmpd & colors;
__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			colors[j] = off_color_table[j];
		}
		tmpdd = tmpdd & colors;
		tmpd = (tmpd | tmpdd);

__DECL_VECTORIZED_LOOP
		for(int j = 0; j < 8; j++) {
			dst[j] = tmpd[j];
		}
		off_color_table += 8;
		on_color_table += 8;
		dst += 8;
	}
}

