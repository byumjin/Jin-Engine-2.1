#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;

layout(set = 0, binding = 1) uniform cameraBuffer
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
	outColor = vec4(0.0);
	
	vec4 thisColor = texture(SceneTexture, fragUV);

	if(thisColor.w == 0.0)
		return;
	else if(thisColor.w == 1.0)
	{
		outColor = thisColor;
		//outColor = vec4(0.0, 1.0, 1.0, 1.0);
	}
	else if(thisColor.w == 2.0)
	{
		uint counter = 0;

		vec4 neighborColor00 = texture(SceneTexture, fragUV + vec2(1.0/viewPortSize.x, 0.0));
		if(neighborColor00.w == 1.0)
		{
			outColor += neighborColor00;
			counter++;
		}
		
		vec4 neighborColor01 = texture(SceneTexture, fragUV - vec2(1.0/viewPortSize.x, 0.0));
		if(neighborColor01.w == 1.0)
		{
			outColor += neighborColor01;
			counter++;
		}

		vec4 neighborColor02 = texture(SceneTexture, fragUV + vec2(0.0, 1.0/viewPortSize.y));
		if(neighborColor02.w == 1.0)
		{
			outColor += neighborColor02;
			counter++;
		}
		
		vec4 neighborColor03 = texture(SceneTexture, fragUV - vec2(0.0, 1.0/viewPortSize.y));
		if(neighborColor03.w == 1.0)
		{
			outColor += neighborColor03;
			counter++;
		}

		if(counter > 0)
		{
			outColor /= float(counter);
			outColor.w = 1.0;
		}
	}
}