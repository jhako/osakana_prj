#version 120


uniform samplerCube envmap;


// バーテックスシェーダから受け取る変数
varying vec4 vPos;
varying vec3 vNrm;
varying vec3 vPosW;
//varying vec3 eye;
//varying vec3 vNrmW;
//varying vec3 ePosW;
 
// GLから設定される定数(uniform)
const float etaRatio = 1.0 / 1.333; //air->water
//const float etaRatio = 1.333; //water->air
const float x_scale = 1.0/640.0;
const float y_scale = 1.0/640.0;
uniform sampler2D tex;

void main(void)
{
    // 入射，反射，屈折ベクトルの計算
    vec3 N = normalize(vNrm);            // 法線ベクトル
//	vec3 I = normalize(vPos.xyz-eye);    // 入射ベクトル
	vec3 I = normalize(vPos.xyz);		 // 入射ベクトル
//	vec3 IL = vec3(0.0, 0.0, 1.0); 	 // 平行光源
//	vec3 IL = normalize(gl_LightSource[0].position.xyz);
	vec3 IL = normalize((gl_LightSource[0].position * vPos.w - gl_LightSource[0].position.w * vPos).xyz);
    vec3 R = reflect(I, N);           	 // 反射ベクトル
    vec3 T = refract(I, N, etaRatio);    // 屈折ベクトル
    vec3 RL = reflect(-IL, N);            // 反射ベクトル

	
/*
	// フレネル効果の直接計算
	float A = etaRatio;
    float B = dot(-I, N);
    float C = sqrt(1.0f - A*A * (1-B*B));
    float Rs = (A*B-C) * (A*B-C) / ((A*B+C) * (A*B+C));
    float Rp = (A*C-B) * (A*C-B) / ((A*C+B) * (A*C+B));
    float alpha = (Rs + Rp) / 2.0f;
	float Kr = min( alpha + 0.20f, 1.0f);
*/

	// フレネル効果の近似計算
    float u = dot(-I, N);
	float f0 = 0.1; //水は0.02だが，波紋が見にくくなる
	float F = f0 + (1.0 - f0) * pow(1.0 - u, 5);
	float Kr = min( F , 1.0f); 

    // 反射環境色の取得
//	vec4 reflecColor = vec4(0, 0, 0, 0);
	vec4 reflecColor = textureCube(envmap, R);
    reflecColor.a = 1.0;

/*
	// ライティングの反射光の計算(透明だから拡散反射なし?)
    vec3 Kd = gl_FrontMaterial.diffuse.xyz;
    vec3 Ld = gl_LightSource[0].diffuse.xyz;
    float diffuseLight = max(dot(IL, N), 0.0);
	vec3 diffuse = Kd*Ld*diffuseLight;
	reflecColor += diffuse;
*/

	// 鏡面反射光の計算（フォンシェーディング）
    vec3 Ls = gl_LightSource[0].specular.xyz;
    vec3 Ks = gl_FrontMaterial.specular.xyz;
    float specularLight = pow(max(dot(RL, N), 0.0), gl_FrontMaterial.shininess);
	reflecColor += vec4(Ks*Ls*specularLight, 1.0);

/*
	// 鏡面反射光の計算（Cook-Torrance）
    vec3 Ls = gl_LightSource[0].specular.xyz;
    vec3 Ks = gl_FrontMaterial.specular.xyz;
    float specularLight = 0.0;
    if(diffuseLight > 0.0)
	{
		vec3 V = normalize(-vPos.xyz);
		vec3 H = normalize(IL+V);
        float NH = dot(N, H);
        float VH = dot(V, H);
        float NV = dot(N, V);
        float NL = dot(N, IL);
        float alpha = acos(NH);
		float m = 0.35f;
        float D = (1.0/(4*m*m*NH*NH*NH*NH))*exp((NH*NH-1)/(m*m*NH*NH));
        float G = min(1, min((2*NH*NV)/VH, (2*NH*NL)/VH));
        float c = VH;
		float refrac = 200.0;
        float g = sqrt(refrac*refrac+c*c-1);
        float FF = ((g-c)*(g-c)/((g+c)*(g+c)))*(1+(c*(g+c)-1)*(c*(g+c)-1)/((c*(g-c)-1)*(c*(g-c)-1)));
        specularLight = D*G*FF/NV;
    }
	reflecColor += vec4(Ks*Ls*specularLight, 1.0);
*/


    // 屈折環境色の計算
    vec4 refracColor = vec4(0.5, 0.5, 0.5, 1);
	vec2 coord = vec2(gl_TexCoord[0].x + T.x * vPosW.z / T.z * x_scale,
					  gl_TexCoord[0].y + T.y * vPosW.z / T.z * y_scale);
	refracColor = texture2D(tex, coord);
    refracColor.a = 1.0;

	// コースティクス（集光効果） -> 太陽の向きほど明るいで近似
	float cD = dot(N, IL);
	float caustics = 0.05 * pow(cD, 10) + 0.1 * pow(cD, 20) + 0.1 * pow(cD, 50) + 0.5 * pow(cD, 100);
	float rC = 0.3; //ファクター
	refracColor = refracColor * (0.9) + vec4(1.0, 1.0, 1.0, 1.0) * caustics * rC; //加算ブレンドのほうが自然？
//	refracColor *= (0.9 + caustics * rC);

	float loss = 0.0; //損失係数（いらない？）
	vec4 color = Kr*reflecColor + (1.0-Kr)*refracColor*(1-loss);

	//for debug
	//vec4 color = vec4(1.0, 1.0, 1.0, 1.0) * caustics;
	//vec4 color = vec4(IL, 1.0);
	//vec4 color = vec4(vec3(pow(cD, 100), 0, 0), 1.0);

//	vec4 color = Kr*reflecColor + (1.0-Kr)*refracColor;

	gl_FragColor = color;
}