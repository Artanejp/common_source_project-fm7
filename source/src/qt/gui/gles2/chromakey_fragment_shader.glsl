varying mediump vec2 v_texcoord;
uniform mediump vec4 color;
uniform mediump vec3 chromakey;
uniform bool do_chromakey;
uniform bool swap_byteorder;
uniform sampler2D a_texture;
void main ()
{
	mediump vec4 pixel_r_1;
	mediump vec4 pixel;

	pixel_r_1 = texture2D(a_texture, v_texcoord);
	
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

