// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014


in mediump vec2 v_texcoord;
out vec4 opixel;

uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;

#define TAPS 24

#if __VERSION__ >= 300
	// THREE_PHASE
const float luma_filter[24 + 1] = float[24 + 1](
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552
	);
const float chroma_filter[24 + 1] = float [24 + 1](
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396
	);
// END "ntsc-decode-filter-3phase.inc" //

#else
uniform float luma_filter[24 + 1];
uniform float chroma_filter[24 + 1];
#endif

#define GAMMA_CORRECTION //comment to disable gamma correction, usually because higan's gamma correction is enabled or you have another shader already doing it
#ifndef HAS_FLOAT_TEXTURE
#define CRT_GAMMA 3.3
#else
#define CRT_GAMMA 2.5
#endif
#define DISPLAY_GAMMA 2.1

#define GAMMA_FACTOR (CRT_GAMMA / DISPLAY_GAMMA)

// #include ntsc-rgbyuv.inc //
mat3 ycbcr2rgb_mat = mat3(
	 1.0, 1.0, 1.0,
	 0.0, -0.34414 , 1.77200,
	 1.40200, -0.71414, 0.0
 );
 
#define ycbcr2rgb(foo) (foo.rgb * ycbcr2rgb_mat)

// END ntsc-rgbyuv.inc //

// fixCoord moved from vertex
#define fixCoord (v_texcoord - (vec2(0.5) * delta)) // Compensate for decimate-by-2.

void main() {
// #include "ntsc-pass2-decode.inc" //
	float one_x = 1.0 / source_size.x;
	vec2 delta = vec2(one_x, 0);
	vec4 signal = vec4(0.0);
	vec2 pos_offset = vec2(float(TAPS - 1.0) * one_x, 0.0);
	vec2 fix_coord = v_texcoord - vec2(0.5 * one_x, 0.0);
	vec4 pix_p, pix_n;
	vec2 addr_p = fix_coord + pos_offset;
	vec2 addr_n = fix_coord - pos_offset;
	for(int ii = 1; ii < TAPS; ii++) {
		pix_p = texture2D(a_texture, addr_p - delta * vec2(ii - 1, 0));
		pix_n = texture2D(a_texture, addr_n + delta * vec2(ii - 1, 0));
		signal = signal +
				(pix_n + pix_p) * vec4(luma_filter[ii], chroma_filter[ii], chroma_filter[ii], 0);
		//addr_p -= delta;
		//addr_n += delta;
	}
	vec4 texvar = texture2D(a_texture, fix_coord);
	// yMax = (0.299+0.587+0.114) * (+-1.0) * (BRIGHTNESS + ARTIFACTING + ARTIFACTING) * (+-1.0)
	// CbMax = (-0.168736 -0.331264 + 0.5) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// CrMax = (0.5 - 0.418688 - 0.081312) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// -> y =  0 to +3.6
	//    Cb = 0 to +1.7
	//    Cr = 0 to +1.7
#ifndef HAS_FLOAT_TEXTURE
//	texvar = texvar * vec3(3.6, 1.7, 1.7);
#endif
	signal +=  texvar * vec4(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS], 0);
// END "ntsc-pass2-decode.inc" //

	vec3 rgb = ycbcr2rgb(signal.xyz);
#ifndef HAS_FLOAT_TEXTURE
//	rgb = rgb * vec3(0.67, 1.0, 1.0);
#endif

#ifdef GAMMA_CORRECTION
   const vec3 gamma = vec3(GAMMA_FACTOR);
   rgb = pow(abs(rgb), gamma.rgb);
#endif
	vec4 pixel = vec4(rgb, 1.0);

	opixel = pixel;
}