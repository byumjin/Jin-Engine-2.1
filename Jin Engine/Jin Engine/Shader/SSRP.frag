#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_PLANES 4

#define UINT_MAX 4294967295
#define FLT_MAX  3.402823466e+38F

layout(binding = 0) uniform sampler2D sceneMap;
layout(binding = 1, r32ui) uniform uimage2D IntermediateBuffer;
layout(binding = 2) uniform sampler2D depthMap;

layout(set = 0, binding = 3) uniform cameraBuffer
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

layout(set = 0, binding = 4) uniform planeInfoBuffer
{	
	PlaneInfo planeInfo[MAX_PLANES];
	uint numPlanes;
	uint pad00;
	uint pad01;
	uint pad02;
};

float getDistance(vec3 planeNormal, vec3 planeCenter, vec3 worldPos)
{
	//plane to point
	float d = -dot(planeNormal, planeCenter);
	return (dot(planeNormal, worldPos) + d) / length(planeNormal);
}

bool intersectPlane(in uint index, in vec3 worldPos, in vec2 fragUV, out vec4 reflectedPos) 
{ 
	PlaneInfo thisPlane = planeInfo[index];

	// assuming vectors are all normalized
	vec3 normalVec;	
	normalVec = thisPlane.rotMat[2].xyz;
	
	vec3 centerPoint = thisPlane.centerPoint.xyz;

	vec3 projectedWorldPos = dot(normalVec, worldPos - centerPoint) * normalVec;
	vec3 target = worldPos - 2.0 * projectedWorldPos;

	//plane to point	
	float dist = getDistance(normalVec, centerPoint, target);
	
	//if target is on the upper-side of plane, false 
	if(dist >= 0.0)
	{
		return false;
	}

	vec3 rO = cameraWorldPos.xyz;
	vec3 rD = normalize(target - rO);

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

		if( (abs(xGap) <= width) && (abs(yGap) <= height))
		{
			reflectedPos = viewProjMat * vec4(hitPoint, 1.0);
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

vec4 getWorldPosition(vec2 UV, float depth)
{
	vec4 worldPos = InvViewProjMat * vec4(UV * 2.0 - 1.0, depth, 1.0);
	worldPos /= worldPos.w;

	return worldPos;
}

uint packInfo(vec2 offset)
{
	uint CoordSys = 0;

	uint YInt = 0;
	int YFrac = 0;
	int XInt = 0;
	int XFrac = 0;

	//define CoordSystem
	if(abs(offset.y) < abs(offset.x) )
	{
		if(offset.x < 0.0) // 3
		{
			YInt = uint(-offset.x);
			YFrac = int(fract(offset.x)*8.0);
			
			XInt = int(offset.y);
			XFrac = int(fract(offset.y)*8.0);

			CoordSys = 3;
		}
		else // 1
		{
			YInt = uint(offset.x);
			YFrac = int(fract(offset.x)*8.0);
			
			XInt = int(offset.y);
			XFrac = int(fract(offset.y)*8.0);

			CoordSys = 1;
		}
	}
	else	
	{
		if(offset.y < 0.0) // 2
		{
			YInt = uint(-offset.y);
			YFrac = int(fract(offset.y)*8.0);
			
			XInt = int(offset.x);
			XFrac = int(fract(offset.x)*8.0);

			CoordSys = 2;
		}
		else // 0
		{
			YInt = uint(offset.y);
			YFrac = int(fract(offset.y)*8.0);
			
			XInt = int(offset.x);
			XFrac = int(fract(offset.x)*8.0);

			CoordSys = 0;
		}
	}

	return  ( (YInt & 0x00000fff ) << 20) | ( (YFrac & 0x00000007) << 17) | ( (XInt & 0x00000fff) << 5) | ( (XFrac & 0x00000007 )<< 2) | CoordSys;
}

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
	
	/*
	uint screenWidth = uint( viewPortSize.x );
	uint screenHeight = uint( viewPortSize.y );
	
	uint index = gl_GlobalInvocationID.x;	

	if(index >= screenWidth * screenHeight)
		return;

	uint indexY = index / screenWidth;
	uint indexX = index - screenWidth * indexY;

	vec2 fragUV = vec2(float(indexX) / (viewPortSize.x), float(indexY) / (viewPortSize.y) );
	*/


	float depth = texture(depthMap, fragUV).x;	

	//if there is no obj
	if(depth >= 1.0)
		return;

	vec4 worldPos = getWorldPosition(fragUV, depth);
	
	vec4 reflectedPos = vec4(0.0);
	
	uint relfectedPlanarIndex;
	for(uint i = 0; i < numPlanes; i++)
	{	
		if(!intersectPlane( i, worldPos.xyz, fragUV, reflectedPos ))
		{
			return;			
		}

		relfectedPlanarIndex = i;
	}

	ivec2 reflectedUV =  ivec2( reflectedPos.x * viewPortSize.x, reflectedPos.y * viewPortSize.y);
	vec2 offset = vec2( (fragUV.x - reflectedPos.x) * viewPortSize.x, ( fragUV.y - reflectedPos.y) * viewPortSize.y);
		

	//pack info
	uint intermediateBufferValue = packInfo(offset);	
	imageAtomicMin(IntermediateBuffer, reflectedUV, intermediateBufferValue);
}
 