#ifdef GL_ES
 precision mediump float;
#endif 

attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;

uniform bool rotate;

const vec4 rot_line1 = vec4(0.0, -1.0, 0.0, 0.0);
const vec4 rot_line2 = vec4(1.0, 0.0, 0.0, 0.0);
const vec4 rot_line3 = vec4(0.0,  0.0, 1.0, 0.0);
const vec4 rot_line4 = vec4(0.0,  0.0, 0.0, 1.0);
const mat4 rot_matrix = mat4(rot_line1, rot_line2,
                             rot_line3, rot_line4);

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
    if(!rotate) {
        gl_Position = vec4(vertex, 1.0);
	v_texcoord = texcoord;
    } else {
        gl_Position = rot_matrix * vec4(vertex, 1.0);
	v_texcoord = texcoord;
    }
}

