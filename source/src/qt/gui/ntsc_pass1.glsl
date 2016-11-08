// NTSC Shader - written by Hans-Kristian Arntzen
// License: GPLv3
// pulled from git://github.com/libretro/common-shaders.git on 01/30/2014

varying vec2 v_texcoord;

uniform sampler2D a_texture;
uniform vec4 source_size;
uniform vec4 target_size;
uniform float phase;

// uniforms added for compatibility
//vec3 col;
//float mod_phase;
//float chroma_phase;

#define THREE_PHASE
#define COMPOSITE

// #include "ntsc-param.inc" //
#define PI 3.14159265

#if defined(TWO_PHASE)
#define CHROMA_MOD_FREQ (4.0 * PI / 15.0)
#elif defined(THREE_PHASE)
#define CHROMA_MOD_FREQ (PI / 3.0)
#endif

#if defined(COMPOSITE)
#define SATURATION 1.0
#define BRIGHTNESS 1.0
#define ARTIFACTING 0.1
#define FRINGING 0.05
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
   return (col * ycbcr_mat);
}

void main() {
	// #include "ntsc-pass1-encode-demodulate.inc" //
	
	vec3 col = texture2D(a_texture, v_texcoord).rgb;
	vec3 ycbcr;
	ycbcr = rgb2ycbcr(col);

#if defined(TWO_PHASE)
	float chroma_phase = PI * (mod(pix_no.y, 2.0) + phase);
#elif defined(THREE_PHASE)
	float chroma_phase = 0.6667 * PI * (mod(pix_no.y, 3.0) + phase);
#endif

	float mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ;

	float i_mod = cos(mod_phase);
	float q_mod = sin(mod_phase);

	ycbcr.yz *= vec2(i_mod, q_mod); // Modulate
	ycbcr *= mix_mat; // Cross-talk
	ycbcr.yz *= vec2(i_mod, q_mod); // Demodulate
	ycbcr = ycbcr + vec3(0.0, 0.5, 0.5);
	gl_FragColor = vec4(ycbcr, 1.0);

// END "ntsc-pass1-encode-demodulate.inc" //
}