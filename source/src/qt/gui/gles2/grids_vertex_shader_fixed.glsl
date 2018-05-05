attribute mediump vec3 vertex;
void main ()
{
  mediump vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = vertex;
  gl_Position = tmpvar_1;
}

