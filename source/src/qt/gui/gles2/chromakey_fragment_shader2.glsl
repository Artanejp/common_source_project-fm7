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
in vec2 v_texcoord;
out vec4 opixel;
#else
varying vec2 v_texcoord;
#endif

uniform vec4 color;
uniform vec3 chromakey;
uniform sampler2D a_texture;

void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;
	
	pixel_r_1 = texture2D(a_texture, v_texcoord);
#ifdef HOST_ENDIAN_IS_LITTLE
	pixel_r_1.rgb = pixel_r_1.bgr;
#endif
	bvec3 _match;
	_match = equal(pixel_r_1.rgb, chromakey.rgb);
	pixel = (all(_match) == true) ? vec4(0.0, 0.0, 0.0, 0.0) : vec4(pixel_r_1.rgb, 1.0);
#if __VERSION__ >= 300
    opixel = pixel;
#else
	gl_FragColor = pixel;
#endif
}

