/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB565 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB565)

constexpr scrntype_t  __RGB_COLOR(scrntype_t r, scrntype_t g, scrntype_t b)
{
	scrntype_t rr = r;
	scrntype_t gg = g;
	scrntype_t bb = b;

	rr &= 0x1f;
	gg &= 0x3f;
	bb &= 0x1f;
	scrntype_t retval;
	retval = (rr << 11) | (gg << 5) | bb;
	#if !defined(__LITTLE_ENDIAN__)
	retval = swap_endian_u16(retval);
	#endif
	return retval;
}

constexpr scrntype_t __RGBA_COLOR(scrntype_t r, scrntype_t g, scrntype_t b, scrntype_t a)
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
	cc >>= 11;
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
	cc >>= 5;
	cc &= 0x3f;
	cc <<= 2;
	cc |= ((cc == 0) ? 0x00 : 0x03);
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
	return (uint8_t)0xff; // Alpha = 255
}

constexpr scrntype_t rgb555le_to_scrntype_t(uint16_t n)
{
	#if defined(__LITTLE_ENDIAN__)
	scrntype_t nn = (scrntype_t)n;
	#else
	scrntype_t nn = (scrntype_t)(swap_endian_u16(n));
	#endif
	scrntype_t r, g, b;
	r = nn & 0x7c00; // r
	g = nn & 0x03e0; // g
	b = nn & 0x001f;
	r <<= 1;
	g <<= 1;
	g |= ((g == 0) ? 0x0000 : 0x0020);
	return (r | g | b);
}

constexpr scrntype_t rgb565le_to_scrntype_t(uint16_t n)
{
	#if defined(__LITTLE_ENDIAN__)
	scrntype_t nn = (scrntype_t)n;
	#else
	scrntype_t nn = (scrntype_t)(swap_endian_u16(n));
	#endif
	return nn;
}
#endif
