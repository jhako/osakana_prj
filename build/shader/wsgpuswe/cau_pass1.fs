
#version 330 core

uniform sampler2D us_tex;
uniform sampler2D nrm_tex;

#define N 7
#define Nhalf 3

in vec4 vpos;
out vec4 FragData0;
out vec4 FragData1;

const float etaRatio = 1.0 / 1.333; //air->water
const float x_scale = 2.0 / 640;
const float y_scale = 2.0 / 640;

void main(void)
{
	float CI[N];
	for(int i = 0; i < N; ++i)
	{
		CI[i] = 0;
	}
	float gy[N];
	for(int i = 0; i < N; ++i)
	{
		gy[i] = vpos.y + y_scale * (i - Nhalf);
	}
	for(int i = 0; i < N; ++i)
	{
		vec3 cp = vpos.xyz + (i - Nhalf) * vec3(x_scale, 0, 0);
		float nx = texture(nrm_tex, cp.xy * 0.5 + vec2(0.5, 0.5)).r;
		float ny = texture(nrm_tex, cp.xy * 0.5 + vec2(0.5, 0.5)).g;
		vec3 sN = normalize(vec3(nx, ny, 1.0));
		vec3 IL = normalize(vec3(0.01, 0.02, -1.0));
		//vec3 IL = normalize(vec3(0.167, -0.13, -1.0));
		vec3 IT = refract(IL, sN, etaRatio);
		cp.z = texture(us_tex, cp.xy * 0.5 + vec2(0.5, 0.5)).r - 9.7f;
		vec2 ray = vec2(IT.x * cp.z / IT.z, IT.y * cp.z / IT.z);
		vec3 ip = vec3(cp.xy + ray, 0.0);
		float ax = max(0, 1.0 - abs(vpos.x - ip.x) * 6.0);
		for(int j = 0; j < N; j++)
		{
			float ay = max(0, 1.0 - abs(gy[j] - ip.y) * 6.0);
			CI[j] += ax*ay / (N * N);
		}
	}
	
	FragData0 = vec4(CI[0], CI[1], CI[2], CI[3]);
	FragData1 = vec4(CI[4], CI[5], CI[6], 1.0);
	//gl_FragData[0] = vec4(1.0);
}

