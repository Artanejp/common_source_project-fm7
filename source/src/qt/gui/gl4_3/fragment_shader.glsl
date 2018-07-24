//precision mediump float;

in vec2 v_texcoord;
out vec4 opixel;

uniform sampler2D a_texture;
void main ()
{
	vec4 pixel = texture(a_texture, v_texcoord);
	opixel = pixel;
}
