#ifdef GL_ES
 precision mediump float;
#endif 

attribute vec3 vertex;

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Calculate vertex position in screen space
    gl_Position = vec4(vertex, 1.0);
    //gl_Position = gl_Vertex;
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
}

