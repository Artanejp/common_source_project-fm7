// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014
#if defined(GL_ES)
	#define USE_GL_ES 1
#elif ((__VERSION__ >= 430) || defined(GL_es_profile))
	#if (GL_es_profile)
		#define USE_GL_ES 1
	#endif
#endif	
#if defined(USE_GL_ES)
	#ifdef HAS_FLOAT_TEXTURE
		#ifdef HAS_HALF_FLOAT_TEXTURE
		#extension GL_OES_texture_half_float : enable
		#else
		#extension GL_OES_texture_float : enable
		#endif
	#endif
	precision  mediump float;
#endif
#if (__VERSION__ >= 300)
#define _CONST const
#else
#define _CONST
#endif

#if (__VERSION__ >= 300)
in mediump vec2 v_texcoord;
out mediump vec4 opixel;
#else // ES2.x or Compatible profile
varying mediump vec2 v_texcoord;
#endif

uniform sampler2D a_texture;
uniform vec2 source_size;
uniform vec2 target_size;
uniform float phase;


#define THREE_PHASE //options here include THREE_PHASE, TWO_PHASE or OLD_THREE_PHASE
#define COMPOSITE

// #include "ntsc-param.inc" //
#define PI 3.14159265

#if defined(TWO_PHASE)
#define CHROMA_MOD_FREQ (4.0 * PI / 15.0)
#elif defined(THREE_PHASE)
#define CHROMA_MOD_FREQ (PI / 6.0)
#endif

#if defined(COMPOSITE)
#define SATURATION 1.2
#define BRIGHTNESS 1.1
#define ARTIFACTING 1.8
#define FRINGING 1.0
#elif defined(SVIDEO)
#define SATURATION 1.0
#define BRIGHTNESS 1.0
#define ARTIFACTING 0.0
#define FRINGING 0.0
#endif

#if defined(COMPOSITE) || defined(SVIDEO)
_CONST mat3 mix_mat = mat3(
	BRIGHTNESS, FRINGING, FRINGING,
	ARTIFACTING, 2.0 * SATURATION, 0.0,
	ARTIFACTING, 0.0, 2.0 * SATURATION
);
#endif
// END "ntsc-param.inc" //

// moved from vertex
#define pix_no (v_texcoord.xy * source_size.xy * (target_size.xy / source_size.xy))

// Change Matrix: [RGB]->[YCbCr]

_CONST mat3 ycbcr_mat = mat3(
      0.29891, -0.16874,  0.50000,
      0.58661, -0.33126, -0.41869,
      0.11448,  0.50000, -0.08131
);

vec3 rgb2ycbcr(vec3 foo)
{
	return (foo * ycbcr_mat);
}

// #include ntsc-rgbyuv.inc //
_CONST mat3 yiq2rgb_mat = mat3(
   1.0, 1.0, 1.0,
   0.956, -0.2720, -1.1060,
   0.6210, -0.6474, 1.7046
);

vec3 yiq2rgb(vec3 yiq)
{
   return (yiq * yiq2rgb_mat);
}

_CONST mat3 yiq_mat = mat3(
      0.2989, 0.5959, 0.2115,
      0.5870, -0.2744, -0.5229,
      0.1140, -0.3216, 0.3114
);

vec3 rgb2yiq(vec4 col)
{
   return (col.xyz * yiq_mat);
}
// END ntsc-rgbyuv.inc //

void main()
{
	// #include "ntsc-pass1-encode-demodulate.inc" //
	vec4 col;
#if (__VERSION__ >= 300)	
	col = texture(a_texture, v_texcoord);
#else
	col = texture2D(a_texture, v_texcoord);
#endif
	vec3 ycbcr;
	ycbcr = rgb2yiq(col);
#if defined(TWO_PHASE)
	float chroma_phase = PI * (mod(pix_no.y, 2.0) + phase);
#elif defined(THREE_PHASE)
	float chroma_phase = 0.6667 * PI * (mod(pix_no.y, 3.0) + phase);
#endif

	float mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ;

	float i_mod = cos(mod_phase);
	float q_mod = sin(mod_phase);

	ycbcr *= vec3(1.0, i_mod, q_mod); // Modulate
#if defined(COMPOSITE) || defined(SVIDEO)
	ycbcr *= mix_mat; // Cross-talk
#endif
	ycbcr *= vec3(1.0, i_mod, q_mod); // Demodulate
#ifndef HAS_FLOAT_TEXTURE
//	ycbcr = ycbcr * vec3(0.277778 ,0.588235, 0.588235);
#endif
#if (__VERSION__ >= 300)	
	opixel = vec4(ycbcr, 1.0);
#else
	gl_FragColor = vec4(ycbcr, 1.0);
#endif
// END "ntsc-pass1-encode-demodulate.inc" //
}