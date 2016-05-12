

#version 400 core


uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

layout(location = 0) in vec2 position;

out vec3 vPosition;


void main(void)
{
	vec4 vpos = vec4(position, 0, 1);
	
	gl_Position = vpos;
	
	vPosition = vpos.xyz;
}
