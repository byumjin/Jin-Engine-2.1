#pragma once

#include "../Core/Common.h"
#include "../Asset/Geometry.h"

#include "../Asset/Material.h"
#include "../Asset/Texture.h"

#include "Actor.h"

class Object : public Actor
{
public:
	Object():bRoll(false), UflipCorrection(false)
	{
		
	}

	~Object()
	{
		/*
		for (size_t i = 0; i < geoms.size(); i++)
		{
			delete geoms[i];
		}

		geoms.clear();
		*/

		vkDestroyBuffer(vulkanApp->getDevice(), uniformObjectBuffer, nullptr);
		vkFreeMemory(vulkanApp->getDevice(), uniformObjectBufferMemory, nullptr);
	}

	void initialize(Vulkan *pvulkanApp, std::string actorNameParam, std::string pathParam, bool needUflipCorrection);

	void processNode(aiNode* node, const aiScene const* scene, int &geomID);

	bool LoadFromFilename(std::string path);

	void connectMaterial(uint32_t matIndex);

	void setAABB();
	/*
	BoundingBox getAABB()
	{
		return AABB;
	}
	*/
	void createObjectBuffer();

	void updateObjectBuffer();

	std::vector<Geometry*> geoms;
	std::vector<Material*> materials;

	std::vector<std::vector<uint32_t>> materialGroup;

	std::vector<BoundingBox> geomAABB;
	//std::vector<bool> Culling;
	//bool IsCulled;
	objectBuffer objectBufferInfo;

	VkBuffer uniformObjectBuffer;
	VkDeviceMemory uniformObjectBufferMemory;

	BoundingBox AABB;

private:

	

	bool bRoll;
	bool UflipCorrection;

	float rollSpeed;

	Vulkan *vulkanApp;

	uint32_t numMaterials;

};