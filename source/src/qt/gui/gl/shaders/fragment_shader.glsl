// Fragmant shader for standard screen.
// Oct 15, 2020 Kyuma Ohta <whatisthis.sowhat@gmail.com>
// License: GPLv2
//
// In values:
// a_texture: source texture (sampler2D).
// v_texcoord: source texture coordinate (vec2(s, t)).
// color: Color multiply value (vec4(R, G, B, A)) .
// luminance: CRT luminance multiply factor(float).
// lum_offset: Luminance offset (float).
//
// Out value:
// o_pixel: Output pixel(vec4(R, G, B, A)).

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
in vec2 v_texcoord;
out vec4 opixel;
#else
varying vec2 v_texcoord;
#endif

uniform sampler2D a_texture;
uniform vec4 color;
uniform mediump float luminance;
uniform mediump float lum_offset;

void main ()
{
	vec2 dist = v_texcoord;
	bvec4 d;
#if (__VERSION__ >= 300)
	vec4 pixel	= texture(a_texture, dist);
#else
	vec4 pixel	= texture2D(a_texture, dist);
#endif
	{
#if (__VERSION__ < 400)
		pixel = vec4(pixel.rgb, 1.0) * vec4(luminance, luminance, luminance, 1.0) + vec4(lum_offset, lum_offset, lum_offset, 0.0);
#else
		pixel = fma(vec4(pixel.rgb, 1.0), vec4(luminance, luminance, luminance, 1.0), vec4(lum_offset, lum_offset, lum_offset, 0.0));
#endif
	}
#if (__VERSION__ >= 300)
	opixel = pixel * color;
#else
	gl_FragColor = pixel * color;
#endif	
}
