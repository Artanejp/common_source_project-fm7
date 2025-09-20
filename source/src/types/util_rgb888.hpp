/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB888 and _RGBA8888 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB888) || defined(_RGBA8888)

template <typename _T>
	scrntype_t RGB_COLOR(_T r, _T g, _T b)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;

	rr &= 0xff;
	gg &= 0xff;
	bb &= 0xff;

	scrntype_t retval;
	#if defined(__LITTLE_ENDIAN__)
	retval = rr | (gg << 8) | (bb << 16) | (0xff << 24);
	#else
	//retval = (rr << 16) | (gg << 8) | bb | (0xff << 24);
	retval = (rr << 24) | (gg << 16) | (bb << 8) | 0xff;
	#endif
	return retval;
}

template <typename _T>
	scrntype_t RGBA_COLOR(_T r, _T g, _T b, _T a)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;
	scrntype_t aa = (scrntype_t)a;

	rr &= 0xff;
	gg &= 0xff;
	bb &= 0xff;
	aa &= 0xff;

	scrntype_t retval;
	#if defined(__LITTLE_ENDIAN__)
	retval = rr | (gg << 8) | (bb << 16) | (aa << 24);
	#else
	//retval = (rr << 16) | (gg << 8) | bb | (0xff << 24);
	retval = (rr << 24) | (gg << 16) | (bb << 8) | aa;
	#endif
	return retval;
}

template <>
	scrntype_t RGB_COLOR<uint8_t>(uint8_t r, uint8_t g, uint8_t b)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;
	scrntype_t retval;
	#if defined(__LITTLE_ENDIAN__)
	retval = rr | (gg << 8) | (bb << 16) | (0xff << 24);
	#else
	//retval = (rr << 16) | (gg << 8) | bb | (0xff << 24);
	retval = (rr << 24) | (gg << 16) | (bb << 8) | 0xff;
	#endif
	return retval;
}

template <>
	scrntype_t RGBA_COLOR<uint8_t>(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;
	scrntype_t aa = (scrntype_t)a;

	scrntype_t retval;
	#if defined(__LITTLE_ENDIAN__)
	retval = rr | (gg << 8) | (bb << 16) | (aa << 24);
	#else
	//retval = (rr << 16) | (gg << 8) | bb | (0xff << 24);
	retval = (rr << 24) | (gg << 16) | (bb << 8) | aa;
	#endif
	return retval;
}

template <typename _T>
	_T R_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (_T)(c & 0xff);
#else
	//return (_T)((c >> 16) & 0xff);
	return (_T)((c >> 24) & 0xff);
#endif
}

template <typename _T>
	_T G_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (_T)((c >> 8) & 0xff);
#else
	//return (_T)((c >> 8) & 0xff);
	return (_T)((c >> 16) & 0xff);
#endif
}

template <typename _T>
	_T B_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (_T)((c >> 16) & 0xff);
#else
	//return (_T)((c >> 0) & 0xff);
	return (_T)((c >> 8) & 0xff);
#endif
}

template <typename _T>
	_T A_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (_T)((c >> 24) & 0xff);
#else
	//return (_T)((c >> 24) & 0xff);
	return (_T)(c & 0xff);
#endif
}


inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	scrntype_t r, g, b;
	scrntype_t nn = (scrntype_t)n;
	#if defined(__LITTLE_ENDIAN__)
	r = (nn & 0x7c00) << (16 + 1);
	g = (nn & 0x03e0) << (8 + 4 + 2);
	b = (nn & 0x001f) << (8 + 3);
	return (r | g | b | 0x000000ff);
	#else
	scrntype_t g2;
	r = (nn & 0x007c) << (16 + 1 + 8);
	g = (nn & 0x0e00) << (8 + 2)
	g2= (nn & 0x0030) << (8 + 4 + 2);
	b = (nn & 0x1f00) << 3;
	return (r | g | g2 | b | 0x000000ff);
	#endif
}
#endif
