#if __VERSION__ >= 300
in mediump vec3 vertex;
in mediump vec2 texcoord;
out mediump vec2 v_texcoord;
#else
attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;
#endif
uniform mat2 rotate_mat;
void main ()
{
	vec4 xyzw = vec4(vertex, 1.0);
	xyzw.xy = rotate_mat * xyzw.xy;
#if 0
	xyzw = mat4(0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) * xyzw;
#endif		   
	gl_Position = xyzw;
    v_texcoord = texcoord;
}

