#version 450
#extension GL_ARB_separate_shader_objects : enable

#define UINT_MAX 4294967295
#define FLT_MAX  3.402823466e+38F
#define MAX_PLANES 4

#define MAX_SCREEN_WIDTH 1920
#define MAX_SCREEN_HEIGHT 1080

#define NUM_SAMPLE 4

#define PI 3.1415926535897932384626422832795028841971
#define RADIAN 0.01745329251994329576923690768489

layout(binding = 0) uniform sampler2D SceneTexture;
layout(binding = 1, r32ui) uniform uimage2D IntermediateBuffer;
layout(binding = 2) uniform sampler2D albedoMap;
layout(binding = 3) uniform sampler2D specularMap;
layout(binding = 4) uniform sampler2D normalMap;
layout(binding = 5) uniform sampler2D NoiseMap;
layout(binding = 6) uniform sampler2D depthMap;

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

layout(set = 0, binding = 8) uniform SSRInfoBuffer
{
	vec4 SSRInfo; //x : global Roughness, y : Intensity, z : bUseNormalmap, w : holePatching
};

layout(set = 0, binding = 9) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
	vec4 viewPortSize;
};

layout(location = 0) in vec2 fragUV;

mat2 rotationMat2(float angle)
{
	float s = sin(angle);
    float c = cos(angle);

	return mat2(c, -s,
				s,  c);
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec4 getWorldPosition(vec2 UV, float depth)
{
	vec4 worldPos = InvViewProjMat * vec4(UV * 2.0 - 1.0, depth, 1.0);
	worldPos /= worldPos.w;

	return worldPos;
}

bool intersectPlane(in uint index, in vec3 worldPos, in vec2 fragUV, out vec4 hitPos, out vec2 relfectedUVonPlanar) 
{ 
	PlaneInfo thisPlane = planeInfo[index];

	// assuming vectors are all normalized
	vec3 normalVec;	
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

			float depth = texture(depthMap, reflectedPos.xy).x;	

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

vec3 getNormalVector(vec2 surfaceUV)
{
	return normalize( texture(normalMap, surfaceUV).xyz * 2.0 - vec3(1.0) );
}

vec3 GGXDistribution_Sample_wh(vec2 xi, float roughness)
{	
    vec3 wh;
	float phi = (2.0 * PI) * xi.y;
	   
	float tanTheta2;
	float cosTheta;

    tanTheta2 = roughness * roughness * xi.x / (1.0 - xi.x);
    cosTheta = 1.0 / sqrt(1.0 + tanTheta2);
   
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    wh = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    return wh;
}

//wo -> viewvector in tangentSpace
vec3 ImportanceSampleGGX(vec3 normal, vec3 worldView, vec2 xi, float roughness, mat3 rotationMat)
{
	vec3 wh = normalize(GGXDistribution_Sample_wh(xi, roughness)); //tangent_Space

	vec3 tangentNormal = vec3(0.0, 0.0, 1.0);

	if(tangentNormal == normal || tangentNormal == -normal)
	{
		vec3 worldHalf = rotationMat * wh;	//tan - > world
		worldHalf = normalize(worldHalf);

		//world reflection Vector
		return normalize(2.0 * dot(worldView, worldHalf)*worldHalf - worldView);
	}
	else
	{
		float dotValue = dot( vec3(0.0, 0.0, 1.0), normal);
	
		float angle = acos( dotValue );
		
		vec3 bitangentVec = normalize(cross(tangentNormal, normal));
				
		wh = mat3(rotationMatrix(bitangentVec, angle)) * wh;	
		vec3 worldHalf = rotationMat * wh; //tan - > world
		worldHalf = normalize(worldHalf);

		//world reflection Vector
		return normalize(2.0 * dot(worldView, worldHalf)*worldHalf - worldView);
		
	}
	
}

vec4 getColorwithNormalMap(vec3 worldPos, vec3 normalVec, float roughness, vec3 reflectedWorldPos, mat3 rotationMat, vec2 Xi)
{
	//Approximation, not exact	
	
	vec3 viewVec_WS =  normalize(cameraWorldPos.xyz - worldPos);
	vec3 reflecVec_WS = ImportanceSampleGGX(normalVec, viewVec_WS, Xi, roughness, rotationMat );
	
	float reflectedDistance = distance(reflectedWorldPos.xyz, worldPos);
		
	vec4 reflectedPos_SS = viewProjMat * vec4(worldPos + (reflecVec_WS * reflectedDistance), 1.0);
	reflectedPos_SS /= reflectedPos_SS.w;
	reflectedPos_SS.xy = (reflectedPos_SS.xy + vec2(1.0)) * 0.5;

	return texture(SceneTexture,  reflectedPos_SS.xy);
}

vec4 unPacked(in uint unpacedInfo, in vec2 dividedViewSize, out uint CoordSys)
{
	float YInt = float(unpacedInfo >> 20);
	int YFrac = int( (unpacedInfo & 0x000E0000) >> 17 );
	
	uint uXInt = (unpacedInfo & 0x00010000) >> 16;

	float XInt = 0.0;

	if(uXInt == 0)
	{
		XInt = float( int(  (unpacedInfo & 0x0001FFE0) >> 5 ));
	}
	else
	{
		XInt = float(int( ((unpacedInfo & 0x0001FFE0) >> 5) | 0xFFFFF000));
	}
	
	int XFrac = int( (unpacedInfo & 0x0000001C) >> 2 );

	float Yfrac = YFrac * 0.125;
	float Xfrac = XFrac * 0.125;
	
	CoordSys = unpacedInfo & 0x00000003;

	vec2 offset = vec2(0.0);

	if(CoordSys == 0)
	{
		offset = vec2( (XInt) / dividedViewSize.x, (YInt)  / dividedViewSize.y);
		//offset = vec2(XInt, YInt);
	}
	else if(CoordSys == 1)
	{
		offset = vec2( (YInt) / dividedViewSize.x, (XInt) / dividedViewSize.y);
		//offset = vec2(0.0, 1.0);
	}
	else if(CoordSys == 2)
	{
		offset = vec2( (XInt) / dividedViewSize.x, -(YInt) / dividedViewSize.y);
		//offset = vec2(0.5, 0.5);
	}
	else if(CoordSys == 3)
	{
		offset = vec2( -(YInt) / dividedViewSize.x, (XInt) / dividedViewSize.y);
		//offset = vec2(1.0, 1.0);
	}

	return vec4(offset, Xfrac, Yfrac);
}


vec4 getColorwithNormal(vec3 worldPos, vec3 normalVec, float globalRoughness, vec3 reflectedWorldPos, mat3 rotMat, vec2 samples[NUM_SAMPLE])
{
	
#if NUM_SAMPLE 
			vec4 getColor = vec4(0.0);

			int validSampleCounter = 0;

			
			for(int i=0; i < NUM_SAMPLE; i++)
			{
				vec4 reflectedColor = getColorwithNormalMap(worldPos.xyz, normalVec, globalRoughness, reflectedWorldPos.xyz, rotMat, samples[i]);	
				getColor += clamp(reflectedColor, 0.0, 1.0);				
			}
			
			return getColor / float(NUM_SAMPLE);
#else			
			return getColorwithNormalMap(worldPos.xyz, normalVec, globalRoughness, reflectedWorldPos.xyz, rotMat, samples[0]);
			
#endif
}


float fade(vec2 UV)
{
	vec2 NDC = UV * 2.0 - vec2(1.0);

	return clamp( 1.0 - max( pow( NDC.y * NDC.y, 4.0) , pow( NDC.x * NDC.x, 4.0)) , 0.0, 1.0); 
}


vec4 fetchColor(vec2 relfectedUV, vec2 UVforNormalMap, vec3 HitPos_WS, float Roughness, mat3 rotMat, vec2 samples[NUM_SAMPLE], bool bUseNormal)
{
	float reflectedDepth = texture(depthMap, relfectedUV).x;
	vec4 reflectedWorldPos = getWorldPosition(relfectedUV, reflectedDepth);
	vec3 normalVec;

	if(bUseNormal)
	{
		normalVec = getNormalVector(UVforNormalMap);
		return getColorwithNormal(HitPos_WS, normalVec, Roughness, reflectedWorldPos.xyz, rotMat, samples);
	}
	else
	{
		//return texture(SceneTexture,  relfectedUV);
		

		normalVec = vec3(0.0, 0.0, 1.0);
		return getColorwithNormal(HitPos_WS, normalVec, Roughness, reflectedWorldPos.xyz, rotMat, samples);
	}
	
}


layout(location = 0) out vec4 outColor;

void main()
{
	outColor =  vec4(0.0);

	ivec2 iUV = ivec2(fragUV.x * viewPortSize.x, fragUV.y * viewPortSize.y);
	
	uint bufferInfo = imageAtomicAdd(IntermediateBuffer, iUV, 0);

	bool bIsInterect = false;

	/*
	if(UINT_MAX > bufferInfo)
	{
		bIsInterect = true;
	}
	*/

	uint CoordSys;
	vec2 offset = unPacked(bufferInfo, viewPortSize.xy, CoordSys).xy;
	
	
	//offset = vec2(0.0);
	//outColor = vec4(  abs(offset) * 2000.0, 1.0, 1.0);
	
	//return;
	

	float depth = texture(depthMap, fragUV).x;

	vec4 worldPos = getWorldPosition(fragUV, depth);

	vec4 HitPos_WS;
	vec2 UVforNormalMap;

	
	bool bUseNormal = false;// SSRInfo.z > 0.5 ? true : false;

	//if(bUseNormal)
	{
		for(uint i = 0; i < numPlanes; i++)
		{	
			if(intersectPlane( i, worldPos.xyz, fragUV, HitPos_WS, UVforNormalMap))
			{
				bIsInterect = true;
				break;		
			}
			else
			{
				UVforNormalMap = vec2(0.0);
			}
		}
	}
	

	//outColor = vec4(UVforNormalMap, 0.0, 1.0);
	//return;
	
	//If is not in the boundary of planar, exit
	if(!bIsInterect)
	{
		//clear IntermediateBuffer
		imageAtomicMax(IntermediateBuffer, iUV, UINT_MAX);
		return;
	}

	
	

	vec4 noiseColor = texture(NoiseMap, (viewPortSize.xy / vec2(1024.0)) * fragUV);
	vec2 Xi = fract(noiseColor.xy);

	vec2 samples[NUM_SAMPLE];

	float angleBias = noiseColor.z * 360 / float(NUM_SAMPLE);
	vec2 midPoint =  vec2(0.5, 0.5);

	for(int i = 0; i< NUM_SAMPLE; i++)
	{
		samples[i] = rotationMat2(angleBias * float(i) * RADIAN) * (Xi - midPoint) + midPoint;
		samples[i] = fract(samples[i]);
	}
	/*
	samples[1] = rotationMat2((angleBias + 90.0) * RADIAN) * (Xi - midPoint) + midPoint;
	samples[2] = rotationMat2((angleBias + 180.0) * RADIAN) * (Xi - midPoint) + midPoint;
	samples[3] = rotationMat2((angleBias + 270.0) * RADIAN) * (Xi - midPoint) + midPoint;

	
	samples[1] = fract(samples[1]);
	samples[2] = fract(samples[2]);
	samples[3] = fract(samples[3]);
	*/

	

	//UV_tiling!!!
	//UVforNormalMap *= 0.4;// SSRInfo.w;
	
	float roughness = texture(specularMap, UVforNormalMap).w;

	float globalRoughness = SSRInfo.x;
	globalRoughness = globalRoughness * globalRoughness;
	globalRoughness *= 0.25;

	float Intensity = SSRInfo.y;
	vec2 relfectedUV = fragUV + offset.xy;

	float offsetLen = FLT_MAX;

	if(bufferInfo < UINT_MAX)
	{		
		//values correction
		float correctionPixel = 2.0;

		if(CoordSys == 0)
			relfectedUV = relfectedUV.xy + vec2(0.0, correctionPixel/viewPortSize.y);
		else if(CoordSys == 1)
			relfectedUV = relfectedUV.xy + vec2(correctionPixel/viewPortSize.x, 0.0);
		else if(CoordSys == 2)
			relfectedUV = relfectedUV.xy - vec2(0.0, correctionPixel/viewPortSize.y);
		else if(CoordSys == 3)
			relfectedUV = relfectedUV.xy - vec2(correctionPixel/viewPortSize.x, 0.0);

		offsetLen = length(offset.xy);
	
		outColor = fetchColor(relfectedUV, UVforNormalMap, HitPos_WS.xyz, globalRoughness, mat3(planeInfo[0].rotMat), samples, bUseNormal);
		

		outColor *= fade(relfectedUV);
		outColor *= Intensity;
		outColor.w = offsetLen;

	}	
	else
		outColor.w = FLT_MAX;
		
	//clear IntermediateBuffer
	imageAtomicMax(IntermediateBuffer, iUV, UINT_MAX);
}