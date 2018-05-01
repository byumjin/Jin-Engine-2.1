#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec4 vertexCol;
layout(location = 2) in vec4 vertexTan;
layout(location = 3) in vec4 vertexBitan;
layout(location = 4) in vec4 vertexNor;
layout(location = 5) in vec2 vertexUV;

layout(location = 0) out vec2 fragUV;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
	gl_Position = vertexPos;
	fragUV = vertexUV;
}