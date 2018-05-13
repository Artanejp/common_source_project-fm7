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

varying mediump vec2 v_texcoord;
uniform sampler2D a_texture;
uniform bool swap_byteorder;
void main ()
{
	if(swap_byteorder) {
		vec4 pixel = texture2D(a_texture, v_texcoord);
		pixel.rgb = pixel.bgr;
		gl_FragColor = pixel;
	} else {
		gl_FragColor = texture2D (a_texture, v_texcoord);
	}
}

