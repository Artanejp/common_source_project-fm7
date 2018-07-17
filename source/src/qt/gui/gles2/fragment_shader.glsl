#ifdef HAS_FLOAT_TEXTURE
#ifdef HAS_HALF_FLOAT_TEXTURE
#extension GL_OES_texture_half_float : enable
#else
#extension GL_OES_texture_float : enable
#endif
#endif

precision  mediump float;

#if __VERSION__ >= 300
in vec2 v_texcoord;
out vec4 opixel;
#else
varying mediump vec2 v_texcoord;
#endif

#if __VERSION__ >= 300
uniform sampler2D a_texture;
#else
uniform sampler2D a_texture;
#endif

void main ()
{
#if __VERSION__ >= 300
	vec4 pixel = texture(a_texture, v_texcoord);
#else
	vec4 pixel = texture2D(a_texture, v_texcoord);
#endif	   
//#ifdef HOST_ENDIAN_IS_LITTLE
//	pixel.rgb = pixel.bgr;
//#endif
#if __VERSION__ >= 300
	opixel = pixel;
#else
	gl_FragColor = pixel;
#endif
}
