// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014

varying vec2 v_texcoord;

uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;

#define TAPS 24
uniform float luma_filter[TAPS + 1];
uniform float chroma_filter[TAPS + 1];

#define OLD_THREE_PHASE //options here include THREE_PHASE, TWO_PHASE or OLD_THREE_PHASE
#define GAMMA_CORRECTION //comment to disable gamma correction, usually because higan's gamma correction is enabled or you have another shader already doing it
#define CRT_GAMMA 2.5
#define DISPLAY_GAMMA 2.1


// #include ntsc-rgbyuv.inc //
mat3 yiq2rgb_mat = mat3(
   1.0, 1.0, 1.0,
   0.956, -0.2720, -1.1060,
   0.6210, -0.6474, 1.7046
);

vec3 yiq2rgb(vec3 yiq)
{
   return (yiq * yiq2rgb_mat);
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
	int i;
	for (i = 0; i < TAPS; i++)
	{
		float offset = float(i);
		vec3 sums = fetch_offset(offset - float(TAPS), one_x) +
		fetch_offset(float(TAPS) - offset, one_x);
	
		signal += sums * vec3(luma_filter[i], chroma_filter[i], chroma_filter[i]);
	}		

	signal += texture2D(a_texture, fixCoord).xyz * vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);

// END "ntsc-pass2-decode.inc" //

   vec3 rgb = yiq2rgb(signal);
   
#ifdef GAMMA_CORRECTION
   vec3 gamma = vec3(CRT_GAMMA / DISPLAY_GAMMA);
   rgb = pow(rgb, gamma.rgb);
#endif

	gl_FragColor = vec4(rgb, 1.0);
}