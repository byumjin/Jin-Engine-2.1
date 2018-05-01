#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;
layout(binding = 1) uniform sampler2D SSRTexture;

layout(set = 0, binding = 2) uniform SSRmodeBuffer
{
	uint SSR_MODE;
};

layout(set = 0, binding = 3) uniform cameraBuffer
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
	
	if(SSR_MODE == 0)
	{
		outColor = SeneColor +  SSR * SSR.w;
	}
	else if(SSR_MODE == 1)
	{
		if(SSR.w > 0.0)
			outColor = SSR;
		else
			outColor = texture(SceneTexture, fragUV);
	}
	else if(SSR_MODE == 2)
	{
		outColor = SSR;
	}	
}