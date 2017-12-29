#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb-master/stb_image.h>


void Texture::connectDevice(Vulkan *vulkanAppParam)
{
	vulkanApp = vulkanAppParam;
}

void Texture::LoadFromFilename(Vulkan *vulkanAppParam, std::string pathParam)
{
	connectDevice(vulkanAppParam);
	path = pathParam;

	loadTextureImage(path);
	
	vulkanApp->createImageView(textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, textureImageView);
	vulkanApp->createTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 0.0f, 0.0f, textureSampler);
}



void Texture::loadTextureImage(std::string path)
{
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	vulkanApp->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(vulkanApp->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(vulkanApp->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	vulkanApp->createImage(VK_IMAGE_TYPE_2D, texWidth, texHeight, 1, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	
	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
	vulkanApp->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1, 0);
	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());

	vkDestroyBuffer(vulkanApp->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), stagingBufferMemory, nullptr);
}