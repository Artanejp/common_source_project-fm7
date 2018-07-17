attribute vec3 vertex;
uniform mat2 rotate_mat;

void main ()
{
	vec2 xy = vertex.xy;
	xy = rotate_mat * xy;
	gl_Position = vec4(xy, vertex.z, 1.0);
}

