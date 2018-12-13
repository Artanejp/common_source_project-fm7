//precision mediump float;

in vec2 v_texcoord;
out uvec4 opixel;
//uniform usamplerBuffer s_texture;
uniform usampler2D a_texture;
void main ()
{
	opixel = texture(a_texture, v_texcoord);
}
