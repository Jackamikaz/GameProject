#version 410

layout (location = 0) in vec3 iPosition;
layout (location = 3) in ivec4 iBoneIDs;
layout (location = 4) in vec4 iBoneWeights;

layout (location = 0) out vec3 oPosition;

uniform mat4 uViewMatrix;
uniform mat4 uFinalMatrix;
uniform mat4 uBones[32];

void main()
{
    mat4 BoneTransform = uBones[iBoneIDs[0]] * iBoneWeights[0];
    BoneTransform     += uBones[iBoneIDs[1]] * iBoneWeights[1];
    BoneTransform     += uBones[iBoneIDs[2]] * iBoneWeights[2];
    BoneTransform     += uBones[iBoneIDs[3]] * iBoneWeights[3];
	
    vec4 pos4 = BoneTransform * vec4(iPosition, 1.0);
	oPosition = (uViewMatrix * pos4).xyz;
	gl_Position = uFinalMatrix * pos4;
}