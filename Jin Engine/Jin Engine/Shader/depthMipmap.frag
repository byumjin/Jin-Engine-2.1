#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D prevDepthMap;

layout(set = 0, binding = 1) uniform screenSizeBuffer
{
	vec4 depthMapSize;
};

layout(location = 0) in vec2 fragUV;

layout(location = 0) out float outColor;


void main()
{
	if(depthMapSize.w == 1.0)
	{
		outColor = texture(prevDepthMap, fragUV).x;
	}	
	else
	{
		vec2 prevMapSize = depthMapSize.xy;

		vec2 pixel = prevMapSize * fragUV;

		ivec2 ipixel = ivec2(int(pixel.x), int(pixel.y));
	
		if(ipixel.x % 2 == 1)
		{
			ipixel.x -= 1;
		}

		if(ipixel.y % 2 == 1)
		{
			ipixel.y -= 1;
		}

		float val0 = texture(prevDepthMap, vec2(ipixel) / prevMapSize).x;
		float val1 = texture(prevDepthMap, vec2(ipixel.x + 1, ipixel.y) / prevMapSize).x;
		float val2 = texture(prevDepthMap, vec2(ipixel.x, ipixel.y + 1) / prevMapSize).x;
		float val3 = texture(prevDepthMap, vec2(ipixel.x + 1, ipixel.y + 1) / prevMapSize).x;

		float min01 = min(val0, val1);
		float min23 = min(val2, val3);

		outColor = min(min01, min23);
	}	
}