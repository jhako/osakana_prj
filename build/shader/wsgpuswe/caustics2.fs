

#version 330 core

uniform sampler2D cau_p1;
uniform sampler2D cau_p2;

in vec4 vpos;
out vec4 FragData0;
out vec4 FragData1;

const float etaRatio = 1.0 / 1.333; //air->water

void main(void)
{
	float color = 0;
	
	ivec2 tpos = ivec2(gl_FragCoord.xy);
	
	color += texelFetchOffset(cau_p1, tpos, 0, ivec2(0, -3)).r;
	color += texelFetchOffset(cau_p1, tpos, 0, ivec2(0, -2)).g;
	color += texelFetchOffset(cau_p1, tpos, 0, ivec2(0, -1)).b;
	color += texelFetch(cau_p1, tpos, 0).a;
	color += texelFetchOffset(cau_p2, tpos, 0, ivec2(0, 1)).r;
	color += texelFetchOffset(cau_p2, tpos, 0, ivec2(0, 2)).g;
	color += texelFetchOffset(cau_p2, tpos, 0, ivec2(0, 3)).b;
	
	FragData0 = vec4(color, color, color, 1.0);
}




/*
#version 330 core

uniform sampler2D us_tex;
uniform sampler2D nrm_tex;

const int R_surf = 640;
const int R_tex = 100;

in vec4 vpos;

const float etaRatio = 1.0 / 1.333; //air->water

float cross2d(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

void main(void)
{
	vec2 t_pos = vpos.xy * 0.5 + vec2(0.5, 0.5);
	ivec2 ti_p1 = ivec2(t_pos * (R_tex - 1));
	ivec2 ti_p2 = ti_p1 + ivec2(1, 0);
	ivec2 ti_p3 = ti_p1 + ivec2(0, 1);
	vec2 ti_p = t_pos * (R_tex - 1);
	float S = abs(cross2d(ti_p2 - ti_p1, ti_p3 - ti_p1));
	float r1 = abs(cross2d(ti_p2 - ti_p, ti_p3 - ti_p)) / S;
	float r2 = abs(cross2d(ti_p1 - ti_p, ti_p3 - ti_p)) / S;
	float r3 = abs(cross2d(ti_p1 - ti_p, ti_p2 - ti_p)) / S;
	float nx1 = texelFetch(nrm_tex, ti_p1, 0).r;
	float ny1 = texelFetch(nrm_tex, ti_p1, 0).g;
	float nx2 = texelFetch(nrm_tex, ti_p2, 0).r;
	float ny2 = texelFetch(nrm_tex, ti_p2, 0).g;
	float nx3 = texelFetch(nrm_tex, ti_p3, 0).r;
	float ny3 = texelFetch(nrm_tex, ti_p3, 0).g;
	vec3 N = r1 * normalize(vec3(nx1, ny1, 1.0))
			 + r2 * normalize(vec3(nx2, ny2, 1.0))
			 + r3 * normalize(vec3(nx3, ny3, 1.0));
	vec3 IL = -vec3(0, 0, 1.0);
	vec3 IT = refract(IL, N, etaRatio);
	vec3 nIT = -normalize(IT);
	float t = pow(nIT.z, 5000);
	gl_FragData[0] = vec4(t, t, t, 1.0);
	//gl_FragData[0] = vec4(r1, r2, r3, 1.0);
}
*/

