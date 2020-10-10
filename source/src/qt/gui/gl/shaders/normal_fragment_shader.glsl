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
in vec2 v_texcoord;
out mediump vec4 opixel;
#else
varying vec2 v_texcoord;
#endif

uniform sampler2D a_texture;
uniform vec4 color;
void main ()
{
#if (__VERSION__ >= 300)
	opixel = texture(a_texture, v_texcoord) * color;
#else
	gl_FragColor = texture(a_texture, v_texcoord) * color;
#endif
}

