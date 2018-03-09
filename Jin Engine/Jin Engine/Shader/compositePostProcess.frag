#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;
layout(binding = 1) uniform sampler2D SSRTexture;

layout(set = 0, binding = 2) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
	vec4 viewPortSize;
};


layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


void main()
{	
	vec4 SeneColor = texture(SceneTexture, fragUV);	
	vec4 SSR = texture(SSRTexture, fragUV);	
	
	/*
	if(SSR.w > 0.0)
		outColor = mix(SeneColor, SSR, SSR.w);
	else
		outColor = texture(SceneTexture, fragUV);
	*/
	
	outColor = SeneColor + SSR;
	
}