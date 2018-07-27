//precision mediump float;

in vec2 v_texcoord;
in vec2 v_xy;
in float vertex_id;
out uvec4 opixel;
uniform float tex_width;
uniform float tex_height;

uniform usamplerBuffer s_texture;
//uniform usampler2D a_texture;
void main ()
{
	//uvec4 pixel = texture(a_texture, v_texcoord);
	uvec4 pixel = texelFetch(s_texture, int(v_texcoord.s * tex_width + v_texcoord.t * tex_width * tex_height));
	//uvec4 pixel = texelFetch(s_texture, int(vertex_id));
	opixel = pixel;
}
