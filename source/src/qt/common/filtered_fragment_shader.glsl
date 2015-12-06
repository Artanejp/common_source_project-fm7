#ifdef GL_ES
 precision mediump float;
#endif 
// This is 3x3 Multi-Sampling Filtered fragment shader.
// Quite heavy.
varying vec2 v_texcoord;
uniform vec4 color;
uniform float tex_width;
uniform float tex_height;
uniform sampler2D a_texture;
//attribute tex_size = vec2(tex_width, tex_height);

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Set fragment color from texture
    vec2 tex_pos = v_texcoord;
    vec2 t_factor;
    vec4 pixel_r;
    vec4 pixel_s;
    vec2 tex_p;
    t_factor.x = 0.125 / tex_width;
    t_factor.y = 0.125 / tex_height;
    tex_p = tex_pos;
    pixel_s = texture2D(a_texture, (tex_p + vec2(-1.0, +1.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(-0.0, +1.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(+1.0, +1.0) * t_factor)) * vec4(0.05) +
	      texture2D(a_texture, (tex_p + vec2(-1.0, +0.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(-0.0, +0.0) * t_factor)) * vec4(0.60) +
              texture2D(a_texture, (tex_p + vec2(+1.0, +0.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(-1.0, -1.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(-0.0, -1.0) * t_factor)) * vec4(0.05) +
              texture2D(a_texture, (tex_p + vec2(+1.0, -1.0) * t_factor)) * vec4(0.05);
    
    pixel_r = pixel_s * color;
    gl_FragColor = pixel_r;
}
