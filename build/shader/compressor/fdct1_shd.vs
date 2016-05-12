
#version 120

varying vec2 pos;

void main(void)
{
	pos = gl_Vertex.xy / 2.0 + 0.5;
	gl_Position = gl_Vertex;
}
