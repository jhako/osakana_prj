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
	//���_�ʒu�C���_�@���̐ݒ�
    vPos = gl_ModelViewMatrix * gl_Vertex;
    vNrm = normalize(gl_NormalMatrix*gl_Normal);
 
	//���_�̈ʒu�̐ݒ�
//	eye = (gl_ModelViewMatrix * vec4(eyePosition, 1.0)).xyz;
	
	//���[���h���W���L�^
	vPosW = gl_Vertex.xyz;
//	vNrmW = normalize(gl_Normal);
//	ePosW = eyePosition;
 
    // �`�撸�_�ʒu
    gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
