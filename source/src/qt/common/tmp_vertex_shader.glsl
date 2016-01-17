#ifdef GL_ES
 precision mediump float;
#endif 

attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;

uniform bool rotate;

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
	gl_Position = vec4(vertex, 1.0);
	v_texcoord = texcoord;
}

