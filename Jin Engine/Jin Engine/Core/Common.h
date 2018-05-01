#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <fstream>
#include <chrono>
#include <string>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define DELETE_SAFE(x) { if(x){ delete x; x = NULL; } }

struct BoundingBox
{
	glm::vec4 Center;            // Center of the box.
	glm::vec4 Extents;           // Distance from the center to each side.

	glm::vec4 minPt;
	glm::vec4 maxPt;

	glm::vec4 corners[8];

	glm::vec4 cullingInfo;
	//bool bCull;
	//float radius;
};

struct FrustumInfo
{
	glm::vec4 planes[6];	
};

struct LightInfo
{
	glm::vec4 color;
	glm::vec4 direction;
	glm::vec4 worldPos;
};

struct constBuffer
{
	glm::vec4 screenInfo; // x - width, y - height, z - invWidth, w - invHeight
};

struct objectBuffer
{
	glm::mat4 modelMat;
	glm::mat4 InvTransposeMat;
};

struct cameraBuffer
{
	glm::mat4 viewMat;
	glm::mat4 projMat;
	glm::mat4 viewProjMat;
	glm::mat4 InvViewProjMat;

	glm::vec4 cameraWorldPos;
	glm::vec4 viewPortSize;

};

struct perframeBuffer
{
	glm::vec4 timeInfo; //x - totalTime, y - deltaTime
};

struct PlaneInfo
{
	glm::mat4 rotMat;
	glm::vec4 centerPoint;
	glm::vec4 size;
};

#define MAX_PLANES 4

struct PlaneInfoPack
{	
	PlaneInfo planeInfo[MAX_PLANES];
	uint32_t numPlanes;
	uint32_t pad00;
	uint32_t pad01;
	uint32_t pad02;
};

struct SSRDepthInfo
{
	glm::vec4 depth;
	//uint32_t order;
	//uint32_t pad00;
	//uint32_t pad01;
	//uint32_t pad02;
};

enum GBUFFER
{
	BASIC_COLOR = 0, SPECULAR_COLOR, NORMAL_COLOR, EMISSIVE_COLOR
};

#define NUM_GBUFFERS 4
#define MAX_CULLING 1024


#define USE_GPU_CULLING 1

#define DEPTH_MIP_POSTPROCESS 0
#define DEPTH_MIP_SIZE 8

#define MAX_SCREEN_WIDTH 1920
#define MAX_SCREEN_HEIGHT 1080

static void check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}
