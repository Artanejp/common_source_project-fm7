#if __VERSION__ >= 300
in mediump vec3 vertex;
#else
attribute mediump vec3 vertex;
#endif

uniform mat2 rotate_mat;
//uniform bool rotate;
void main ()
{
#if 0
  if (!(rotate)) {
    mediump vec4 tmpvar_1;
    tmpvar_1.w = 1.0;
    tmpvar_1.xyz = vertex;
    gl_Position = tmpvar_1;
  } else {
    mediump vec4 tmpvar_2;
    tmpvar_2.w = 1.0;
    tmpvar_2.xyz = vertex;
    gl_Position = (mat4(0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) * tmpvar_2);
  };
#else
	vec2 xy = vertex.xy;
	xy = rotate_mat * xy;
	
	gl_Position = vec4(xy, vertex.z, 1.0);
#endif		
}

