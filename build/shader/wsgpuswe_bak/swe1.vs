
#version 400 core
 
varying vec2 vpos;

layout(location = 0) in vec2 position;

void main(void)
{
	//vpos = gl_Vertex.xy;
	vpos = position;
	gl_Position = vec4(position, 0, 1);
}
