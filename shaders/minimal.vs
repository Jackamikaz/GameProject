#version 410

layout (location = 0) in vec3 iPosition;

layout (location = 0) out vec3 oPosition;

uniform mat4 uViewMatrix;
uniform mat4 uFinalMatrix;

void main()
{
    vec4 pos4 = vec4(iPosition, 1.0);
	oPosition = (uViewMatrix * pos4).xyz;
	gl_Position = uFinalMatrix * pos4;
}