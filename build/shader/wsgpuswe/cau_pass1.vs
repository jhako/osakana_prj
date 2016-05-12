
#version 330 core


in vec2 position;
out vec4 vpos;

void main(void)
{
	vpos = vec4(position, 0, 1);
	
	gl_Position = vpos;
}


