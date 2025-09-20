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


template <typename _TR, typename _TG, typename _TB>
	constexpr scrntype_t RGB_COLOR(_TR r, _TG g, _TB b)
{
	return __RGB_COLOR((scrntype_t)r, (scrntype_t)g, (scrntype_t)b); 
}

template <typename _TR, typename _TG, typename _TB, typename _TA>
	constexpr scrntype_t RGBA_COLOR(_TR r, _TG g, _TB b, _TA a)
{
	return __RGBA_COLOR((scrntype_t)r, (scrntype_t)g, (scrntype_t)b, (scrntype_t)a); 
}

#if defined(_RGB888) || defined(_RGBA8888)
template <>
	constexpr scrntype_t RGB_COLOR<uint8_t, uint8_t, uint8_t>(uint8_t r, uint8_t g, uint8_t b)
{
	return __RGB_COLOR(r, g, b);
}

template <>
	constexpr scrntype_t RGBA_COLOR<uint8_t, uint8_t, uint8_t, uint8_t>(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return __RGBA_COLOR(r, g, b, a);
}
#endif

// Make BLANK, not TRANSPARENT.
constexpr scrntype_t msb_to_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT BLANK
	//        '1' = BLANK
	scrntype_t _n;
	#if defined(__LITTLE_ENDIAN__)
	_n = ((n & 0x8000) != 0) ? RGBA_COLOR(0, 0, 0, 255) : RGBA_COLOR(255, 255, 255, 255);
	#else
	_n = ((n & 0x0080) != 0) ? RGBA_COLOR(0, 0, 0, 255) : RGBA_COLOR(255, 255, 255, 255);
	#endif
	return _n;
}

// ALL TRANSPARENT MASK.
constexpr scrntype_t msb_to_transparent_mask_u16le(uint16_t n)
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

// Not TRANSPARENT, but makes ALPHA MASK.
constexpr scrntype_t msb_to_alpha_mask_u16le(uint16_t n)
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


