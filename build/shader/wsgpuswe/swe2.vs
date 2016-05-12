
#version 120
 
varying vec2 vpos;

void main(void)
{
	vpos = gl_Vertex.xy;
	gl_Position = gl_Vertex;
}
