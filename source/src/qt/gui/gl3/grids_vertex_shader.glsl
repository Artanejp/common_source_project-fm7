attribute vec3 vertex;
uniform mat2 rotate_mat;
void main ()
{
	vec4 xyzw = vec4(vertex, 1.0);
	xyzw.xy = rotate_mat * xyzw.xy;
#if 0
	xyzw = mat4(0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) * xyzw;
#endif		   
	gl_Position = xyzw;
}

