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
out vec4 opixel;
#else
varying vec2 v_texcoord;
#endif

uniform vec4 color;
uniform vec3 chromakey;
uniform bool do_chromakey;
uniform bool swap_byteorder;
uniform sampler2D a_texture;
void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;

#if (__VERSION__ >= 300)
	pixel_r_1 = texture(a_texture, v_texcoord);
#else
	pixel_r_1 = texture2D(a_texture, v_texcoord);
#endif
	if(do_chromakey) {
		if(pixel_r_1.rgb != chromakey.rgb) {
			pixel_r_1 = pixel_r_1 * color;
			pixel = vec4(pixel_r_1.rgb, 1.0);
#if (__VERSION__ >= 300)
			opixel = pixel;
#else
			gl_FragColor = pixel;
#endif
		} else {
			pixel = vec4(0.0);
			//discard;
		}
	} else {
		pixel_r_1 = pixel_r_1 * color;
		pixel = vec4(pixel_r_1.rgb, 1.0);
#if (__VERSION__ >= 300)
		opixel = pixel;
#else
		gl_FragColor = pixel;
#endif
	}
}

