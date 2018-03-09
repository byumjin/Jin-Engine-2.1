#include "Postprocess.h"

PostProcess::PostProcess(Vulkan* pVulkanApp, std::string materialNameParam, VkFormat frameBufferFormat, uint32_t layerCountParam, VkBuffer vertexBuffer, VkFilter filterParam, VkSamplerMipmapMode mipParam,
	bool bComputeParam, int numRenderTargetParam) : renderPass(NULL)
{
	vulkanApp = pVulkanApp;
	materialName = materialNameParam;
	format = frameBufferFormat;
	layerCount = layerCountParam;
	singleTriangularVertexBuffer = vertexBuffer;

	filter = filterParam;
	mipmapMode = mipParam;

	bCompute = bComputeParam;

	numRenderTarget = numRenderTargetParam;

	createRenderTargets();
	createSemaphores();
	createQueues();
}

void PostProcess::shutDown()
{
	renderTargets.clear();

	for (size_t i = 0; i < semaphores.size(); i++)
	{
		vkDestroySemaphore(vulkanApp->getDevice(), semaphores[i], nullptr);
	}

	semaphores.clear();

	queues.clear();

}

void PostProcess::releaseRenderingPart()
{
	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		renderTargets[i]->shutDown();
	}

	//renderTargets.clear();

	for (size_t i = 0; i < cmds.size(); i++)
	{
		vkFreeCommandBuffers(vulkanApp->getDevice(), cmdPool, 1, &cmds[i]);
	}

	cmds.clear();

	for (size_t i = 0; i < framebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanApp->getDevice(), framebuffers[i], nullptr);
		framebuffers[i] = NULL;
	}

	framebuffers.clear();
	
	if(!bCompute)
		vkDestroyRenderPass(vulkanApp->getDevice(), renderPass, nullptr);
	
	vkDestroyCommandPool(vulkanApp->getDevice(), cmdPool, nullptr);
}



void PostProcess::initialize(glm::vec4 sizeScaleParam)
{
	sizeScale = sizeScaleParam;

	extent.width = static_cast<uint32_t>(sizeScale.x / sizeScale.z);
	extent.height = static_cast<uint32_t>(sizeScale.y / sizeScale.w);

	updateRenderTargets();

	vulkanApp->createCommandPool(cmdPool);

	if (!bCompute)
		createRenderpass();

	std::vector<VkImageView> collectedImageViews;

	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		collectedImageViews.push_back(renderTargets[i]->textureImageView);
	}

	if (!bCompute)
	{
		vulkanApp->createFramebuffers(collectedImageViews, NULL, framebuffers, renderPass, extent.width, extent.height, layerCount, 1);
		vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, framebuffers, cmds, cmdPool);
	}
	else
	{
		std::vector<VkFramebuffer> dummyFrameBuffer;
		dummyFrameBuffer.resize(1);
		vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, dummyFrameBuffer, cmds, cmdPool);
	}
}

void PostProcess::createSemaphores()
{
	semaphores.resize(1);

	for (size_t i = 0; i < semaphores.size(); i++)
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(vulkanApp->getDevice(), &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void PostProcess::deleteSemaphores()
{
	for (size_t i = 0; i < semaphores.size(); i++)
	{
		vkDestroySemaphore(vulkanApp->getDevice(), semaphores[i], nullptr);
	}
}

void PostProcess::createQueues()
{
	queues.resize(1);

	QueueFamilyIndices indices = vulkanApp->findQueueFamilies(vulkanApp->getPhysicalDevice(), vulkanApp->getSurface());

	for (size_t i = 0; i < queues.size(); i++)
	{
		vkGetDeviceQueue(vulkanApp->getDevice(), indices.graphicsFamily, 0, &queues[i]);
	}
}

void PostProcess::recordCommandBuffer()
{
	//record
	std::vector<VkClearValue> clearValues;
	clearValues.resize(2);
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	if (bCompute)
	{
		vulkanApp->recordCommandBuffers(&cmds, cmdPool, NULL, materialName, NULL, extent, NULL, 1, NULL, 0, 0, (extent.width * extent.height) / 32 + 1, 1, 1);
	}
	else
		vulkanApp->recordCommandBuffers(&cmds, cmdPool, &framebuffers, materialName, renderPass, extent, &clearValues, 0, singleTriangularVertexBuffer, 0, 3, 0, 0, 0);

	
}

void PostProcess::createRenderpass()
{
	std::vector<VkAttachmentReference> attachmentRefs = {};
	attachmentRefs.resize(1);
	attachmentRefs[0].attachment = 0;
	attachmentRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses = {};
	subpasses.resize(1);
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = static_cast<uint32_t>(attachmentRefs.size());
	subpasses[0].pColorAttachments = attachmentRefs.data();
	subpasses[0].pDepthStencilAttachment = NULL;

	std::vector<VkAttachmentDescription> attachments = {};
	attachments.resize(1);
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::vector<VkSubpassDependency> dependencies = {};
	dependencies.resize(1);
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	vulkanApp->createRenderPass(format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, attachments, subpasses, dependencies, renderPass);
}

void PostProcess::createRenderTargets()
{
	renderTargets.resize(numRenderTarget);

	for (uint32_t i = 0; i < renderTargets.size(); i++)
	{
		renderTargets[i] = new Texture;
		renderTargets[i]->connectDevice(vulkanApp);
	}
}

void PostProcess::updateRenderTargets()
{
	for (uint32_t i = 0; i < renderTargets.size(); i++)
	{
		if (bCompute)
		{
			vulkanApp->createImage(VK_IMAGE_TYPE_2D, extent.width, extent.height, 1, 1, 1, format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				renderTargets[i]->textureImage, renderTargets[i]->textureImageMemory);
		}
		else
		{
			vulkanApp->createImage(VK_IMAGE_TYPE_2D, extent.width, extent.height, 1, 1, 1, format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				renderTargets[i]->textureImage, renderTargets[i]->textureImageMemory);			
		}

		vulkanApp->createImageView(renderTargets[i]->textureImage, VK_IMAGE_VIEW_TYPE_2D, format,
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount, renderTargets[i]->textureImageView);

		vulkanApp->createTextureSampler(filter, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
			mipmapMode, 0.0f, 0.0f, 0.0f, renderTargets[i]->textureSampler);

		if (bCompute)
		{
			vulkanApp->transitionImageLayout(renderTargets[i]->textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
		}
		
	}

}


VkRenderPass PostProcess::getRenderPass()
{
	return renderPass;
}

VkExtent2D PostProcess::getExtent()
{
	return extent;
}

VkSemaphore* PostProcess::getFirstSM()
{
	return &semaphores[0];
}

VkQueue PostProcess::getFirstQueue()
{
	return queues[0];
}