varying mediump vec2 v_texcoord;
uniform sampler2D a_texture;
uniform mediump vec4 color;
void main ()
{
	gl_FragColor = (texture2D (a_texture, v_texcoord) * color);
}

