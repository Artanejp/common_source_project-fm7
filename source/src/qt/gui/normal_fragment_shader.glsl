varying vec2 v_texcoord;
uniform vec4 color;
uniform sampler2D a_texture;
void main ()
{
	gl_FragColor = (texture2D (a_texture, v_texcoord) * color);
}

