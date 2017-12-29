#pragma once

#include "../Core/Vulkan.h"
#include "../Asset/Material.h"


class PostProcess
{
public:

	void shutDown()
	{

		vkDestroyRenderPass(vulkanApp->getDevice(), renderPass, nullptr);
		vkFreeCommandBuffers(vulkanApp->getDevice(), cmdPool, 1, &cmd);
		vkDestroyCommandPool(vulkanApp->getDevice(), cmdPool, nullptr);

		for (size_t i = 0; i < semaphores.size(); i++)
		{
			vkDestroySemaphore(vulkanApp->getDevice(), semaphores[i], nullptr);
		}

		delete material;
	}

	void initialize(Vulkan* pVulkanApp, Material *pMaterial)
	{
		vulkanApp = pVulkanApp;
		material = pMaterial;


	}

	

private:
	
	Vulkan *vulkanApp;
	Material *material;

	VkRenderPass renderPass;
	VkCommandPool cmdPool;
	VkCommandBuffer cmd;

	std::vector<VkSemaphore> semaphores;
	std::vector<VkQueue> queues;

};