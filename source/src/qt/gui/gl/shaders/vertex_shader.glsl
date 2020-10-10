#if defined(GL_ES)
	#define USE_GL_ES 1
#elif ((__VERSION__ >= 430) || defined(GL_es_profile))
	#if (GL_es_profile)
		#define USE_GL_ES 1
	#endif
#endif	
#if defined(USE_GL_ES)
	#ifdef HAS_FLOAT_TEXTURE
		#ifdef HAS_HALF_FLOAT_TEXTURE
		#extension GL_OES_texture_half_float : enable
		#else
		#extension GL_OES_texture_float : enable
		#endif
	#endif
	precision  mediump float;
#endif

#if (__VERSION__ >= 300)
#define _CONST const
#else
#define _CONST
#endif

#if (__VERSION__ >= 300)
in mediump vec3 vertex;
in mediump vec2 texcoord;
out mediump vec2 v_texcoord;
#else
attribute vec3 vertex;
attribute vec2 texcoord;
varying vec2 v_texcoord;
#endif

uniform mat2 rotate_mat;

void main ()
{
	vec2 xy = vertex.xy;
//	vec2 r = vec2(2.0, 2.0);
//	vec2 z = vec2(1.0) - (r - sqrt(r * r - xy * xy));
//	vec2 theta = vec2(asin(xy.x / r.x), asin(xy.y / r.y));
	xy = rotate_mat * xy;
//	xy = vec2(1.0) - (xy * vec2(cos(theta.x), cos(theta.y)));

	gl_Position = vec4(xy, vertex.z, 1.0);
    v_texcoord = texcoord;
}

