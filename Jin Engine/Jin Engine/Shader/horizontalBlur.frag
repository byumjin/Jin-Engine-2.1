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

//float offset[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
//float weight[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

void main()
{	
	outColor = vec4(0.0);
	outColor = texture( SceneTexture, vec2(fragUV)/viewPortSize.xy ) * weight[0];
    for (int i=1; i<3; i++)
	{
        outColor += texture( SceneTexture, ( vec2(fragUV)+vec2( offset[i] / viewPortSize.x, 0.0))) * weight[i];
        outColor += texture( SceneTexture, ( vec2(fragUV)-vec2( offset[i] / viewPortSize.x, 0.0))) * weight[i];
    }

	outColor = clamp(outColor, 0.0, 1.0);
}