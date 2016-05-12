
#version 400 core


out vec4 FragColor;
in vec3 gFacetNormal;
in vec4 vpos;
in vec4 view;

uniform sampler2D under_tex;
uniform sampler2D cau_tex;
uniform samplerCube envmap;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

const float etaRatio = 1.0 / 1.333; //air->water

const vec3 light_dir = vec3(0.01, 0.02, -1.0);
//const vec3 light_dir = vec3(0.167, -0.13, -1.0);

float amplify(float d, float scale, float offset)
{
	d = scale * d + offset;
	d = clamp(d, 0, 1);
	d = 1 - exp2(-2*d*d);
	return d;
}

void main()
{
	vec3 N = normalize(gFacetNormal);
	vec3 I = normalize(view.xyz);		 // 入射ベクトル
    vec3 R = reflect(I, N);           	 // 反射ベクトル
    vec3 T = refract(I, N, etaRatio);    // 屈折ベクトル
    vec3 IL = normalize((modelViewMatrix * vec4(light_dir, 0.0)).xyz); //ライト入射ベクトル（平行光源）
    vec3 RL = reflect(IL, N);            // 反射ベクトル

//	vec3 diff = vec3(1.0, 1.0, 1.0) * max(dot(N, IL), 0.0);
	
	// 反射光の計算
	vec4 reflecColor = vec4(0.0);
	
	reflecColor += textureCube(envmap, R);
	
	//vec3 halfway = normalize(IL + view.xyz);
	//vec3 specular = vec3(1.0, 1.0, 1.0) * pow(max(dot(N, halfway), 0.0), 64.0);
    vec3 specular = vec3(0.2, 0.2, 0.2) * pow(max(dot(RL, N), 0.0), 64.0);
	
	reflecColor += vec4(specular, 0.0);
	
	// 屈折環境色の計算
    vec4 refracColor = vec4(0.0);
	//float depth = vpos.z - 9.9;
	float depth = vpos.z - 9.9;
	vec2 coord = vpos.xy * 0.5 + vec2(0.5, 0.5) + T.xy * depth / T.z * 0.5;
	refracColor = texture2D(under_tex, coord);
    refracColor.a = 1.0;
	
	// コークティクス
	//vec4 caustics = texture(cau_tex, vpos.xy * 0.5 + vec2(0.5, 0.5));
	float cau_depth = vpos.z - 9.7;
	vec2 cau_coord = vpos.xy * 0.5 + vec2(0.5, 0.5) + T.xy * cau_depth / T.z * 0.5;
	vec4 caustics = texture(cau_tex, cau_coord);
	
	//refracColor = refracColor * 1.0 + (refracColor * caustics) * 0.5;
	refracColor = refracColor * 0.1 + (refracColor * caustics) * 1.2;
	
	// フレネル効果の近似計算
    float u = dot(-I, N);
	float f0 = 0.1; //水は0.02だが，波紋が見にくくなる
	float F = f0 + (1.0 - f0) * pow(1.0 - u, 5);
	float Kr = F;
	
	
	//vec3 color = reflecColor.xyz;
	//vec3 color = refracColor.xyz;
	//vec3 color = specular;
	//vec3 color = Kr * reflecColor.xyz + (1.0 - Kr) * refracColor.xyz;
	vec3 color = mix(refracColor.xyz, reflecColor.xyz, Kr);

	FragColor = vec4(color, 1.0);
//	FragColor = vec4(1.0);
//	if(vpos.x > 0 && vpos.y > 0)
//	{
//		FragColor = caustics;
//		//FragColor = texture(cau_tex, vpos.xy * 0.5 + vec2(0.5, 0.5));
//	}
}
