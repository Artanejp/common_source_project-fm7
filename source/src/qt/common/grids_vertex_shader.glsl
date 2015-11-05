#ifdef GL_ES
 precision mediump float;
#endif 

attribute vec3 vertex;
uniform bool rotate;

const vec4 rot_line1 = vec4(0.0, -1.0, 0.0, 0.0);
const vec4 rot_line2 = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 rot_line3 = vec4(0.0, 0.0, 1.0, 0.0);
const vec4 rot_line4 = vec4(0.0, 0.0, 0.0, 1.0);
const mat4 rot_matrix = mat4(rot_line1, rot_line2,
                             rot_line3, rot_line4);
// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    // Calculate vertex position in screen space
    if(!rotate) {
        gl_Position = vec4(vertex, 1.0);
    } else {
        gl_Position = rot_matrix * vec4(vertex, 1.0);
    }
}

