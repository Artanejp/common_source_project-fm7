attribute vec3 vertex;
void main ()
{
    vec4 tmpvar_1;
    tmpvar_1.w = 1.0;
    tmpvar_1.xyz = vertex.xyz;
    gl_Position = tmpvar_1;
}

