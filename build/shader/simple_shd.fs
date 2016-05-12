

#version 120
 
uniform sampler2D tex;

void main(void)
{
	gl_FragColor = texture2DProj(tex, gl_TexCoord[0]);
}