in vec3 vertex;
in vec2 texcoord;
out vec2 v_texcoord;

void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = vertex;
  gl_Position = tmpvar_1;
  v_texcoord = texcoord;
}

