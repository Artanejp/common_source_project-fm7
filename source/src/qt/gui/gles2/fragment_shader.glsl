varying mediump vec2 v_texcoord;
uniform sampler2D a_texture;
uniform bool swap_byteorder;
void main ()
{
	if(swap_byteorder) {
		mediump vec4 pixel = texture2D(a_texture, v_texcoord);
		pixel.rgb = pixel.bgr;
		gl_FragColor = pixel;
	} else {
		gl_FragColor = texture2D (a_texture, v_texcoord);
	}
}

