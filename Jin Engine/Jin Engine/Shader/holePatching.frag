#version 450
#extension GL_ARB_separate_shader_objects : enable

#define EXPENSIVE_PATCHING 0
#define FLT_MAX  3.402823466e+38F

layout(binding = 0) uniform sampler2D SceneTexture;

layout(set = 0, binding = 1) uniform SSRInfoBuffer
{
	vec4 SSRInfo; //x : global Roughness, y : Intensity, z : bUseNormalmap, w : holePatching
};

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
	vec4 thisColor = texture(SceneTexture, fragUV);
	outColor = thisColor;

	
	
	if(SSRInfo.w < 0.5)
	{
		outColor.w = 1.0;
		return;
	}

	if(thisColor.w > 0.0)
	{
		float threshold = thisColor.w;
		float minOffset = threshold;
		

		vec4 neighborColor00 = texture(SceneTexture, fragUV + vec2(1.0/viewPortSize.x, 0.0));
		if(neighborColor00.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor00.w);			
		}

		vec4 neighborColor01 = texture(SceneTexture, fragUV - vec2(1.0/viewPortSize.x, 0.0));
		if(neighborColor01.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor01.w);			
		}

		vec4 neighborColor02 = texture(SceneTexture, fragUV + vec2(0.0, 1.0/viewPortSize.y));
		if(neighborColor02.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor02.w);			
		}

		vec4 neighborColor03 = texture(SceneTexture, fragUV - vec2(0.0, 1.0/viewPortSize.y));
		if(neighborColor03.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor03.w);			
		}

#if EXPENSIVE_PATCHING
		vec4 neighborColor04 = texture(SceneTexture, fragUV + vec2(1.0/viewPortSize.x, 1.0/viewPortSize.y));
		if(neighborColor04.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor04.w);			
		}

		vec4 neighborColor05 = texture(SceneTexture, fragUV + vec2(1.0/viewPortSize.x, -1.0/viewPortSize.y));
		if(neighborColor05.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor05.w);			
		}

		vec4 neighborColor06 = texture(SceneTexture, fragUV + vec2(-1.0/viewPortSize.x, 1.0/viewPortSize.y));
		if(neighborColor06.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor06.w);			
		}

		vec4 neighborColor07 = texture(SceneTexture, fragUV - vec2(1.0/viewPortSize.x, 1.0/viewPortSize.y));
		if(neighborColor07.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor07.w);			
		}
#endif

		if(minOffset == neighborColor00.w)
		{
				outColor = neighborColor00;
		}
		else if(minOffset == neighborColor01.w)
		{
				outColor = neighborColor01;
		}
		else if(minOffset == neighborColor02.w)
		{
				outColor = neighborColor02;
		}
		else if(minOffset == neighborColor03.w)
		{
				outColor = neighborColor03;
		}
#if EXPENSIVE_PATCHING
		else if(minOffset == neighborColor04.w)
		{
				outColor = neighborColor04;
		}
		else if(minOffset == neighborColor05.w)
		{
				outColor = neighborColor05;
		}
		else if(minOffset == neighborColor06.w)
		{
				outColor = neighborColor06;
		}
		else if(minOffset == neighborColor07.w)
		{
				outColor = neighborColor07;
		}
#endif
		
		if(minOffset <= threshold)		
			outColor.w = SSRInfo.y;
		else
			outColor.w = 0.0;
	}
}