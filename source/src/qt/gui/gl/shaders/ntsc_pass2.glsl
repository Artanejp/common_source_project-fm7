// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014
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

#define GAMMA_CORRECTION //comment to disable gamma correction, usually because higan's gamma correction is enabled or you have another shader already doing it
#define CRT_GAMMA 2.5
#define DISPLAY_GAMMA 2.1

#define GAMMA_FACTOR (CRT_GAMMA / DISPLAY_GAMMA)

#if (__VERSION__ >= 300)
in mediump vec2 v_texcoord;
out vec4 opixel;
#else // ES2.x or Compatible profile
varying mediump vec2 v_texcoord;
#endif

uniform sampler2D a_texture;
uniform vec2 source_size;
uniform vec2 target_size;


#define TAPS 24
#define THREE_PHASE 

#if (__VERSION__ >= 300)
	#if defined OLD_THREE_PHASE
	_CONST float luma_filter[TAPS + 1] = float[TAPS + 1](
		-0.000071070,
		-0.000032816,
		0.000128784,
		0.000134711,
		-0.000226705,
		-0.000777988,
		-0.000997809,
		-0.000522802,
		0.000344691,
		0.000768930,
		0.000275591,
		-0.000373434,
		0.000522796,
		0.003813817,
		0.007502825,
		0.006786001,
		-0.002636726,
		-0.019461182,
		-0.033792479,
		-0.029921972,
		0.005032552,
		0.071226466,
		0.151755921,
		0.218166470,
		0.243902439);
	_CONST float chroma_filter[TAPS + 1] = float[TAPS + 1](
		0.001845562,
		0.002381606,
		0.003040177,
		0.003838976,
		0.004795341,
		0.005925312,
		0.007242534,
		0.008757043,
		0.010473987,
		0.012392365,
		0.014503872,
		0.016791957,
		0.019231195,
		0.021787070,
		0.024416251,
		0.027067414,
		0.029682613,
		0.032199202,
		0.034552198,
		0.036677005,
		0.038512317,
		0.040003044,
		0.041103048,
		0.041777517,
		0.042004791);
	#elif defined(THREE_PHASE)
// #include "ntsc-decode-filter-3phase.inc" //
	_CONST float luma_filter[TAPS + 1] = float[TAPS + 1](
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552);

	_CONST float chroma_filter[TAPS + 1] = float[TAPS + 1](
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396);
	// END "ntsc-decode-filter-3phase.inc" //
	#elif defined(TWO_PHASE)
// #include "ntsc-decode-filter-3phase.inc" //
	_CONST float luma_filter[TAPS + 1] = float[TAPS + 1](
		-0.000012020,
		-0.000022146,
		-0.000013155,
		-0.000012020,
		-0.000049979,
		-0.000113940,
		-0.000122150,
		-0.000005612,
		0.000170516,
		0.000237199,
		0.000169640,
		0.000285688,
		0.000984574,
		0.002018683,
		0.002002275,
		-0.000909882,
		-0.007049081,
		-0.013222860,
		-0.012606931,
		0.002460860,
		0.035868225,
		0.084016453,
		0.135563500,
		0.175261268,
		0.190176552);
	_CONST float chroma_filter[TAPS + 1] = float[TAPS + 1](
		-0.000118847,
		-0.000271306,
		-0.000502642,
		-0.000930833,
		-0.001451013,
		-0.002064744,
		-0.002700432,
		-0.003241276,
		-0.003524948,
		-0.003350284,
		-0.002491729,
		-0.000721149,
		0.002164659,
		0.006313635,
		0.011789103,
		0.018545660,
		0.026414396,
		0.035100710,
		0.044196567,
		0.053207202,
		0.061590275,
		0.068803602,
		0.074356193,
		0.077856564,
		0.079052396);
	// END "ntsc-decode-filter-3phase.inc" //
	#endif
#else // __VERSION__ < 300, GLES2 or GL3
uniform float luma_filter[TAPS + 1];
uniform float chroma_filter[TAPS + 1];
// END "ntsc-decode-filter-3phase.inc" //
#endif


// #include ntsc-rgbyuv.inc //
_CONST mat3 ycbcr2rgb_mat = mat3(
	 1.0, 1.0, 1.0,
	 0.0, -0.34414 , 1.77200,
	 1.40200, -0.71414, 0.0
 );
 
vec3 ycbcr2rgb(vec3 foo)
{
	return  foo * ycbcr2rgb_mat;
}
// END ntsc-rgbyuv.inc //
// #include ntsc-rgbyuv.inc //
_CONST mat3 yiq2rgb_mat = mat3(
   1.0, 1.0, 1.0,
   0.956, -0.2720, -1.1060,
   0.6210, -0.6474, 1.7046
);

vec3 yiq2rgb(vec3 yiq)
{
   return (yiq * yiq2rgb_mat);
}

_CONST mat3 yiq_mat = mat3(
      0.2989, 0.5959, 0.2115,
      0.5870, -0.2744, -0.5229,
      0.1140, -0.3216, 0.3114
);

vec3 rgb2yiq(vec3 col)
{
   return (col * yiq_mat);
}
// END ntsc-rgbyuv.inc //

#define fixCoord (v_texcoord - vec2(0.5 / source_size.x, 0.0)) // Compensate for decimate-by-2.
#if __VERSION__ >= 300
#define fetch_offset(offset, one_x) \
	texture(a_texture, fixCoord + vec2((offset) * (one_x), 0.0)).xyz
#else
#define fetch_offset(offset, one_x) \
	texture2D(a_texture, fixCoord + vec2((offset) * (one_x), 0.0)).xyz
#endif

void main()
{
	float one_x = 1.0 / source_size.x;
	vec3 signal = vec3(0.0);
	for (int i = 0; i < TAPS; i++)
	{
		float offset = float(i);
		vec3 sums = fetch_offset(offset - float(TAPS), one_x) +
			fetch_offset(float(TAPS) - offset, one_x);
	
		signal += sums * vec3(luma_filter[i], chroma_filter[i], chroma_filter[i]);
	}
#if __VERSION__ >= 300
	signal += texture(a_texture, fixCoord).xyz * vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);
#else
	signal += texture2D(a_texture, fixCoord).xyz * vec3(luma_filter[TAPS], chroma_filter[TAPS], chroma_filter[TAPS]);
#endif
	
	vec3 rgb;	
	rgb = yiq2rgb(signal);

#ifdef GAMMA_CORRECTION
	vec3 gamma = vec3(CRT_GAMMA / DISPLAY_GAMMA);
	rgb = pow(rgb, gamma.rgb);
#endif
	
#if (__VERSION__ >= 300)
	opixel = vec4(rgb, 1.0);
#else
	gl_FragColor = vec4(rgb, 1.0);
#endif
}
