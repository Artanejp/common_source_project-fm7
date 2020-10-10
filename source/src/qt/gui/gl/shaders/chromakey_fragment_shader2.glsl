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
//in mediump float luminance;
//in mediump float lum_offset;
out vec4 opixel;
#else
varying vec2 v_texcoord;
//varying mediump float luminance;
//varying mediump float lum_offset;
#endif

uniform mediump float luminance;
uniform mediump float lum_offset;

uniform vec4 color;
uniform vec3 chromakey;
uniform sampler2D a_texture;

void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;
	bvec3 _match;
	
#if (__VERSION__ >= 300)
	pixel_r_1 = texture(a_texture, v_texcoord);
#else
	pixel_r_1 = texture2D(a_texture, v_texcoord);
#endif
	 _match = equal(pixel_r_1.rgb, chromakey.rgb);
	if(all(_match)) {
		pixel = vec4(0.0, 0.0, 0.0, 0.0);
	} else {
#if (__VERSION__ < 400)
		pixel = vec4(pixel_r_1.rgb, 1.0) * vec4(luminance, luminance, luminance, 1.0) + vec4(lum_offset, lum_offset, lum_offset,0.0);
#else
		pixel = fma(vec4(pixel_r_1.rgb, 1.0),vec4(luminance, luminance, luminance, 1.0), vec4(lum_offset, lum_offset, lum_offset, 0.0));
#endif
	}
#if (__VERSION__ >= 300)
    opixel = pixel;
#else
    gl_FragColor = pixel;
#endif		 
}
