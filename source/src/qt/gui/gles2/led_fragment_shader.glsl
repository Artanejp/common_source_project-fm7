#ifdef HAS_FLOAT_TEXTURE
#ifdef HAS_HALF_FLOAT_TEXTURE
#extension GL_OES_texture_half_float : enable
#else
#extension GL_OES_texture_float : enable
#endif
#endif
#ifdef HAS_FRAGMENT_HIGH_PRECISION
#extension GL_OES_fragment_precision_high : enable
precision  highp float;
#else
precision  mediump float;
#endif
#if __VERSION__ >= 300
out mediump vec4 opixel;
#endif
uniform vec4 color;
void main ()
{
#if __VERSION__ >= 300
	opixel = color;
#else
	gl_FragColor = color;
#endif

}


