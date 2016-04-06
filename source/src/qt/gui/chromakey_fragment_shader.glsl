#ifdef GL_ES
 precision mediump float;
#endif 

varying vec2 v_texcoord;
uniform vec4 color;
uniform vec3 chromakey;
uniform bool do_chromakey;

uniform sampler2D a_texture;

// This is from sample of Qt5 , see http://doc.qt.io/qt-5/qtopengl-cube-example.html .
void main()
{
	// Set fragment color from texture
	vec4 pixel_t = texture2D(a_texture, v_texcoord );
	vec4 pixel_r = pixel_t;
	vec4 c = vec4(chromakey, 1.0);
    
	if(do_chromakey == true) {
		pixel_r.a = 1.0;
		if(pixel_r != c) { // Chromakey;
			pixel_r = pixel_r * color;
		} else {
			pixel_r = vec4(0.0, 0.0, 0.0, 0.0);
		}
	} else {
		pixel_r = pixel_r * color;
	}
       	gl_FragColor = pixel_r;
}
