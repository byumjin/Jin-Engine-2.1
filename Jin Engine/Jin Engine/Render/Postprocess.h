#pragma once

#include "../Core/Vulkan.h"
#include "../Asset/Material.h"


class PostProcess
{
public:

	PostProcess(Vulkan* pVulkanApp, std::string materialNameParam, VkFormat frameBufferFormat, uint32_t layerCountParam, VkBuffer vertexBuffer, VkFilter filterParam, VkSamplerMipmapMode mipParam,
		bool bComputeParam, int numRenderTargetParam);

	void shutDown();

	void releaseRenderingPart();


	void initialize(glm::vec4 sizeScaleParam);

	void createSemaphores();

	void deleteSemaphores();

	void createQueues();

	void recordCommandBuffer();

	void createRenderpass();
	
	void createRenderTargets();
	

	void updateRenderTargets();

	std::string getMaterialName()
	{
		return materialName;
	}

	VkRenderPass getRenderPass();

	VkExtent2D getExtent();

	VkSemaphore *getFirstSM();

	VkQueue getFirstQueue();

	std::vector<VkCommandBuffer> cmds;
	std::vector<Texture*> renderTargets;

	glm::vec4 sizeScale;

	bool bCompute;
private:
	
	Vulkan *vulkanApp;

	std::string materialName;

	VkRenderPass renderPass;
	VkCommandPool cmdPool;
	
	VkBuffer singleTriangularVertexBuffer;
	
	std::vector<VkFramebuffer> framebuffers;

	std::vector<VkSemaphore> semaphores;
	std::vector<VkQueue> queues;

	VkFormat format;
	VkExtent2D extent;
	uint32_t layerCount;

	VkFilter filter;
	VkSamplerMipmapMode mipmapMode;

	

	int numRenderTarget;


};