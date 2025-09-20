/*!
  @
  * Utilities: RGB_COLOR() and similer one.
*/
#pragma once

#if defined(_RGB555)
	#include "util_rgb555.hpp"
#elif defined(_RGB565)
	#include "util_rgb565.hpp"
#elif defined(_RGB888) || defined(_RGBA8888)
	#include "util_rgb888.hpp"
#else
	#error "COMMON:VIDEO: Unsupported video format.\nSupportings are: _RGB555, _RGB565, _RGB888 and _RGBA8888 .")
#endif

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
