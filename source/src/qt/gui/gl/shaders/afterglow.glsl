// Afterglow Shader - written by Kyuma Ohta <whatisthis.sowhat@gmail.com>
// License: GPLv2

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

#define PHASES 6
uniform sampler2D newest_texture;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
//uniform sampler2D texture6;
//uniform sampler2D texture7;

uniform vec2 source_size;
uniform vec2 target_size;
uniform float phase;

#if (__VERSION__ >= 300)
in mediump vec2 v_texcoord;
out mediump vec4 opixel;
#else // ES2.x or Compatible profile
varying mediump vec2 v_texcoord;
#endif

#if (__VERSION__ >= 300)
	_CONST float pixel_factor[PHASES] = float[PHASES](
		   0.8,
		   0.11,
		   0.055,
		   0.025,
		   0.008,
		   0.002
	   );
#else
	float pixel_factor[PHASES];
#endif

void main()
{
	vec4 texel[PHASES];
	vec4 ratio[PHASES];
	for(int i=0; i < PHASES; i++) {
		ratio[i] = vec(pixel_factor[i], pixel_factor[i], pixel_factor[i], 1.0);
	}
	texel[0] = texture(newest_texture, v_texcoord);
	texel[1] = texture(texture1, v_texcoord);
	texel[2] = texture(texture2, v_texcoord);
	texel[3] = texture(texture3, v_texcoord);
	texel[4] = texture(texture4, v_texcoord);
	texel[5] = texture(texture5, v_texcoord);

	vec4 pixel = vec4(0.0);
	for(int i = 0; i < PHASES; i++) {
		pixel += (texel[i] * ratio[i]);
	}
#if (__VERSION__ >= 300)
	opixel = pixel;
#else
	gl_FragColor = pixel;
#endif
}	