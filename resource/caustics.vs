
#version 120

const float etaRatio = 1.0 / 1.333; //air->water

varying vec3 o_vpos;
varying vec3 n_vpos;

void main(void)
{
	//頂点位置，頂点法線の設定
    vec4 vPos = gl_Vertex;
    vec3 N = normalize(gl_NormalMatrix * gl_Normal);

	vec3 IL = normalize((gl_LightSource[0].position * vPos.w - gl_LightSource[0].position.w * vPos).xyz);
    vec3 IT = refract(IL, N, etaRatio);    // 屈折ベクトル
	vec4 newPos = vec4(vPos.x + IT.x * vPos.z / IT.z, vPos.y + IT.y * vPos.z / IT.z, 0.0, 1.0);

	o_vpos = vPos.xyz;
	n_vpos = newPos.xyz;
 
    // 描画頂点位置
	vec4 ftrnpos = gl_ModelViewProjectionMatrix * newPos;
	gl_Position = vec4(ftrnpos.x, ftrnpos.y, 0, 1);
}