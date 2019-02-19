//precision  mediump float;

in mediump vec2 v_texcoord;
out vec4 opixel;

uniform vec4 color;
uniform vec3 chromakey;
uniform bool do_chromakey;
uniform bool swap_byteorder;
uniform sampler2D a_texture;
void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;

	pixel_r_1 = texture(a_texture, v_texcoord);
	
	if(do_chromakey) {
		if(pixel_r_1.rgb != chromakey.rgb) {
			pixel_r_1 = pixel_r_1 * color;
			pixel = vec4(pixel_r_1.rgb, 1.0);
			opixel = pixel;
		} else {
			pixel = vec4(0.0);
			//discard;
		}
	} else {
		pixel_r_1 = pixel_r_1 * color;
		pixel = vec4(pixel_r_1.rgb, 1.0);
		opixel = pixel;
	}
}

