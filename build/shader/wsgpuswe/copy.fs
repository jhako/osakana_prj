
#version 330

uniform sampler2D src;
out vec4 FragColor;

void main(void)
{
	ivec2 tpos = ivec2(gl_FragCoord.xy);
	vec4 color = texelFetch(src, tpos, 0);
	FragColor = color;
}
