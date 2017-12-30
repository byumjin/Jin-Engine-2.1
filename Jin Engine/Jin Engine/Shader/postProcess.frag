#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


void main()
{
	outColor = texture(SceneTexture, fragUV);   
}