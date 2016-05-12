
#version 120

uniform int TX;
uniform int TY;
uniform sampler2D src;

varying vec2 pos;

const float coff0 = 1.0 / sqrt(8.0);
const float coffn = 1.0 / 2.0;

const float PI = 3.1415926535897932384626433832795;

float itfx(int n)
{
	return (2.0 * n + 1) / (2 * TX);
}
float itfy(int n)
{
	return (2.0 * n + 1) / (2 * TY);
}

void main(void)
{
	int ix = ivec2(gl_FragCoord.xy).x;
	int iy = ivec2(gl_FragCoord.xy).y;
	int dx = int(mod(ix, 8));
	int dy = int(mod(iy, 8));
	int bx = ix / 8 * 8;
	int by = iy / 8 * 8;
	vec3 F = vec3(0.0);
	vec3 f;
	float coff = dy == 0 ? coff0 : coffn;
	for(int i = 0; i < 8; ++i)
	{
		f = texture2D(src, vec2(pos.x, itfy(by + i))).xyz;
		F += f * coff * cos((2 * i + 1) * PI * dy / (2 * 8));
	}
	gl_FragColor = vec4(F, 1.0);
}