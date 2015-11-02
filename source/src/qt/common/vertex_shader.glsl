attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;


// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Calculate vertex position in screen space
    gl_Position = vec4(vertex, 1.0);

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = texcoord;
}

