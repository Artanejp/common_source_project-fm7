//precision mediump float;
in mediump vec3 vertex;
in mediump vec2 texcoord;

out mediump vec2 v_texcoord;

uniform mat2 rotate_mat;
void main ()
{
	vec2 xy = vertex.xy;
//	vec2 r = vec2(2.0, 2.0);
//	vec2 z = vec2(1.0) - (r - sqrt(r * r - xy * xy));
//	vec2 theta = vec2(asin(xy.x / r.x), asin(xy.y / r.y));
	xy = rotate_mat * xy;
//	xy = vec2(1.0) - (xy * vec2(cos(theta.x), cos(theta.y)));
	gl_Position = vec4(xy, vertex.z, 1.0);
    v_texcoord = texcoord;
}

