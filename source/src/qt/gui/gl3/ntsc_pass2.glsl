// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014

varying vec2 v_texcoord;

uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;

#define TAPS 24
uniform float luma_filter[24 + 1];
uniform float chroma_filter[24 + 1];

#define THREE_PHASE //options here include THREE_PHASE, TWO_PHASE or OLD_THREE_PHASE
#define GAMMA_CORRECTION //comment to disable gamma correction, usually because higan's gamma correction is enabled or you have another shader already doing it
#define CRT_GAMMA 2.5
#define DISPLAY_GAMMA 2.1


// #include ntsc-rgbyuv.inc //
mat3 ycbcr2rgb_mat = mat3(
	 1.0, 1.0, 1.0,
	 0.0, -0.34414 , 1.77200,
	 1.40200, -0.71414, 0.0
 );
#define ycbcr2rgb(ycbcr) (ycbcr * ycbcr2rgb_mat);

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
	float pos_offset = float(TAPS - 1) * one_x;
	vec3 sums_p = vec3(0.0, 0.0, 0.0);
	//vec3 sums_n[TAPS];
	vec2 fix_coord = v_texcoord - vec2(0.5 / source_size.x, 0.0);
	vec2 delta = vec2(one_x, 0);
	vec3 pix_p, pix_n;
	vec3 tmpv;
	vec2 addr_p = fix_coord + vec2(pos_offset, 0);
	vec2 addr_n = fix_coord - vec2(pos_offset, 0);
	for (i = 1 ; i < TAPS; i++) {
		pix_p = texture2D(a_texture, addr_p - (vec2(i - 1, 0) * delta)).xyz;
		pix_n = texture2D(a_texture, addr_n + (vec2(i - 1, 0) * delta)).xyz;
		signal += (pix_n + pix_p) * vec3(luma_filter[i], chroma_filter[i], chroma_filter[i]);
//		signal = signal + pix_p;
//		addr_p = addr_p - delta;
//		addr_n = addr_n + delta;
	}	   

	signal += texture2D(a_texture, fixCoord).xyz * vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);
// END "ntsc-pass2-decode.inc" //

   vec3 rgb = ycbcr2rgb(signal);
#ifdef GAMMA_CORRECTION
   vec3 gamma = vec3(CRT_GAMMA / DISPLAY_GAMMA);
   rgb = pow(rgb, gamma.rgb);
#endif
	gl_FragColor = vec4(rgb, 1.0);
}