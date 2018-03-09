#pragma once

#include "Asset.h"

class Texture : public Asset
{
public:
	Texture():mipLevel(0)
	{

	}

	~Texture()
	{
		shutDown();
	}

	void connectDevice(Vulkan *vulkanAppParam);

	void LoadFromFilename(Vulkan *vulkanAppParam, std::string pathParam);

	void loadTextureImage(std::string path);
	void loadTexture2DArrayImage(std::string path, std::string extension);

	void setMiplevel(int mipLevelParam)
	{
		mipLevel = mipLevelParam;
	}

	virtual void shutDown()
	{
		vkDestroySampler(vulkanApp->getDevice(), textureSampler, nullptr);
		vkDestroyImageView(vulkanApp->getDevice(), textureImageView, nullptr);
		vkDestroyImage(vulkanApp->getDevice(), textureImage, nullptr);
		vkFreeMemory(vulkanApp->getDevice(), textureImageMemory, nullptr);
	}

	void setVulkanApp(Vulkan *vulkanAppParam)
	{
		vulkanApp = vulkanAppParam;
	}


	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;

	VkSampler textureSampler;

	/*
	VkImage getImage()
	{
		return textureImage;
	}

	VkDeviceMemory getImageMemory()
	{
		return textureImageMemory;
	}


	VkImageView getImageView()
	{
		return textureImageView;
	}

	VkSampler getSampler()
	{
		return textureSampler;
	}
	*/

	int mipLevel;

private:

	int texWidth;
	int texHeight;
	int texChannels;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	
};
