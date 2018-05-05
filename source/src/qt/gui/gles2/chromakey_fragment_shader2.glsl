varying mediump vec2 v_texcoord;
uniform mediump vec4 color;
uniform mediump vec3 chromakey;
uniform bool do_chromakey;
uniform sampler2D a_texture;
void main ()
{
	mediump vec4 pixel_r_1;
	mediump vec4 pixel;
	
	pixel_r_1 = texture2D(a_texture, v_texcoord);
	
	if(pixel_r_1.rgb != chromakey.rgb) {
		pixel_r_1 = pixel_r_1 * color;
		pixel = vec4(pixel_r_1.rgb, 1.0);
		gl_FragColor = pixel;
	} else {
	    discard;
	}
}

