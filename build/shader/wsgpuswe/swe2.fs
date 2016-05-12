
#version 130

uniform sampler2D us0;
uniform sampler2D x_us;
uniform sampler2D y_us;
uniform sampler2D nrm_tex;

const float dt = 0.00016;
const float dx = 0.01;
const float dy = 0.01;
const float g = 9.8;

out vec4 FragData0;
out vec4 FragData1;

//CFL条件 : dx > cdt

vec3 F(vec3 a)
{
//	if(a.x < 0.000001) return vec3(0.0);
//	else
	return vec3(a.y,
				a.y * a.y / a.x + 0.5f * g * a.x * a.x,
				a.y * a.z / a.x);
}

vec3 G(vec3 a)
{
//	if(a.x < 0.000001) return vec3(0.0);
//	else
	return vec3(a.z,
				a.y * a.z / a.x,
				a.z * a.z / a.x + 0.5f * g * a.x * a.x);
}


void main(void)
{
	ivec2 tpos = ivec2(gl_FragCoord.xy);
	vec3 p = texelFetch(us0, tpos, 0).xyz;
	
	vec3 xp = texelFetch(x_us, tpos, 0).xyz;
	vec3 yp = texelFetch(y_us, tpos, 0).xyz;
	vec3 xe = texelFetchOffset(x_us, tpos, 0, ivec2(1, 0)).xyz;
	vec3 yn = texelFetchOffset(y_us, tpos, 0, ivec2(0, 1)).xyz;

	vec3 u = p - dt / dx * (F(xe) - F(xp)) - dt / dy * (G(yn) - G(yp));
	
	FragData0 = vec4(u, 1.0);

	
	vec3 v1 = vec3(1.0f, 0.0f, (xe - xp) / (dx));
	vec3 v2 = vec3(0.0f, 1.0f, (yn - yp) / (dy));
	
	vec3 n = cross(v1, v2);
	
	FragData1.r = n.x;
	FragData1.g = n.y;
}
