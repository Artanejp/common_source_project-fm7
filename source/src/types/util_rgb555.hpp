/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB555 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB555)
template <typename _T>
	scrntype_t RGB_COLOR(_T r, _T g, _T b)
{
	scrntype_t rr = (scrntype_t)r;
	scrntype_t gg = (scrntype_t)g;
	scrntype_t bb = (scrntype_t)b;

	rr &= 0x1f;
	gg &= 0x1f;
	bb &= 0x1f;
	scrntype_t retval;
	retval = (rr << 10) | (gg << 5) | bb;
	return retval;
}

template <typename _T>
	scrntype_t RGBA_COLOR(_T r, _T g, _T b, _T a)
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
	cc >>= 10;
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
	cc >>= 10;
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
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
	return (_T)0xff; //
}

inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	return n;
}
#endif
