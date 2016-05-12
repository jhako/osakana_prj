
#version 120

uniform int TX;
uniform int TY;
uniform sampler2D src;
uniform sampler2D fzigzag;
uniform sampler2D quant;

varying vec2 pos;

float itfx(int n)
{
	return (2.0 * n + 1) / (2 * TX);
}
float itfy(int n)
{
	return (2.0 * n + 1) / (2 * TY);
}

const float qp = 1;

void main(void)
{
	int ix = ivec2(gl_FragCoord.xy).x;
	int iy = ivec2(gl_FragCoord.xy).y;
	int dx = int(mod(ix, 8));
	int dy = int(mod(iy, 8));
	int bx = ix / 8 * 8;
	int by = iy / 8 * 8;
	ivec2 zp = ivec2(texture2D(fzigzag, vec2((2.0 * dx + 1) / (2 * 8), (2.0 * dy + 1) / (2 * 8))).xy * 256);
	//ivec2 zp = ivec2(texture2D(fzigzag, vec2(dx / 8, dy / 8)).xy * 255);
	vec3 F = texture2D(src, vec2(itfx(bx + zp.x), itfy(by + zp.y))).xyz;
	float Q = texture2D(quant, vec2((2.0 * zp.x + 1) / (2 * 8), (2.0 * zp.y + 1) / (2 * 8))).r * 256.0;
	
	/*
	F *= 255;
	F = floor(8.0 * F / ( Q * qp) + 0.5);
	if(F.r > 2047.0) F.r = 2047.0;
	else if(F.r < -2048) F.r = -2048;
	if(F.g > 2047.0) F.g = 2047.0;
	else if(F.g < -2048.0) F.g = -2048.0;
	if(F.b > 2047.0) F.b = 2047.0;
	else if(F.b < -2048.0) F.b = -2048.0;
	F += 2048;
	F /= 16;
	F /= 256;
	*/

	F = floor(F * 8.0 * 256.0 / (Q * qp) + 0.5) * (0.5 / (256.0 * 8.0)) + vec3(127.0 / 255.0);

	/*
	if(F.r > 1.0) F.r = 1.0;
	else if(F.r < 0.0) F.r = 0.0;
	if(F.g > 1.0) F.g = 1.0;
	else if(F.g < 0.0) F.g = 0.0;
	if(F.b > 1.0) F.b = 1.0;
	else if(F.b < 0.0) F.b = 0.0;
	*/
	
	
	
	/*
	F *= 256;
	//F = floor(8.0 * F / ( Q * qp) + 0.5);
	F = 8.0 * F / ( Q * qp);
	//F = ivec3(F);
	F /= 256;
	F /= 16;
	F += vec3(0.5);
	*/
	gl_FragColor = vec4(F, 1.0);
	
	//gl_FragColor = vec4(Q / 83, 0, 0, 1);
}