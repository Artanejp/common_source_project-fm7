#if __VERSION__ >= 300
in mediump vec3 vertex;
in mediump vec2 texcoord;
out mediump vec2 v_texcoord;
#else
attribute mediump vec3 vertex;
attribute mediump vec2 texcoord;
varying mediump vec2 v_texcoord;
#endif

uniform mat2 rotate_mat;
uniform bool rotate;
uniform mediump mat4 v_ortho;
void main ()
{
#if 0
  if (!(rotate)) {
    mediump vec4 tmpvar_1;
    tmpvar_1.w = 1.0;
    tmpvar_1.xyz = vertex;
    gl_Position = tmpvar_1 * v_ortho;
    v_texcoord = texcoord;
  } else {
    mediump vec4 tmpvar_2;
    tmpvar_2.w = 1.0;
    tmpvar_2.xyz = vertex;
    gl_Position = (mat4(0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) * tmpvar_2 * v_ortho);
    v_texcoord = texcoord;
  };
#else
	vec4 xyzw = vec4(vertex, 1.0);
	xyzw.xy = rotate_mat * xyzw.xy;
	
#if 0
	xyzw = mat4(0.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0) * xyzw;
#endif		   
	xyzw = xyzw * v_ortho;	
	gl_Position = xyzw;
    v_texcoord = texcoord;
#endif
}

