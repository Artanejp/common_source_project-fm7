#if __VERSION__ >= 300
in mediump vec2 v_texcoord;
uniform sampler2D a_texture;
uniform mediump vec4 color;
out mediump vec4 opixel;
#else
varying mediump vec2 v_texcoord;
uniform sampler2D a_texture;
uniform mediump vec4 color;
#endif
void main ()
{
#if __VERSION__ >= 300
	opixel = (texture(a_texture, v_texcoord) * color);
#else
	gl_FragColor = (texture2D (a_texture, v_texcoord) * color);
#endif		 
}

