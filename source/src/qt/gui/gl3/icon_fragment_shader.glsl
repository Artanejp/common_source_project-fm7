varying vec2 v_texcoord;
uniform vec4 color;
uniform vec3 chromakey;
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
	gl_FragColor = pixel;

}

