#version 120

varying vec4 vPos;
varying vec3 vNrm;
varying vec3 eye;

varying vec3 vPosW;
//varying vec3 vNrmW;
//varying vec3 ePosW;


//uniform vec3 eyePosition;
 
void main(void)
{
	//頂点位置，頂点法線の設定
    vPos = gl_ModelViewMatrix * gl_Vertex;
    vNrm = normalize(gl_NormalMatrix*gl_Normal);
 
	//視点の位置の設定
//	eye = (gl_ModelViewMatrix * vec4(eyePosition, 1.0)).xyz;
	
	//ワールド座標を記録
	vPosW = gl_Vertex.xyz;
//	vNrmW = normalize(gl_Normal);
//	ePosW = eyePosition;
 
    // 描画頂点位置
    gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
