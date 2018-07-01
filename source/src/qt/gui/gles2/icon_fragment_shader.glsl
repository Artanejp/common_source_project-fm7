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
out mediump vec4 opixel;
#else
varying  vec2 v_texcoord;
#endif
uniform  vec4 color;
uniform  vec3 chromakey;
uniform bool do_chromakey;
uniform sampler2D a_texture;
void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;
	float alpha;
	pixel_r_1 = texture2D(a_texture, v_texcoord);
	//alpha = pixel_r_1.a * color.a;

	pixel_r_1 = pixel_r_1 * color;
	pixel = pixel_r_1 * color; //;vec4(pixel_r_1.rgb, alpha);
#if __VERSION__ >= 300
	opixel = pixel;
#else
	gl_FragColor = pixel;
#endif
}

