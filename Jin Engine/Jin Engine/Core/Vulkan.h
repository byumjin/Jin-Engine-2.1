#pragma once

#include "Interface.h"

struct QueueFamilyIndices
{
	int graphicsFamily = -1;	
	int presentFamily = -1;
	int transferFamily = -1;
	int computeFamily = -1;

	bool isComplete()
	{
		return computeFamily >= 0 && graphicsFamily >= 0 && transferFamily >= 0 &&  presentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

#ifdef _DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}



class Vulkan
{
public:

	Vulkan();
	~Vulkan();
	void initialize(Interface &interface);
	
	void shutDown();

	void createInstance(const char* engineName, int major, int minor, int patch);
	std::vector<const char*> getRequiredExtensions();

	bool checkValidationLayerSupport();
	void setupDebugCallback();

	void createSurface(Interface &interface);
	void pickPhysicalDevice();

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createLogicalDevice();

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Interface &interface);

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);


	

	void createRenderPass(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout initialLayout, VkImageLayout finalLayout,
		std::vector<VkAttachmentDescription> &attachments,
		std::vector<VkSubpassDescription> &subpasses,
		std::vector<VkSubpassDependency> &dependencies,
		VkRenderPass &renderPass);

	void createFramebuffers(std::vector<VkImageView> imageViews, VkImageView depthImageView, std::vector<VkFramebuffer>  &framebuffers, VkRenderPass &renderPass,
		uint32_t width, uint32_t height, uint32_t layerCount, uint32_t numColorAttachment);

	void createCommandPool(VkCommandPool &cmdPool);

	void createCommandBuffers(VkCommandBufferLevel cmdLevel, std::vector<VkFramebuffer> &Framebuffers, std::vector<VkCommandBuffer> &cmdBuffers,
		VkCommandPool &cmdPool);

	void recordCommandBuffers(std::vector<VkCommandBuffer> *cmdBuffers,
		VkCommandPool cmdPool, std::vector<VkFramebuffer> *Framebuffers,
		std::string materialName,
		VkRenderPass renderPass, VkExtent2D extent, std::vector<VkClearValue> *clearValues,
		int drawMode,
		VkBuffer vertexBuffer, uint32_t vertexOffset, uint32_t vertexCount,
		uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);

	VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool);
	void endSingleTimeCommands(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

	void createBufferView(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize size, VkBufferView bufferView)
	{
		VkBufferViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		view_info.pNext = NULL;
		view_info.buffer = buffer;
		view_info.format = format;
		view_info.offset = offset;
		view_info.range = size;
		vkCreateBufferView(device, &view_info, NULL, &bufferView);
	}

	void destroyBufferView(VkBufferView bufferView)
	{
		vkDestroyBufferView(device, bufferView, NULL);
		bufferView = NULL;
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool cmdPool, VkQueue queue);

	void updateBuffer(void* srcData, VkDeviceMemory deviceMemory, VkDeviceSize size)
	{
		void* data;
		vkMapMemory(device, deviceMemory, 0, size, 0, &data);
		memcpy(data, srcData, static_cast<size_t>(size));
		vkUnmapMemory(device, deviceMemory);
	}

	void bufferMemoryBarrier(VkBuffer buffer, VkDeviceSize size, VkAccessFlags src, VkAccessFlags dst, VkCommandPool commandPool, VkQueue queue);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, VkQueue queue);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool commandPool, VkQueue queue, VkImageSubresourceRange subresourceRange);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel);

	void blitImage(VkImage srcImage, VkImageLayout srcLayout, VkImage dstImage, VkImageLayout dstLayout, uint32_t regionCount, VkFilter filter, VkImageBlit imageBlit, VkCommandPool commandPool, VkQueue queue)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);
		vkCmdBlitImage(commandBuffer, srcImage,	srcLayout, dstImage, dstLayout, regionCount, &imageBlit, filter);

		endSingleTimeCommands(commandPool, commandBuffer, queue);
	}

	VkDeviceSize createImage(VkImageType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelParam, uint32_t arrayLayersParam,
		VkFormat format, VkImageTiling tiling, VkImageLayout imageLayout, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount,
		VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);


	void createImageView(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectFlags,
		uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkImageView &imageView);


	void createTextureSampler(VkFilter filter, VkSamplerAddressMode wrappingMode, VkBool32 anisotropy, float maxAnisotropy, VkBorderColor color, VkBool32 unnormalizedCoords,
		VkSamplerMipmapMode mipmapMode, float bias, float minLod, float maxLod, VkSampler &textureSampler);

	VkDevice getDevice()
	{
		return device;
	}

	VkPhysicalDevice getPhysicalDevice()
	{
		return physicalDevice;
	}

	
	VkSurfaceKHR getSurface()
	{
		return surface;
	}

	/*
	VkSwapchainKHR getSwapchain()
	{
		return swapChain;
	}

	VkSwapchainKHR* getSwapchainPointer()
	{
		return &swapChain;
	}
	*/

	VkCommandPool getTransferCmdPool()
	{
		return transferCmdPool;
	}

	VkQueue getTransferQueue()
	{
		return transferQueue;
	}

private:

	VkInstance instance;
	VkDebugReportCallbackEXT callback;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	std::vector<const char*> deviceExtensions;

	VkSurfaceKHR surface;
	//VkSwapchainKHR swapChain;

	

	VkCommandPool transferCmdPool;
	VkQueue transferQueue;
};

