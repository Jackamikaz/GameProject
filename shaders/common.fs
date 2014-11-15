#version 410

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTexCoord;

out vec4 FragColor;

uniform sampler2D uSampler0;
uniform vec3 uDirectionalLight;
uniform float uAmbientFactor;

void main()
{                                    
    FragColor = texture2D(uSampler0,iTexCoord);
	if (dot(iNormal,-uDirectionalLight) < 0) {
		FragColor *= uAmbientFactor;
	}
	FragColor.a = 1.0f;
}