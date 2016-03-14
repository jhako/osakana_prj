
#version 120

varying vec3 o_vpos;
varying vec3 n_vpos;
varying vec3 N;


void main(void)
{
	float oldS = length(dFdx(o_vpos)) * length(dFdy(o_vpos));
	float newS = length(dFdx(n_vpos)) * length(dFdy(n_vpos));
	gl_FragData[0] = vec4(0.2, 0.2, 0.2, 1.0) * oldS / newS;
	//gl_FragData[0] = vec4(1, 1, 1, 1.0);	
}
