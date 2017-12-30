#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main()
 {
	vec4 sceneColor = texture(SceneTexture, fragUV);

	if(sceneColor.w > 0.0)
		outColor = sceneColor;	
	else // Sky
	{
		outColor = vec4(0.5, 1.0, 1.0, 1.0);	
	}
		
}