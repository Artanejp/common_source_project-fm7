// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014
precision  mediump float;

varying mediump vec2 v_texcoord;

uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;
uniform float phase;
uniform bool swap_byteorder;

// uniforms added for compatibility
//vec3 col;
//float mod_phase;
//float chroma_phase;

#define THREE_PHASE //options here include THREE_PHASE, TWO_PHASE or OLD_THREE_PHASE
#define COMPOSITE

// #include "ntsc-param.inc" //
#define PI 3.14159265

#if defined(TWO_PHASE)
#define CHROMA_MOD_FREQ (4.0 * PI / 15.0)
#elif defined(THREE_PHASE)
#define CHROMA_MOD_FREQ (PI / 3.0)
#endif

#if defined(COMPOSITE)
#define SATURATION 1.1
#define BRIGHTNESS 1.0
#define ARTIFACTING 1.3
#define FRINGING 1.2
#elif defined(SVIDEO)
#define SATURATION 1.0
#define BRIGHTNESS 1.0
#define ARTIFACTING 0.0
#define FRINGING 0.0
#endif

#if defined(COMPOSITE) || defined(SVIDEO)
mat3 mix_mat = mat3(
	BRIGHTNESS, FRINGING, FRINGING,
	ARTIFACTING, 2.0 * SATURATION, 0.0,
	ARTIFACTING, 0.0, 2.0 * SATURATION
);
#endif
// END "ntsc-param.inc" //

// moved from vertex
#define pix_no (v_texcoord.xy * source_size.xy * (target_size.xy / source_size.xy))

mat3 yiq2rgb_mat = mat3(
   1.0, 1.0, 1.0,
   0.956, -0.2720, -1.1060,
   0.6210, -0.6474, 1.7046
);

vec3 yiq2rgb(vec3 yiq)
{
   return (yiq * yiq2rgb_mat);
}

mat3 yiq_mat = mat3(
      0.2989, 0.5959, 0.2115,
      0.5870, -0.2744, -0.5229,
    0.1140, -0.3216, 0.3114
);
vec3 rgb2yiq(vec3 col)
{
   return (col * yiq_mat);
}

// Change Matrix: [RGB]->[YCbCr]

mat3 ycbcr_mat = mat3(
      0.29891, -0.16874,  0.50000,
      0.58661, -0.33126, -0.41869,
      0.11448,  0.50000, -0.08131
);
vec3 rgb2ycbcr(vec3 col)
{
	vec3 ycbcr = col * ycbcr_mat;
   return ycbcr;
}

mat3 ycbcr2rgb_mat = mat3(
	 1.0, 1.0, 1.0,
	 0.0, -0.34414 , 1.77200,
	 1.40200, -0.71414, 0.0
 );
 
vec3 ycbcr2rgb(vec3 ycbcr)
{
	//vec3 ra = ycbcr * vec3(1.0, 0.7, 1.0);
	return (ycbcr * ycbcr2rgb_mat);
}

void main() {
	// #include "ntsc-pass1-encode-demodulate.inc" //
	
	vec3 col = texture2D(a_texture, v_texcoord).rgb;
	vec3 ycbcr;
	if(swap_byteorder) {
		col.rgb = col.bgr;
	}
	//ycbcr = rgb2yiq(col);
	ycbcr = rgb2ycbcr(col);
//
// From https://ja.wikipedia.org/wiki/YUV#RGB%E3%81%8B%E3%82%89%E3%81%AE%E5%A4%89%E6%8F%9B
//  ITU-R BT.601 / ITU-R BT.709 (1250/50/2:1)
//  Y = 0.299 * R + 0.587 * G + 0.114 * B
//  Cb = -0.168736 * R - 0.331264 * G + 0.5 * B
//  Cr = 0.5 * R - 0.418688 * G - 0.081312 * B

#if defined(TWO_PHASE)
	float chroma_phase = PI * (mod(pix_no.y, 2.0) + phase);
#elif defined(THREE_PHASE)
	float chroma_phase = 0.6667 * PI * (mod(pix_no.y, 3.0) + phase);
#endif

	float mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ;

	float i_mod = cos(mod_phase);
	float q_mod = sin(mod_phase);
	ycbcr *= vec3(1.0, i_mod, q_mod); // Modulate
	ycbcr *= mix_mat; // Cross-talk
	ycbcr *= vec3(1.0, i_mod, q_mod); // Demodulate
	
	// yMax = (0.299+0.587+0.114) * (+-1.0) * (BRIGHTNESS + ARTIFACTING + ARTIFACTING) * (+-1.0)
	// CbMax = (-0.168736 -0.331264 + 0.5) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// CrMax = (0.5 - 0.418688 - 0.081312) * (+-1.0) * (FRINGING + 2*SATURATION) * (+-1.0)
	// -> y =  0 to +3.6
	//    Cb = 0 to +1.7
	//    Cr = 0 to +1.7
	ycbcr = ycbcr * vec3(0.277778 ,0.588235, 0.588235);
	// Normalise
	vec4 outvar = vec4(ycbcr, 1.0);
	gl_FragColor = outvar;
// END "ntsc-pass1-encode-demodulate.inc" //
}