#ifdef GL_ES
 precision mediump float;
#endif 

varying vec2 v_texcoord;
uniform vec4 color;
uniform int tex_width;
uniform int tex_height;
uniform sampler2D a_texture;
//attribute tex_size = vec2(tex_width, tex_height);

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Set fragment color from texture
    vec2 tex_pos = v_texcoord;
    vec4 pixel_t = texture2D(a_texture, tex_pos);
    vec4 pixel_r;
    
    pixel_r = pixel_t * color;
    gl_FragColor = pixel_r;
}
