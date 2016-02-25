
#version 120

const float etaRatio = 1.0 / 1.333; //air->water

uniform float width;
uniform float height;
varying vec3 o_vpos;
varying vec3 n_vpos;

void main(void)
{
	//頂点位置，頂点法線の設定
    vec4 vPos = gl_Vertex;
    vec3 N = normalize(gl_NormalMatrix * gl_Normal);

	vec3 IL = normalize((gl_LightSource[0].position * vPos.w - gl_LightSource[0].position.w * vPos).xyz);
    vec3 IT = refract(IL, N, etaRatio);    // 屈折ベクトル
	vec4 newPos;
	if(IT.z < -0.00000001)
	{
		//画面内に収める処理
		vec2 ray = vec2(IT.x * vPos.z / IT.z, IT.y * vPos.z / IT.z);
		vec2 tA = (vec2(0, 0) - vPos.xy) / ray;
		vec2 tB = (vec2(width, height) - vPos.xy) / ray;
		vec2 tP = max(tA, tB);
		float t = min(1.0, min(tP.x, tP.y));
		newPos = vec4(vPos.xy + ray*t, 0.0, 1.0);
	}
	else
	{
		newPos = vPos;
	}

	o_vpos = vPos.xyz;
	n_vpos = newPos.xyz;
 
    // 描画頂点位置
	vec4 ftrnpos = gl_ModelViewProjectionMatrix * newPos;
	gl_Position = vec4(ftrnpos.x, ftrnpos.y, 0, 1);
	//gl_Position = ftransform();
}