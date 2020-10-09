//precision  mediump float;

in vec2 v_texcoord;
in mediump float luminance;
in mediump float lum_offset;

out vec4 opixel;

uniform vec4 color;
uniform vec3 chromakey;
uniform sampler2D a_texture;

void main ()
{
	vec4 pixel_r_1;
	vec4 pixel;
	bvec3 _match;
	
	pixel_r_1 = texture(a_texture, v_texcoord);

	_match = equal(pixel_r_1.rgb, chromakey.rgb);
	pixel = (all(_match) == true) ? vec4(0.0, 0.0, 0.0, 0.0) : vec4(pixel_r_1.rgb, 1.0);
    opixel = pixel;
}
