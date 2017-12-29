#pragma once

#include "../Core/Vulkan.h"
#include "Actor.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 1000.0f

class Light : public Actor
{
public:
	LightInfo lightInfo;

	void initialize(Vulkan *pVulkanApp)
	{
		vulkanApp = pVulkanApp;
	}

	void shutDown()
	{
		//vkDestroyBuffer(vulkanApp->getDevice(), lightUniformBuffer, nullptr);
		//vkFreeMemory(vulkanApp->getDevice(), lightUniformMemory, nullptr);
	}

	

	//VkBuffer lightUniformBuffer;
	//VkDeviceMemory lightUniformMemory;

private:
	Vulkan *vulkanApp;
};

class DirectionalLight : public Light
{
public:

	glm::vec3 getViewVector3()
	{
		return -glm::vec3(modelMat[2].x, modelMat[2].y, modelMat[2].z);
	}

	glm::vec4 getViewVector4()
	{
		return glm::vec4(getViewVector3(), 0.0);
	}

	
};

class PointLight : public Light
{
public:
};