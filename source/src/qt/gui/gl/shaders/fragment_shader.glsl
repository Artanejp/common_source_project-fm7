
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
in vec2 v_texcoord;
out vec4 opixel;
#else
varying vec2 v_texcoord;
#endif

uniform sampler2D a_texture;
uniform vec4 color;
uniform vec2 distortion_v;
uniform mediump float luminance;
uniform mediump float lum_offset;

// Note: This distortion shading from:
// CRT-Geom.shader/curvature.fs , https://github.com/hizzlekizzle/quark-shaders 
vec2 radialDistortion(vec2 coord)
{
	vec2 cc = coord - vec2(0.5);
	vec2 dist = dot(cc, cc) * distortion_v;
	return coord + cc * (vec2(1.0) - dist) * dist;
}

void main ()
{
//	vec2 dist = radialDistortion(v_texcoord);
	vec2 dist = v_texcoord;
	bvec4 d;
#if (__VERSION__ >= 300)
	vec4 pixel	= texture(a_texture, dist);
#else
	vec4 pixel	= texture2D(a_texture, dist);
#endif
	_CONST vec2 high = vec2(1.0, 1.0);
	_CONST vec2 low  = vec2(0.0, 0.0);
	d.xy = lessThan(dist, low);
	d.zw = greaterThan(dist, high);
	if(any(d)) {
		pixel = vec4(0.0, 0.0, 0.0, 0.0);
	} else {
#if (__VERSION__ < 400)
		pixel = vec4(pixel.rgb, 1.0) * vec4(luminance, luminance, luminance, 1.0) + vec4(lum_offset, lum_offset, lum_offset, 0.0);
#else
		pixel = fma(vec4(pixel.rgb, 1.0), vec4(luminance, luminance, luminance, 1.0), vec4(lum_offset, lum_offset, lum_offset, 0.0));
#endif
	}
#if (__VERSION__ >= 300)
	opixel = pixel * color;
#else
	gl_FragColor = pixel * color;
#endif	
}
