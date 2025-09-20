/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB555 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB555)

constexpr scrntype_t __RGB_COLOR(scrntype_t r, scrntype_t  g, scrntype_t b)
{
	scrntype_t rr = r;
	scrntype_t gg = g;
	scrntype_t bb = b;

	rr &= 0x1f;
	gg &= 0x1f;
	bb &= 0x1f;
	scrntype_t retval;
	retval = (rr << 10) | (gg << 5) | bb;
	#if !defined(__LITTLE_ENDIAN__)
	retval = swap_endian_u16(retval);
	#endif
	return retval;
}

constexpr scrntype_t __RGBA_COLOR(scrntype_t r, scrntype_t  g, scrntype_t b, scrntype_t a)
{
	return __RGB_COLOR(r, g, b);
}

constexpr uint8_t R_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(c);
	#endif
	cc >>= 10;
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
	return (uint8_t)cc;
}

constexpr uint8_t G_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(c);
	#endif
	cc >>= 10;
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
	return (uint8_t)cc;
}

constexpr uint8_t B_OF_COLOR(scrntype_t c)
{
	#if defined(__LITTLE_ENDIAN__)
    scrntype_t cc = c;
	#else
	scrntype_t cc = swap_endian_u16(c);
	#endif
	cc &= 0x1f;
	cc <<= 3;
	cc |= ((cc == 0) ? 0x00 : 0x07);
	return (uint8_t)cc;
}

constexpr uint8_t A_OF_COLOR(scrntype_t c)
{
	return (uint8_t)0xff; //
}

constexpr scrntype_t rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	return n;
}

constexpr scrntype_t rgb565le_to_scrntype_t(uint16_t n)
{
	#if defined(__LITTLE_ENDIAN__)
	scrntype_t nn = (scrntype_t)n;
	#else
	scrntype_t nn = (scrntype_t)(swap_endian_u16(n));
	#endif
	scrntype_t r, g, b;
	r = nn & 0xf800; // r
	g = nn & 0x07e0; // g
	b = nn & 0x001f;
	r >>= 1;
	g >>= 1;
	g &= 0x03e0;
	return (r | g | b);
}

#endif
