#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D SceneTexture;
layout(binding = 1) uniform sampler2D SpecularGbuffer;
layout(binding = 2) uniform sampler2D NormalGbuffer;

layout(binding = 3) uniform sampler2D noiseTex;

layout(set = 0, binding = 4) uniform cameraBuffer
{
	mat4 viewMat;
	mat4 projMat;
	mat4 viewProjMat;
	mat4 InvViewProjMat;

	vec4 cameraWorldPos;
	vec4 viewPortSize;
};

layout(binding = 5) uniform sampler2D DepthMipTexture0;
layout(binding = 6) uniform sampler2D DepthMipTexture1;
layout(binding = 7) uniform sampler2D DepthMipTexture2;
layout(binding = 8) uniform sampler2D DepthMipTexture3;
layout(binding = 9) uniform sampler2D DepthMipTexture4;
layout(binding = 10) uniform sampler2D DepthMipTexture5;
layout(binding = 11) uniform sampler2D DepthMipTexture6;
layout(binding = 12) uniform sampler2D DepthMipTexture7;

float getDepthFromMipmap(int level, vec2 UV)
{
	float depth;
	switch(level)
	{
		case 0: depth = texture(DepthMipTexture0, UV).x;
		break;

		case 1: depth = texture(DepthMipTexture1, UV).x;
		break;

		case 2: depth = texture(DepthMipTexture2, UV).x;
		break;

		case 3: depth = texture(DepthMipTexture3, UV).x;
		break;

		case 4: depth = texture(DepthMipTexture4, UV).x;
		break;

		case 5: depth = texture(DepthMipTexture5, UV).x;
		break;

		case 6: depth = texture(DepthMipTexture6, UV).x;
		break;

		case 7: depth = texture(DepthMipTexture7, UV).x;
		break;

		default: depth = 1.0;
		break;
	}

	return depth;
}


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

vec3 intersectDepthPlane(vec3 o, vec3 d, float t)
{
	return o + d * t;
}

vec2 getCell(vec2 ray, vec2 cellCount)
{
	// does this need to be floor, or does it need fractional part - i think cells are meant to be whole pixel values (integer values) but not sure
	return floor(ray * cellCount);
}

vec3 intersectCellBoundary(vec3 o, vec3 d, vec2 cellIndex, vec2 cellCount, vec2 crossStep, vec2 crossOffset)
{
	vec2 index = cellIndex + crossStep;
	index /= cellCount;
	index += crossOffset;
	vec2 delta = index - o.xy;
	delta /= d.xy;
	float t = min(delta.x, delta.y);
	return intersectDepthPlane(o, d, t);
}

float getMinimumDepthPlane(vec2 uv, float level)
{
	// not sure why we need rootLevel for this
	return getDepthFromMipmap(int(level), uv);
}

vec2 getCellCount(float level)
{
	vec2 div = vec2(pow(2.0,level));
	return viewPortSize.xy / div;
}

bool crossedCellBoundary(vec2 cellIdxOne, vec2 cellIdxTwo)
{
	return int(cellIdxOne.x) != int(cellIdxTwo.x) || int(cellIdxOne.y) != int(cellIdxTwo.y);
}

vec3 getClosetCell(vec3 o, vec2 index, vec3 dir, vec2 sign, vec2 cellCount, float level)
{
	vec2 transedIndex = index;
	
	if(sign.x > 0.0)
		transedIndex.x = ceil(transedIndex.x);
	else
		transedIndex.x = floor(transedIndex.x);

	if(sign.y > 0.0)
		transedIndex.y = ceil(transedIndex.y);
	else
		transedIndex.y = floor(transedIndex.y);

	float pixelGap = pow(2.0, level);


	if(transedIndex.x == index.x)
	{
		if(sign.x > 0.0)
			transedIndex.x += pixelGap;
		else
			transedIndex.x -= pixelGap;
	}

	if(transedIndex.y == index.y)
	{
		if(sign.y > 0.0)
			transedIndex.y += pixelGap;
		else
			transedIndex.y -= pixelGap;
	}

	float slope;

	float X1;
	float Y1;

	float X2;
	float Y2;

	X1 = index.x;
	Y1 = index.y;

	X2 = transedIndex.x;
	Y2 = transedIndex.y;

	float newX2;	
	float newY2;

	vec2 gap;

	if(dir.x != 0.0 && dir.y != 0.0)
	{
		slope = dir.y / dir.x;

		newX2 = (Y2 - Y1) / slope + X1;	
		newY2 = slope * (X2 - X1) + Y1;

		gap.x = distance(vec2(newX2, Y2), index);
		gap.y = distance(vec2(X2, newY2), index);
	}
	else if(dir.x == 0.0)
	{
		newY2 = Y2;
		newX2 = X2 = X1;

		gap.y = abs(Y2 - Y1);
		gap.x = 1024.0;
	}
	else if(dir.y == 0.0)
	{
		newX2 = X2;
		newY2 = Y1;

		gap.x = abs(X2 - X1);
		gap.y = 1024.0;
	}

	float t;
	vec2 delta;

	if( gap.x < gap.y )
	{
		transedIndex = vec2(X2, newY2);

		delta.x = transedIndex.x/cellCount.x - o.x;
		delta.x /= dir.x;

		t = delta.x;
	}
	else
	{
		transedIndex = vec2(newX2, Y2);

		delta.y = transedIndex.y/cellCount.y - o.y;
		delta.y /= dir.y;

		t = delta.y;
	}

	return vec3(transedIndex/cellCount, t);
	//return intersectDepthPlane(o, dir, t);
}


vec3 hiZTraceV(vec3 Pss, vec3 D)
{
	return vec3(0.0);
}


vec3 hiZTrace(vec3 Pss, vec3 D)
{
	//xy is in ScreenSpace
	vec3 o = intersectDepthPlane(Pss, D, -Pss.z);

	vec2 sign = vec2(D.x >= 0.0 ? 1.0 : -1.0, D.y >= 0.0 ? 1.0 : -1.0);

	float level = 0.0;

	uint iterations = 0u;
	uint max = 64;

	vec3 ray = Pss;
	
	vec2 offset = 1.0 / (8.0 * viewPortSize.xy) * sign;

	while(level >= 0.0 && iterations < max)
	{		
		vec2 cellCount = getCellCount(level);		
		vec2 cellIndex = ray.xy * cellCount;

		vec2 oldCellIndex = getCell(ray.xy , cellCount);

		ray = getClosetCell(o, cellIndex, D, sign, cellCount, level);		

		if(ray.x > 1.0 || ray.x < 0.0 || ray.y > 1.0 || ray.y < 0.0 )
		{
			iterations = max;
			break;
		}

		vec2 newCellIndex = getCell(ray.xy + offset, cellCount);

		if(oldCellIndex == newCellIndex)
		{	
			return vec3(-2.0);
			//level -= 1.0;
		}		

		float depth = getMinimumDepthPlane(ray.xy, level);

		if(ray.z > depth)
		//if(ray.z > depthLinear(depth))
		{
			break;
		}

		iterations++;
	}

	if(iterations == max)
		return vec3(-1.0);
	else
		return ray;
}


void main()
{
	float depth = getDepthFromMipmap(0, fragUV);

	if(depth >= 1.0)
	{
		outColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	outColor = clamp( texture( SceneTexture, fragUV), 0.0, 1.0);
	return;

	float linearDepth = depthLinear(depth);

	

	//get WorldPosition
	vec4 worldPos = getWorldPosition(fragUV, depth);
	vec3 viewVec = normalize( cameraWorldPos.xyz - worldPos.xyz); 
	vec3 normalVec = texture(NormalGbuffer, fragUV).xyz;
	vec3 relfecVec =  normalize(reflect(-viewVec ,normalVec ));

	vec4 Pos_SS = viewProjMat * vec4(worldPos.xyz + relfecVec, 1.0);
	Pos_SS /= Pos_SS.w;
	Pos_SS.xy = (Pos_SS.xy + vec2(1.0)) * 0.5;

	

	
	vec3 viewVec_VS = vec3(viewMat * vec4(-viewVec, 0.0));


	// PVS
	//vec3 positionVS = viewVec_VS * linearDepth;

	vec4 worldPos_VS = viewMat * worldPos;
	vec4 cameraPos_VS = viewMat * cameraWorldPos;

	vec3 toPositionVS = normalize(worldPos_VS.xyz);
	

	vec3 normalVec_VS = normalize(vec3(viewMat * vec4(normalVec, 0.0)));
	//vec3 relfecVec_VS =  normalize(reflect( toPositionVS, normalVec_VS ));
	vec3 relfecVec_VS = normalize(vec3(viewMat * vec4(relfecVec, 0.0)));

	//forward to camera 
	if(relfecVec_VS.z >= 0.0)
	{
		outColor = vec4( 0.0, 0.0, 0.0, 0.0);
		return;
	}	

	vec4 positionPrimeSS4 = projMat * vec4(worldPos_VS.xyz + relfecVec_VS, 1.0);
	positionPrimeSS4 /= positionPrimeSS4.w;

	vec3 positionPrimeSS = positionPrimeSS4.xyz;

	positionPrimeSS4.xy = (positionPrimeSS4.xy + vec2(1.0)) * 0.5;
	//positionPrimeSS4.z = depthLinear(positionPrimeSS4.z);


	vec3 Pss_ori = vec3( fragUV , depth);

	vec3 Vss = positionPrimeSS4.xyz - Pss_ori.xyz;

	

	vec3 D = Vss / Vss.z;

	//outColor =  texture( SceneTexture, D.xy);   clamp( vec4( D.x,D.y,0.0, 0.0), 0.0, 1.0);
	//	return;
		
	vec3 result = hiZTrace(Pss_ori, D);
	
	outColor = vec4(result.xy, 0.0, 1.0);
	return;

	if(result.x == -1.0)
		outColor = vec4(0.0);
	else if(result.x == -2.0)
		outColor = vec4(1.0, 0.0, 1.0, 1.0);
	else
		outColor = clamp( texture( SceneTexture, result.xy), 0.0, 1.0);
}
