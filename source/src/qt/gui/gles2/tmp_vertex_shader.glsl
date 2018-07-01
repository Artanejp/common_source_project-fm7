#if __VERSION__ >= 300
in mediump vec3 vertex;
in mediump vec2 texcoord;
out mediump vec2 v_texcoord;
#else
attribute mediump vec3 vertex;
attribute mediump vec2 texcoord;
varying mediump vec2 v_texcoord;
#endif
void main ()
{
  mediump vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = vertex;
  gl_Position = tmpvar_1;
  v_texcoord = texcoord;
}

