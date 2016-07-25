varying vec2 v_texcoord;
uniform vec4 color;
uniform vec3 chromakey;
uniform bool do_chromakey;
uniform sampler2D a_texture;
void main ()
{
  vec4 pixel_r_1;
  pixel_r_1 = texture2D (a_texture, v_texcoord);
  vec4 tmpvar_2;
  tmpvar_2.w = 1.0;
  tmpvar_2.xyz = chromakey;
  if ((do_chromakey == bool(1))) {
    pixel_r_1.w = 1.0;
    if ((pixel_r_1 != tmpvar_2)) {
      pixel_r_1 = (pixel_r_1 * color);
    } else {
      pixel_r_1 = vec4(0.0, 0.0, 0.0, 0.0);
    };
  } else {
    pixel_r_1 = (pixel_r_1 * color);
  };
  gl_FragColor = pixel_r_1;
}

