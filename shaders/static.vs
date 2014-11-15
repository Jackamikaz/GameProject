#version 410

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTexCoord;

layout (location = 0) out vec3 oPosition;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec2 oTexCoord;

uniform mat4 uViewMatrix;
uniform mat4 uFinalMatrix;

void main()
{
    vec4 pos4 = vec4(iPosition, 1.0);
	oPosition = (uViewMatrix * pos4).xyz;
	oNormal = normalize((uViewMatrix * vec4(iNormal, 0.0)).xyz);
	oTexCoord = iTexCoord;
	gl_Position = uFinalMatrix * pos4;
}