//precision mediump float;

in vec2 v_texcoord;

out vec4 opixel;

uniform usampler2D a_texture;
uniform vec4 color;
uniform vec2 distortion_v;

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
	vec4 pixel	= texture(a_texture, dist);
	if((dist.x < 0.0) || (dist.x > 1.0) || (dist.y < 0.0) || (dist.y > 1.0)) {
		pixel = vec4(0.0, 0.0, 0.0, 1.0);
	}
	opixel = pixel * color;
}
