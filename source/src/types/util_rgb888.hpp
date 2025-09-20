/*
 * Utilities: Around RGB_COLOR() and RGBA_COLOR() for _RGB888 and _RGBA8888 .
 *
 * Author: Kyuma Otha <whatisthis.sowhat@gmail.com>
 * History:
 *          2025-09-20: Split from common.cpp / common.h / util_video.h .
 */
#pragma once

#if defined(_RGB888) || defined(_RGBA8888)

constexpr scrntype_t __RGB_COLOR(scrntype_t r, scrntype_t  g, scrntype_t b)
{
	scrntype_t rr = r;
	scrntype_t gg = g;
	scrntype_t bb = b;

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

constexpr scrntype_t __RGBA_COLOR(scrntype_t r, scrntype_t  g, scrntype_t b, scrntype_t a)
{
	scrntype_t rr = r;
	scrntype_t gg = g;
	scrntype_t bb = b;
	scrntype_t aa = a;

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

constexpr scrntype_t __RGB_COLOR(uint8_t r, uint8_t g, uint8_t b)
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

constexpr scrntype_t  __RGBA_COLOR(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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


constexpr uint8_t R_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (uint8_t)(c & 0xff);
#else
	//return (uint8_t)((c >> 16) & 0xff);
	return (uint8_t)((c >> 24) & 0xff);
#endif
}

constexpr uint8_t G_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (uint8_t)((c >> 8) & 0xff);
#else
	//return (uint8_t)((c >> 8) & 0xff);
	return (uint8_t)((c >> 16) & 0xff);
#endif
}

constexpr uint8_t B_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (uint8_t)((c >> 16) & 0xff);
#else
	//return (uint8_t )((c >> 0) & 0xff);
	return (uint8_t)((c >> 8) & 0xff);
#endif
}

constexpr uint8_t A_OF_COLOR(scrntype_t c)
{
#if defined(__LITTLE_ENDIAN__)
	return (uint8_t)((c >> 24) & 0xff);
#else
	//return (uint8_t)((c >> 24) & 0xff);
	return (uint8_t)(c & 0xff);
#endif
}


constexpr scrntype_t rgb555le_to_scrntype_t(uint16_t n)
{
	scrntype_t r, g, b;
	scrntype_t nn = (scrntype_t)n;
	#if defined(__LITTLE_ENDIAN__)
	r = nn & 0x7c00;
	g = nn & 0x03e0;
	b = nn & 0x001f;
	r <<= (16 + 1);
	g <<= (8 + 4 + 2);
	b <<= (8 + 3);
	r |= ((r == 0) ?  0x00000000 : 0x07000000);
	g |= ((g == 0) ?  0x00000000 : 0x00070000);
	b |= ((b == 0) ?  0x00000000 : 0x00000700);
	return (r | g | b | 0x000000ff);
	#else
	scrntype_t g2;
	r = nn & 0x007c;
	g = nn & 0x0e00;
	g2= nn & 0x0030;
	b = nn & 0x1f00;
	r  <<= (16 + 1 + 8);
	g  <<= (8 + 2);
	g2 <<= (8 + 4 + 2);
	b <<= 3;
	g |= g2;
	r |= ((r == 0) ?  0x00000000 : 0x07000000);
	g |= ((g == 0) ?  0x00000000 : 0x00070000);
	b |= ((b == 0) ?  0x00000000 : 0x00000700);
	return (r | g | b | 0x000000ff);
	#endif
}

constexpr scrntype_t rgb565le_to_scrntype_t(uint16_t n)
{
	scrntype_t r, g, b;
	scrntype_t nn = (scrntype_t)n;
	#if defined(__LITTLE_ENDIAN__)
	r = nn & 0xf800;
	g = nn & 0x07e0;
	b = nn & 0x001f;
	r <<= 16;
	g <<= (8 + 4 + 1);
	b <<= (8 + 3);
	r |= ((r == 0) ?  0x00000000 : 0x07000000);
	g |= ((g == 0) ?  0x00000000 : 0x00030000);
	b |= ((b == 0) ?  0x00000000 : 0x00000700);
	return (r | g | b | 0x000000ff);
	#else
	scrntype_t g2;
	r = nn & 0x00f8;
	g = nn & 0x0e00;
	g2= nn & 0x0070;
	b = nn & 0x1f00;
	g2 <<= 8;
	g |= g2;
	r <<= (16 + 8);
	g <<= (8 + 1);
	b <<= 3;
	r |= ((r == 0) ?  0x00000000 : 0x07000000);
	g |= ((g == 0) ?  0x00000000 : 0x00030000);
	b |= ((b == 0) ?  0x00000000 : 0x00000700);
	return (r | g | b | 0x000000ff);
	#endif
}
#endif
