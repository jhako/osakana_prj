
#version 400 core


layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;
//layout(line_strip, max_vertices = 4) out;

uniform mat3 normalMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform sampler2D us_tex;
uniform sampler2D nrm_tex;

out vec3 gFacetNormal;
out vec4 vpos;
out vec4 position;
out vec4 view;

void main()
{
	mat4 pmMatrix = projectionMatrix * modelViewMatrix;
	
	vec4 vpos0 = gl_in[0].gl_Position;
	vpos0.z = texture(us_tex, vpos0.xy * 0.5 + vec2(0.5, 0.5)).r;
	vec4 vpos1 = gl_in[1].gl_Position;
	vpos1.z = texture(us_tex, vpos1.xy * 0.5 + vec2(0.5, 0.5)).r;
	vec4 vpos2 = gl_in[2].gl_Position;
	vpos2.z = texture(us_tex, vpos2.xy * 0.5 + vec2(0.5, 0.5)).r;

	float nx, ny;
	
	nx = texture(nrm_tex, vpos0.xy * 0.5 + vec2(0.5, 0.5)).r;
	ny = texture(nrm_tex, vpos0.xy * 0.5 + vec2(0.5, 0.5)).g;
    gFacetNormal = normalize(normalMatrix * vec3(nx, ny, 1.0));
	vpos = vpos0;
	position = pmMatrix * vpos0;
	view = normalize(modelViewMatrix * vpos0);
	gl_Position = position; EmitVertex();

	nx = texture(nrm_tex, vpos1.xy * 0.5 + vec2(0.5, 0.5)).r;
	ny = texture(nrm_tex, vpos1.xy * 0.5 + vec2(0.5, 0.5)).g;
    gFacetNormal = normalize(normalMatrix * vec3(nx, ny, 1.0));
	vpos = vpos1;
	position = pmMatrix * vpos1;
	view = normalize(modelViewMatrix * vpos1);
	gl_Position = position; EmitVertex();

	nx = texture(nrm_tex, vpos2.xy * 0.5 + vec2(0.5, 0.5)).r;
	ny = texture(nrm_tex, vpos2.xy * 0.5 + vec2(0.5, 0.5)).g;
    gFacetNormal = normalize(normalMatrix * vec3(nx, ny, 1.0));
	vpos = vpos2;
	position = pmMatrix * vpos2;
	view = normalize(modelViewMatrix * vpos2);
	gl_Position = position; EmitVertex();
	
	EndPrimitive();
}
