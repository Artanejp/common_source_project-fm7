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
in mediump vec2 v_texcoord;
out vec4 opixel;
#else
varying mediump vec2 v_texcoord;
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

#if __VERSION__ >= 300
	pixel_r_1 = texture(a_texture, v_texcoord);
#else
	pixel_r_1 = texture2D(a_texture, v_texcoord);
#endif	
	if(do_chromakey) {
		if(pixel_r_1.rgb != chromakey.rgb) {
			pixel_r_1 = pixel_r_1 * color;
			pixel = vec4(pixel_r_1.rgb, 1.0);
			gl_FragColor = pixel;
		} else {
			pixel = vec4(0.0);
			//discard;
		}
	} else {
		pixel_r_1 = pixel_r_1 * color;
		pixel = vec4(pixel_r_1.rgb, 1.0);
		gl_FragColor = pixel;
	}
}

