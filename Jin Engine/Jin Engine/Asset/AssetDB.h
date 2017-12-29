#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#include "Asset.h"

#include "../Actor/Object.h"



class AssetDatabase
{
private:
	static AssetDatabase * instance;
	static Vulkan * vulkanApp;

	std::unordered_map<std::type_index, std::unordered_map<std::string, Asset*>> assetMap;	

public:
	
	//GeoList
	std::vector<std::string> geomList;

	//TextureList
	std::vector<std::string> textureList;

	//MaterialList
	std::vector<std::string> materialList;

	std::vector<Object*> objectManager;
	//std::vector<Material*> materialManager;

	AssetDatabase();
	
	~AssetDatabase()
	{

	}

	void cleanUp()
	{
		for (uint32_t i = 0; i < geomList.size(); i++)
		{
			Geometry* pGeo = FindAsset<Geometry>(geomList[i]);
			delete pGeo;
		}

		for (uint32_t i = 0; i < textureList.size(); i++)
		{
			Texture* pTex = FindAsset<Texture>(textureList[i]);
			delete pTex;
		}

		
		for (uint32_t i = 0; i < materialList.size(); i++)
		{
			Material* pMaterial = FindAsset<Material>(materialList[i]);
			delete pMaterial;
		}
		

		for (uint32_t i = 0; i < objectManager.size(); i++)
		{
			delete objectManager[i];
		}

		assetMap.clear();

		geomList.clear();
		textureList.clear();
		materialList.clear();
		objectManager.clear();
	}


	void releaseRenderingPart()
	{
		for (uint32_t i = 0; i < materialList.size(); i++)
		{
			Material* pMaterial = FindAsset<Material>(materialList[i]);
			pMaterial->shutDown();
			//pMaterial->releasePipeline();
		}
	}



	Material* LoadMaterial(std::string materialName)
	{
		return FindAsset<Material>(materialName);		
	}
	
	Texture* SaveTexture(std::string path)
	{
		AssetDatabase::GetInstance()->textureList.push_back(path);
		return AssetDatabase::GetInstance()->LoadAsset<Texture>(path);
		
	}

	Geometry* SaveGeomtery(std::string path)
	{
		AssetDatabase::GetInstance()->geomList.push_back(path);
		return AssetDatabase::GetInstance()->LoadAsset<Geometry>(path);		
	}

	/*
	Material* SaveMaterial(std::string path)
	{
		AssetDatabase::GetInstance()->materialList.push_back(path);
		return AssetDatabase::GetInstance()->LoadAsset<Material>(path);
	}
	*/

	/*
	void SaveObjectMaterial(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass)
	{
		Material* GbufferMat = new GbufferMaterial;

		instance->SaveAsset<Material>(GbufferMat, name);

		GbufferMat->LoadFromFilename(vulkanApp, name);
		GbufferMat->addTexture(instance->LoadAsset<Texture>(albedo));
		GbufferMat->addTexture(instance->LoadAsset<Texture>(specular));
		GbufferMat->addTexture(instance->LoadAsset<Texture>(normal));
		GbufferMat->addTexture(instance->LoadAsset<Texture>(emissive));

		GbufferMat->addUniformBuffer(objectBuffer);
		GbufferMat->addUniformBuffer(cameraBuffer);

		GbufferMat->setShaderPaths("Shader/gbuffers.vert.spv", "Shader/gbuffers.frag.spv", "", "", "", "");
		GbufferMat->createDescriptor(ScreenOffsets, sizeScale);

		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		colorBlendAttachments.resize(NUM_GBUFFERS);

		GbufferMat->createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
		GbufferMat->createColorBlendAttachmentState(colorBlendAttachments[1], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
		GbufferMat->createColorBlendAttachmentState(colorBlendAttachments[2], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
		GbufferMat->createColorBlendAttachmentState(colorBlendAttachments[3], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	
		VkPipelineDepthStencilStateCreateInfo depthStencil;
		GbufferMat->createDepthStencilState(depthStencil);
		
		GbufferMat->createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_TRUE, depthStencil, 0.0f, renderPass);

		AssetDatabase::GetInstance()->materialList.push_back(name);
	}
	*/

	static AssetDatabase * GetInstance();

	void setVulkanApp(Vulkan *pVulkanApp)
	{
		vulkanApp = pVulkanApp;
	}

	template <class T>
	T* LoadAsset(std::string id)
	{
		T * t = FindAsset<T>(id);

		if (t != nullptr)
			return t;

		t = new T();
		t->LoadFromFilename(vulkanApp, id);
		assetMap[typeid(T)][id] = t;
		return t;
	}

	template<class T>
	T * SaveAsset(T *t, std::string id)
	{
		t->LoadFromFilename(vulkanApp, id);
		assetMap[typeid(T)][id] = t;
		return t;
	}


	template<class T>
	T * FindAsset(std::string id)
	{
		std::unordered_map<std::string, Asset*>& idMap = assetMap[typeid(T)];

		if (idMap.find(id) != idMap.end())
			return dynamic_cast<T*>(idMap[id]);

		return nullptr;
	}

};