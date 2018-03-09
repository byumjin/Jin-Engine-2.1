#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_LIGHT 128

layout(binding = 0) uniform sampler2D basicGbuffer;
layout(binding = 1) uniform sampler2D specularGbuffer;
layout(binding = 2) uniform sampler2D normalGbuffere;
layout(binding = 3) uniform sampler2D emissiveGbuffer;
layout(set = 0, binding = 4) uniform sampler2D DepthMap;

layout(set = 0, binding = 5) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
	vec4 viewPortSize;
};

struct LightInfo
{
	vec4 color;
	vec4 direction;
	vec4 worldPos;
};

layout(set = 0, binding = 6) uniform pointLightBuffer
{
	LightInfo pointLight[MAX_LIGHT];
};

layout(set = 0, binding = 7) uniform directionalLightBuffer
{
	LightInfo directionalLight[MAX_LIGHT];
};

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


float PI = 3.1415926535897932384626422832795028841971f;

vec2 LightingFunGGX_FV(float dotLH, float roughness)
{
	float alpha = roughness*roughness;

	//F
	float F_a, F_b;
	float dotLH5 = pow(clamp(1.0f - dotLH, 0.0f, 1.0f), 5.0f);
	F_a = 1.0f;
	F_b = dotLH5;

	//V
	float vis;
	float k = alpha * 0.5f;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	vis = 1.0f/(dotLH*dotLH*invK2 + k2);

	return vec2((F_a - F_b)*vis, F_b*vis);
}

float LightingFuncGGX_D(float dotNH, float roughness)
{
	float alpha = roughness*roughness;
	float alphaSqr = alpha*alpha;
	float denom = dotNH * dotNH * (alphaSqr - 1.0f) + 1.0f;

	return alphaSqr / (PI*denom*denom);
}

vec3 GGX_Spec(vec3 Normal, vec3 HalfVec, float Roughness, vec3 BaseColor, vec3 SpecularColor, vec2 paraFV)
{
	float NoH = clamp(dot(Normal, HalfVec), 0.0f, 1.0f);

	float D = LightingFuncGGX_D(NoH * NoH * NoH * NoH, Roughness);
	vec2 FV_helper = paraFV;

	vec3 F0 = SpecularColor;
	vec3 FV = F0*FV_helper.x + vec3(FV_helper.y, FV_helper.y, FV_helper.y);
	
	return D * FV;
}


vec3 PBR(vec3 LightVec, vec3 HalfVec, vec3 NormalVec, vec3 ReflectVec, vec3 albedo,  vec3 specColor, float Roughness, float Metallic, float energyConservation)
{
	vec3 diffuseTerm = vec3(0.0f);
	vec3 specularTerm = vec3(0.0f);
		
	float LoH = clamp(dot(LightVec, HalfVec), 0.0f, 1.0f);	

	//DIFFUSE
	diffuseTerm = albedo;
				
	//VXGI can't handle metallic reflection because of its distance based contribution
	//diffuseTerm = mix(diffuseTerm, diffuseTerm * GlobalIllumination.xyz / GIintensity, Metallic);

	//SPECULAR
	specularTerm = GGX_Spec(NormalVec, HalfVec, Roughness, diffuseTerm, specColor, LightingFunGGX_FV(LoH, Roughness)) *energyConservation;

	return (diffuseTerm + specularTerm);
}

void main()
{
	vec3 resultColor = vec3(0.0);

    vec4 BasicColorMap = texture(basicGbuffer, fragUV);
	vec4 SpecColorMap = texture(specularGbuffer, fragUV);
    vec4 NormalMap = texture(normalGbuffere, fragUV);
	vec4 EmissiveMap = texture(emissiveGbuffer, fragUV);

	float Roughness = SpecColorMap.w;
	float energyConservation = 1.0f - Roughness;
	float Metallic = NormalMap.w;

	//getPosition form Depth
	float depth = texture(DepthMap, fragUV).x;
	
	//get WorldPosition
	vec4 worldPos = InvViewProjMat * vec4(fragUV.xy * 2.0 - 1.0, depth, 1.0);
	worldPos /= worldPos.w;

	vec3 NormalVec = NormalMap.xyz;

	//Point Light
	for(uint i = 0; i < 9; i++)
	{		
		LightInfo thisPointLight = pointLight[i];

		vec3 LightVec = thisPointLight.worldPos.xyz - worldPos.xyz;

		float distance = length(LightVec);

		LightVec /= distance;

		float radius = thisPointLight.direction.w;
		
		float lightAtt = 1.0 - distance/radius;

		float NoL = dot(LightVec, NormalVec);

		vec3 ViewVec = normalize(cameraWorldPos.xyz - worldPos.xyz); 
		vec3 HalfVec = normalize(ViewVec + LightVec);				

		vec3 ReflectVec = reflect(-ViewVec, NormalVec);

		//Physically-based shader			
		if (NoL > 0.0f && lightAtt > 0.0)
		{
			resultColor += PBR(ViewVec, HalfVec, NormalVec,  ReflectVec, BasicColorMap.xyz, SpecColorMap.xyz, Roughness, Metallic, energyConservation)
			* NoL * thisPointLight.color.xyz * thisPointLight.color.a *  (lightAtt * lightAtt);
		}		
	}	

	//Directional Light
	for(uint i = 0; i < 1; i++)
	{
		LightInfo thisdirectionalLight = directionalLight[i];

		vec3 LightVec = -thisdirectionalLight.direction.xyz;
		float NoL = dot(LightVec, NormalVec);
		vec3 ViewVec = normalize(cameraWorldPos.xyz - worldPos.xyz); 
		vec3 HalfVec = normalize(ViewVec + LightVec);
		vec3 ReflectVec = -reflect(ViewVec, NormalVec);

		//Physically-based shader			
		if (NoL > 0.0f)
		{
			resultColor += PBR(ViewVec, HalfVec, NormalVec,  ReflectVec, BasicColorMap.xyz, SpecColorMap.xyz, Roughness, Metallic, energyConservation)
			* NoL * thisdirectionalLight.color.xyz * thisdirectionalLight.color.a;
		}	
	}

	resultColor += vec3(EmissiveMap.xyz);	
	outColor = vec4(resultColor, depth);

	//outColor = texture(basicGbuffer, fragUV);
    //outColor = (texture(normalGbuffere, fragUV) + vec4(1.0)) * 0.5;
}