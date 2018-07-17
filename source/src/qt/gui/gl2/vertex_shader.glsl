attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;

uniform mat2 rotate_mat;
void main ()
{
	vec2 xy = vertex.xy;
	xy = rotate_mat * xy;
	gl_Position = vec4(xy, vertex.z, 1.0);
    v_texcoord = texcoord;
}

