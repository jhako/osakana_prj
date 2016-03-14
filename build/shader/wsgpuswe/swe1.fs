
#version 400 core

uniform sampler2D us0;
uniform sampler2D x_us;
uniform sampler2D y_us;
uniform sampler2D obstacle_tex;
uniform sampler2D debug_tex;

const float dt = 0.00016;
const float dx = 0.01;
const float dy = 0.01;
const float g = 9.8;
const float c = 0.3;


//CFL条件 : dx > cdt


vec3 F(vec3 a)
{
	if(a.x < 0.000001) return vec3(0.0);
	else
	return vec3(a.y,
				a.y * a.y / a.x + 0.5f * g * a.x * a.x,
				a.y * a.z / a.x);
}

vec3 G(vec3 a)
{
	if(a.x < 0.000001) return vec3(0.0);
	else
	return vec3(a.z,
				a.y * a.z / a.x,
				a.z * a.z / a.x + 0.5f * g * a.x * a.x);
}

void main(void)
{
	ivec2 tpos = ivec2(gl_FragCoord.xy);
	ivec2 tsize = textureSize(x_us, 0);
	
	vec3 p = texelFetch(us0, tpos, 0).xyz;
	vec3 op = texelFetch(obstacle_tex, tpos, 0).xyz;
	
	vec3 w = texelFetchOffset(us0, tpos, 0, ivec2(-1, 0)).xyz;
	vec3 s = texelFetchOffset(us0, tpos, 0, ivec2(0, -1)).xyz;
	vec3 ow = texelFetchOffset(obstacle_tex, tpos, 0, ivec2(-1, 0)).xyz;
	vec3 os = texelFetchOffset(obstacle_tex, tpos, 0, ivec2(0, -1)).xyz;
	
//	if(ow.x > 0 || tpos.x == 0 || tpos.x == tsize.x - 1) w = p;
//	if(os.x > 0 || tpos.y == 0 || tpos.y == tsize.y - 1) s = p;

	vec3 xu = 0.5 * (p + w) - 0.5 * dt / dx * (F(p) - F(w));
	vec3 yu = 0.5 * (p + s) - 0.5 * dt / dy * (G(p) - G(s));
	
	xu.y -= c * 1.0 / 2.0 * (p.y / p.x + w.y / w.x) * xu.x;
	xu.z -= c * 1.0 / 2.0 * (p.z / p.x + w.z / w.x) * xu.x;
	yu.y -= c * 1.0 / 2.0 * (p.y / p.x + s.y / s.x) * yu.x;
	yu.z -= c * 1.0 / 2.0 * (p.z / p.x + s.z / s.x) * yu.x;
	
	if(tpos.x == 0 || tpos.y == 0)
	{
		xu.x = p.x;
		yu.x = p.x;
		xu.y = 0.0;
		xu.z = 0.0;
		yu.y = 0.0;
		yu.z = 0.0;
	}
	if(tpos.x == tsize.x - 1)
	{
		xu.x = w.x;
		yu.x = w.x;
		xu.y = 0.0;
		xu.z = 0.0;
		yu.y = 0.0;
		yu.z = 0.0;
	}
	if(tpos.y == tsize.y - 1)
	{
		xu.x = s.x;
		yu.x = s.x;
		xu.y = 0.0;
		xu.z = 0.0;
		yu.y = 0.0;
		yu.z = 0.0;
	}

	gl_FragData[0] = vec4(xu, 1.0);
	gl_FragData[1] = vec4(yu, 1.0);
	
	//debug
//	vec4 color = vec4(0.0);
//	gl_FragData[2] = color;
}
