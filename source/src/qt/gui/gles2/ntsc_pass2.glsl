// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014

#ifdef HAS_FRAGMENT_HIGH_PRECISION
precision  highp float;
#else
precision  mediump float;
#endif
varying mediump vec2 v_texcoord;

uniform bool swap_byteorder;
uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;

#define TAPS 24
uniform float luma_filter[24 + 1];
uniform float chroma_filter[24 + 1];

#define GAMMA_CORRECTION //comment to disable gamma correction, usually because higan's gamma correction is enabled or you have another shader already doing it
#ifndef HAS_FLOAT_TEXTURE
#define CRT_GAMMA 3.3
#else
#define CRT_GAMMA 2.5
#endif
#define DISPLAY_GAMMA 2.1


// #include ntsc-rgbyuv.inc //
mat3 yiq2rgb_mat = mat3(
   1.0, 1.0, 1.0,
   0.956, -0.2720, -1.1060,
   0.6210, -0.0, 1.7046
);

vec3 yiq2rgb(vec3 yiq)
{
	return (yiq * yiq2rgb_mat);
}

mat3 ycbcr_mat = mat3(
      0.29891, -0.16874,  0.50000,
      0.58661, -0.33126, -0.41869,
      0.11448,  0.50000, -0.08131
);
vec3 rgb2ycbcr(vec3 col)
{
	vec3 ycbcr = col * ycbcr_mat;
   return ycbcr;
}

mat3 ycbcr2rgb_mat = mat3(
	 1.0, 1.0, 1.0,
	 0.0, -0.34414 , 1.77200,
	 1.40200, -0.71414, 0.0
 );
 
vec3 ycbcr2rgb(vec3 ycbcr)
{
	//vec3 ra = ycbcr * vec3(1.0, 0.7, 1.0);
	return (ycbcr * ycbcr2rgb_mat);
}

mat3 yiq_mat = mat3(
      0.2989, 0.5959, 0.2115,
      0.5870, -0.2744, -0.5229,
      0.1140, -0.3216, 0.3114
);

vec3 rgb2yiq(vec3 col)
{
   return (col * yiq_mat);
}
// END ntsc-rgbyuv.inc //

// fixCoord moved from vertex
#define fixCoord (v_texcoord - vec2(0.5 / source_size.x, 0.0)) // Compensate for decimate-by-2.

#define fetch_offset(offset, one_x) \
	texture2D(a_texture, fixCoord + vec2((offset) * (one_x), 0.0)).xyz


void main() {
// #include "ntsc-pass2-decode.inc" //
	float one_x = 1.0 / source_size.x;
	vec3 signal = vec3(0.0);
	int i,j;
	int ibegin = 1;
#if 0
	for (int ii = 1; ii < TAPS; ii++)
	{
		float offset = float(ii);
		vec3 sums = fetch_offset(offset - float(TAPS), one_x) +
				fetch_offset(float(TAPS) - offset, one_x);
		sums = sums * vec3(3.6, 1.7, 1.7);
		signal += sums * vec3(luma_filter[ii], chroma_filter[ii], chroma_filter[ii]);
	}
#else
	float pos_offset = float(TAPS - ibegin) * one_x;
	vec3 sums_p = vec3(0.0, 0.0, 0.0);
	//vec3 sums_n[TAPS];
	vec2 fix_coord = v_texcoord - vec2(0.5 / source_size.x, 0.0);
	vec2 delta = vec2(one_x, 0);
	vec3 pix_p, pix_n;
	vec3 tmpv;
	vec2 addr_p = fix_coord + vec2(pos_offset, 0);
	vec2 addr_n = fix_coord - vec2(pos_offset, 0);

	for(int ii = 1; ii < TAPS; ii++) {
		pix_p = texture2D(a_texture, addr_p).xyz;
		pix_n = texture2D(a_texture, addr_n).xyz;
#ifndef HAS_FLOAT_TEXTURE
		pix_p = pix_p * vec3(3.6, 1.7, 1.7);
		pix_n = pix_n * vec3(3.6, 1.7, 1.7);
#endif
		pix_p = (pix_n + pix_p) * vec3(luma_filter[ii], chroma_filter[ii], chroma_filter[ii]);
		signal = signal + pix_p;
		addr_p = addr_p - delta;
		addr_n = addr_n + delta;
	}
#endif
	vec3 texvar = texture2D(a_texture, fixCoord).xyz;
	// yMax = (0.299+0.587+0.114) * (+-1.0) * (BRIGHTNESS + ARTIFACTING + ARTIFACTING) * (+-1.0)
	// CbMax = (-0.168736 -0.331264 + 0.5) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// CrMax = (0.5 - 0.418688 - 0.081312) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// -> y =  0 to +3.6
	//    Cb = 0 to +1.7
	//    Cr = 0 to +1.7
#ifndef HAS_FLOAT_TEXTURE
	texvar = texvar * vec3(3.6, 1.7, 1.7);
#endif
	signal +=  texvar * vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);
// END "ntsc-pass2-decode.inc" //

	vec3 rgb = ycbcr2rgb(signal);
#ifndef HAS_FLOAT_TEXTURE
	rgb = rgb * vec3(0.67, 1.0, 1.0);
#endif

#ifdef GAMMA_CORRECTION
   vec3 gamma = vec3(CRT_GAMMA / DISPLAY_GAMMA);
   rgb = pow(abs(rgb), gamma.rgb);
#endif
	vec4 pixel = vec4(rgb, 1.0);
	if(swap_byteorder) {
		pixel.rgba = pixel.bgra;
		gl_FragColor = pixel;
	} else {
		gl_FragColor = pixel;
	}
}