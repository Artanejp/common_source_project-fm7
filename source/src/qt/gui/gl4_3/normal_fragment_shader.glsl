in vec2 v_texcoord;
out mediump vec4 opixel;

uniform sampler2D a_texture;
uniform vec4 color;
void main ()
{
	opixel = texture(a_texture, v_texcoord) * color;
}

