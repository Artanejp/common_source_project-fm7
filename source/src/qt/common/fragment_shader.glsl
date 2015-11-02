varying vec2 v_texcoord;
uniform vec4 color;

uniform sampler2D a_texture;

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Set fragment color from texture
    //gl_FragColor = texture2D(a_texture, v_texcoord );
    //gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    gl_FragColor = color;
}
