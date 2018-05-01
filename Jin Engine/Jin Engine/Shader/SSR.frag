#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;

layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D specularMap;
layout(binding = 3) uniform sampler2D normalMap;

layout(binding = 4) uniform sampler2D depthTexture;

#define MAX_PLANES 4

layout(set = 0, binding = 5) uniform SSRInfoBuffer
{
	vec4 SSRInfo; //x : global Roughness, y : Intensity, z : bUseNormalmap, w : holePatching
};


layout(set = 0, binding = 6) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
	vec4 viewPortSize;
};

struct PlaneInfo
{
	mat4 rotMat;
	vec4 centerPoint;
	vec4 size;
};

layout(set = 0, binding = 7) uniform planeInfoBuffer
{	
	PlaneInfo planeInfo[MAX_PLANES];
	uint numPlanes;
	uint pad00;
	uint pad01;
	uint pad02;
};

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;



float depthLinear(float depth)
{	
	float f = 1000.0;
	float n = 0.1;
	return (2.0 * n) / (f + n - depth * (f - n));
}

vec4 getWorldPosition(vec2 UV, float depth)
{
	vec4 worldPos = InvViewProjMat * vec4(UV * 2.0 - 1.0, depth, 1.0);
	worldPos /= worldPos.w;

	return worldPos;
}

float fade(vec2 UV)
{
	vec2 NDC = vec2(UV.x * 2.0 - 1.0,  UV.y * 2.0 - 1.0);

	return clamp( 1.0 - max( pow( NDC.y * NDC.y, 4.0) , pow( NDC.x * NDC.x, 4.0)) , 0.0, 1.0); 
}

float getDistance(vec3 planeNormal, vec3 planeCenter, vec3 worldPos)
{
	//plane to point
	float d = -dot(planeNormal, planeCenter);
	return (dot(planeNormal, worldPos) + d) / length(planeNormal);
}

vec3 getNormalVector(vec2 surfaceUV)
{
	return normalize( texture(normalMap, surfaceUV).xyz * 2.0 - vec3(1.0) );
}

bool intersectPlane(in uint index, in vec3 worldPos, in vec2 fragUV, out vec3 normalVec, out vec4 hitPos, out vec2 relfectedUVonPlanar, bool bUseNormalMap) 
{ 
	PlaneInfo thisPlane = planeInfo[index];

	// assuming vectors are all normalized
	//vec3 normalVec;	
	normalVec = thisPlane.rotMat[2].xyz;
	
	vec3 centerPoint = thisPlane.centerPoint.xyz;

	vec3 rO = cameraWorldPos.xyz;
	vec3 rD = normalize(worldPos - rO);

	vec3 rD_VS = mat3(viewMat) * rD;

	
	if(rD_VS.z > 0.0)
	{
		return false;
	}
	

    float denom = dot(normalVec, rD); 

    if (denom < 0.0)
	{ 
        vec3 p0l0 = centerPoint - rO; 
        float t = dot(normalVec, p0l0) / denom; 

		if(t <= 0.0)
		{
			return false;
		}

		vec3 hitPoint = rO + rD*t;	

		vec3 gap = hitPoint - centerPoint;
		
		float xGap = dot(gap, thisPlane.rotMat[0].xyz);
		float yGap = dot(gap, thisPlane.rotMat[1].xyz);

		float width = thisPlane.size.x * 0.5;
		float height = thisPlane.size.y * 0.5;

		vec4 reflectedPos;

		if( (abs(xGap) <= width) && (abs(yGap) <= height))
		{
			hitPos = vec4(hitPoint, 1.0);
			reflectedPos = viewProjMat * hitPos;
			reflectedPos /= reflectedPos.w;

			reflectedPos.xy = (reflectedPos.xy + vec2(1.0)) * 0.5;

			float depth = texture(depthTexture, reflectedPos.xy).x;	

			if(depth <= reflectedPos.z)
				return false;
			
			if( reflectedPos.x < 0.0 || reflectedPos.y < 0.0  || reflectedPos.x > 1.0 || reflectedPos.y > 1.0 )
			{
				return false;
			}
			else
			{
				relfectedUVonPlanar = vec2(xGap / width, yGap / height) * 0.5 + vec2(0.5);
				relfectedUVonPlanar *= vec2(thisPlane.size.zw);

				if(bUseNormalMap)
				{
					normalVec =  mat3(thisPlane.rotMat) * getNormalVector(relfectedUVonPlanar);
				}

				return true; 
			}			
		}	
		else
		{
			return false;
		}
    } 
	else
	{
		return false; 
	}
} 

void main()
{	
	float depth = texture(depthTexture, fragUV).r;
	
	if(depth >= 1.0)
	{
		outColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	vec4 worldPos = getWorldPosition(fragUV, depth);

	

	vec3 currentPos = worldPos.xyz;
	vec3 prevPos = currentPos;
	vec4 reflectionColor = vec4(0.0, 0.0, 0.0, 0.0);

	float threshold = 2.0;
	
	float prevDepth;
	float prevDepthFromDepthBuffer;

	bool bHit = false;
	float fadeFactor = 0.0;

	//float maxStep = 2048.0;
	//float stepSize = 0.1;
	
	float maxStep = 128.0;
	float stepSize = 1.0;
	
	float Intensity = 1.0;

	bool bIsInterect = false;
	bool bUseInterpolation = SSRInfo.w > 0.5 ? true : false;

	uint relfectedPlanarIndex;
	vec4 hitpoint;
	vec2 UVforNormalMap;
	vec3 WorldNormal;

	bool bUseNormalMap = SSRInfo.z > 0.5 ? true : false;

	for(uint i = 0; i < numPlanes; i++)
	{	
		if(!intersectPlane( i, worldPos.xyz, fragUV, WorldNormal, hitpoint, UVforNormalMap, bUseNormalMap))
		{
			//continue;			
		}
		else
		{
			bIsInterect = true;
			break;
			/*
			float localDist =  distance(hitpoint, cameraWorldPos.xyz);
			
			if( localDist <  minDist )
			{
				minDist = localDist;
			}
			*/
		}

		relfectedPlanarIndex = i;
	}

	//bIsInterect = true;

	UVforNormalMap *= 0.4;// SSRInfo.w;

	if(!bIsInterect)
	{
		outColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	
	
	//if(bUseNormalMap)
	//	WorldNormal = getNormalVector(UVforNormalMap);
	

	vec3 viewVec = normalize(worldPos.xyz - cameraWorldPos.xyz);
	vec3 relfectVec = reflect(viewVec , WorldNormal);


	//rayMarching
	for(float i = 0.0; i < maxStep; i++ )
	{			
		currentPos += relfectVec * stepSize;

		vec4 pos_SS = viewProjMat * vec4(currentPos, 1.0);
		pos_SS /= pos_SS.w;
		vec2 screenSpaceCoords = vec2((pos_SS.x + 1.0) * 0.5, (pos_SS.y + 1.0 )*0.5);

		

		if(screenSpaceCoords.x > 1.0 || screenSpaceCoords.x < 0.0 || screenSpaceCoords.y > 1.0 || screenSpaceCoords.y < 0.0 || pos_SS.z >= 1.0)
		{
			fadeFactor = 0.0;
			//bHit = true;

			//outColor = vec4(0.0, 1.0, 0.0, 0.0);
			//return;
			break;
		}
		
		float depth_SS = texture(depthTexture, screenSpaceCoords).r;

		

		
		if(pos_SS.z > depth_SS)
		{				
			float currentLinearDepth = depthLinear(depth_SS);
			vec4 cworldPos = getWorldPosition( screenSpaceCoords, depth_SS);


			float currentIndicatedLinearDepth = depthLinear(pos_SS.z);

			if( distance(cworldPos.xyz, currentPos) < stepSize * threshold)
			{
				

				float prevIndicatedLinearDepth = depthLinear(prevDepth);
				float prevLinearDepth = depthLinear(prevDepthFromDepthBuffer);
				
				float denom = ( (currentLinearDepth - prevLinearDepth) - (currentIndicatedLinearDepth - prevIndicatedLinearDepth) );

				if(denom == 0.0)
				{
					reflectionColor = vec4(0.0, 0.0, 0.0, 0.0);

					fadeFactor = 0.0;
					bHit = true;
					//outColor = vec4(1.0, 0.0, 0.0, 0.0);
					//return;
					break;
				}

				float lerpVal = (prevIndicatedLinearDepth - prevLinearDepth) / denom;
				

				//exception
				if(i < 0.5)
					lerpVal = 1.0;

				vec3 lerpedPos;

				if(bUseInterpolation)
				{
					lerpedPos = prevPos + relfectVec * stepSize * lerpVal;
				}
				else
				{
					lerpedPos = currentPos;
				}

				//lerpedPos = prevPos.xyz;

				vec4 lerpedPos_SS =  viewProjMat * vec4(lerpedPos, 1.0);
				lerpedPos_SS /= lerpedPos_SS.w;
				
				vec2 lerpedScreenSpaceCoords = vec2((lerpedPos_SS.x + 1.0) * 0.5, (lerpedPos_SS.y + 1.0) * 0.5);

				//out of screen
				if(lerpedScreenSpaceCoords.x > 1.0 || lerpedScreenSpaceCoords.x < 0.0 || lerpedScreenSpaceCoords.y > 1.0 || lerpedScreenSpaceCoords.y < 0.0 || lerpedPos_SS.z >= 1.0)
				{
					reflectionColor = vec4(0.0, 0.0, 0.0, 0.0);

					fadeFactor = 0.0;
					bHit = true;
					//outColor = vec4(0.0, 0.0, 1.0, 0.0);
					//return;
					break;
				}

				//reflection with backface
				/*
				if( dot(relfectVec, texture(u_Gbuffer_Specular, lerpedScreenSpaceCoords).xyz) > 0.0 || dot(relfectVec, -viewVec ) > 0.0 )
				{					

					reflectionColor = vec4(0.0, 0.0, 0.0, 0.0);

					fadeFactor = 0.0;
					bHit = true;
					break;
				}
				*/
				reflectionColor = texture(SceneTexture, lerpedScreenSpaceCoords);

				fadeFactor = 1.0;// fade(lerpedScreenSpaceCoords);
				fadeFactor = min(pow(1.0 -  (i + 1.0)/maxStep, 0.05), fadeFactor);
				bHit = true;

				break;
			}
			/*
			else
			{
				fadeFactor = 0.0;
				bHit = true;
				break;
			}
			*/
		}

		prevDepthFromDepthBuffer = depth_SS;
		prevDepth = pos_SS.z;
		prevPos = currentPos;

	}
 	

	fadeFactor = fadeFactor * fadeFactor;

	//We are not going to make inner pool scene. Thus, this is fine
	

	//float energyConservation = 1.0 - roughness * roughness;

	outColor = reflectionColor * Intensity;// * energyConservation;
	outColor.xyz = mix(vec3(0.0), outColor.xyz, fadeFactor);
	outColor = clamp(outColor, 0.0, 1.0);

	outColor.w = 1.0; //SSR_Mask
	
}
