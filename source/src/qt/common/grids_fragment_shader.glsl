#ifdef GL_ES
 precision mediump float;
#endif 

uniform vec4 color; // Note: BRGA.

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Set fragment color from texture
    gl_FragColor = color;
}
