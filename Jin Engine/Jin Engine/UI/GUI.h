#pragma once

#include "imgui.h"
#include "imgui_impl_glfw_vulkan.h"

#include "../Core/Vulkan.h"

class GUI
{
public:

	GUI()
	{
		vulkanApp = NULL;
	}

	~GUI()
	{
		
	}

	void cleanUp()
	{
		vkDestroyDescriptorPool(init_data.device, init_data.descriptor_pool, init_data.allocator);
		vkDestroyRenderPass(init_data.device, init_data.render_pass, init_data.allocator);
	}

	void initGUI(Vulkan* pVulkanApp)
	{
		vulkanApp = pVulkanApp;
		// Setup ImGui binding
		ImGui::CreateContext();
		io = ImGui::GetIO(); (void)io;

		init_data = {};
		init_data.allocator = NULL;
		init_data.gpu = vulkanApp->getPhysicalDevice();
		init_data.device = vulkanApp->getDevice();
		
		init_data.pipeline_cache = VK_NULL_HANDLE;


		setRenderPass();

		setDescriptorPool();
		
		init_data.check_vk_result = check_vk_result;
	}

	/*
	void createFrameBuffer(std::vector<VkImageView> &ImageViews, std::vector<VkFramebuffer> &frameBuffers,  uint32_t width, uint32_t height, uint32_t layerCount)
	{
		//createFrameBuffer
		vulkanApp->createFramebuffers(ImageViews, NULL, frameBuffers, init_data.render_pass, width, height, layerCount, 1);
	}
	*/

	void setRenderPass()
	{
		VkResult err;

		VkAttachmentDescription attachment = {};
		attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;

		err = vkCreateRenderPass(init_data.device, &info, init_data.allocator, &init_data.render_pass);
		check_vk_result(err);

		

	}

	void setDescriptorPool()
	{
		VkResult err;

		VkDescriptorPoolSize pool_size[11] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * 11;
		pool_info.poolSizeCount = 11;
		pool_info.pPoolSizes = pool_size;

		err = vkCreateDescriptorPool(init_data.device, &pool_info, init_data.allocator, &init_data.descriptor_pool);
		check_vk_result(err);
	}

	ImGuiIO io;
	ImGui_ImplGlfwVulkan_Init_Data init_data;
	
	Vulkan* vulkanApp;


};

