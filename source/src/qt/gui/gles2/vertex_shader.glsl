attribute mediump vec3 vertex;
attribute mediump vec2 texcoord;
varying mediump vec2 v_texcoord;
uniform bool rotate;
uniform mediump mat4 v_ortho;
void main ()
{
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
}

