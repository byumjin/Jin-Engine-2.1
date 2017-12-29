#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <fstream>
#include <chrono>

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

};

struct perframeBuffer
{
	glm::vec4 time; //x - totalTime, y - deltaTime
};

enum GBUFFER
{
	BASIC_COLOR = 0, SPECULAR_COLOR, NORMAL_COLOR, EMISSIVE_COLOR
};

#define NUM_GBUFFERS 4
#define MAX_CULLING 1024

#define USE_GPU_CULLING 1