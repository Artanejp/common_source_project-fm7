varying mediump vec2 v_texcoord;
uniform mediump vec4 color;
uniform mediump vec3 chromakey;
uniform bool do_chromakey;
uniform sampler2D a_texture;
void main ()
{
	mediump vec4 pixel_r_1;
	mediump vec4 pixel;
	mediump float alpha;
	pixel_r_1 = texture2D(a_texture, v_texcoord);
	//alpha = pixel_r_1.a * color.a;

	pixel_r_1 = pixel_r_1 * color;
	pixel = pixel_r_1 * color; //;vec4(pixel_r_1.rgb, alpha);
	gl_FragColor = pixel;

}

