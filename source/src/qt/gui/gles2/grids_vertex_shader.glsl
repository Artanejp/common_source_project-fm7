#if __VERSION__ >= 300
in mediump vec3 vertex;
#else
attribute mediump vec3 vertex;
#endif

uniform mat2 rotate_mat;

void main ()
{
	vec2 xy = vertex.xy;
	xy = rotate_mat * xy;
	
	gl_Position = vec4(xy, vertex.z, 1.0);
}

