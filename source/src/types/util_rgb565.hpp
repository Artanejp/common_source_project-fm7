/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB565 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB565)

template <typename _T>
	scrntype_t RGB_COLOR(_T r, _T g, _T b)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;

	rr &= 0x1f;
	gg &= 0x3f;
	bb &= 0x1f;
	scrntype_t retval;
	retval = (rr << 11) | (gg << 5) | bb;
	return retval;
}

template <typename _T>
	const scrntype_t RGBA_COLOR(_T r, _T g, _T b, _T a)
{
	return RGB_COLOR(r, g, b);
}

template <typename _T>
	_T R_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(cc);
	#endif
	cc >>= 11;
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
	return (_T)cc;
}

template <typename _T>
	_T G_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(cc);
	#endif
	cc >>= 5;
	cc &= 0x3f;
	cc <<= 2;
	cc |= ((cc == 0) ? 0x00 : 0x03);
	return (_T)cc;
}

template <typename _T>
	_T B_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(cc);
	#endif
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
	return (_T)cc;
}

template <typename _T>
	_T A_OF_COLOR(scrntype_t c)
{
	return (_T)0xff; // Alpha = 255
}

inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if defined(__LITTLE_ENDIAN__)
	scrntype_t nn = (scrntype_t)n;
	#else
	scrntype_t nn = (scrntype_t)(swap_endian_u16(n));
	#endif
	scrntype_t r;
	r = nn & 0x7c00; // r
	r |= (nn & 0x03e0); // g
	r <<= 1;
	r |= (nn & 0x001f); // b
	return r;
}
#endif
