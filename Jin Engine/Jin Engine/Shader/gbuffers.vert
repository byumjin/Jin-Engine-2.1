#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(set = 0, binding = 4) uniform objectBuffer
{
	mat4 modelMat;
	mat4 InvTransposeMat;
};

layout(set = 0, binding = 5) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
};

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec4 vertexCol;
layout(location = 2) in vec4 vertexTan;
layout(location = 3) in vec4 vertexBitan;
layout(location = 4) in vec4 vertexNor;
layout(location = 5) in vec2 vertexUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec3 fragBiTangent;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec2 fragUV;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
    gl_Position = viewProjMat * modelMat *  vec4(vertexPos.xyz, 1.0);

    fragColor = vertexCol.xyz;

	fragTangent = normalize(vertexTan.xyz);
	fragBiTangent = normalize(vertexBitan.xyz);
	fragNormal = normalize(vertexNor.xyz);

	fragUV = vertexUV;
}