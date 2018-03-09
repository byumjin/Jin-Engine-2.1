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

	vulkanApp->createImageView(textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevel, 0, 1, textureImageView);
	vulkanApp->createTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_TRUE, 16.0, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 0.0f, 5.0f, textureSampler);
}



void Texture::loadTextureImage(std::string path)
{
	stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	mipLevel = glm::max(static_cast<int>(glm::floor(glm::log2(static_cast<float>(glm::min(texWidth, texHeight))))), 0) + 1;

	VkDeviceSize imageSize = vulkanApp->createImage(VK_IMAGE_TYPE_2D, texWidth, texHeight, 1, mipLevel, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	vulkanApp->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	VkDeviceSize baseImageSize = texWidth * texHeight * 4;

	void* data;
	vkMapMemory(vulkanApp->getDevice(), stagingBufferMemory, 0, baseImageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(baseImageSize));
	vkUnmapMemory(vulkanApp->getDevice(), stagingBufferMemory);

	stbi_image_free(pixels);	
	
	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
	
	vulkanApp->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1, 0);

	//vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
	//Prepare base mip level
	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());

	//Generating the mip-chain

	for (int32_t i = 1; i < mipLevel; i++)
	{
		VkImageBlit imageBlit{};

		// Source
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.srcOffsets[1].x = int32_t(texWidth >> (i - 1));
		imageBlit.srcOffsets[1].y = int32_t(texHeight >> (i - 1));
		imageBlit.srcOffsets[1].z = 1;

		// Destination
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstSubresource.mipLevel = i;
		imageBlit.dstOffsets[1].x = int32_t(texWidth >> i);
		imageBlit.dstOffsets[1].y = int32_t(texHeight >> i);
		imageBlit.dstOffsets[1].z = 1;

		VkImageSubresourceRange mipSubRange = {};
		mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mipSubRange.baseMipLevel = i;
		mipSubRange.levelCount = 1;
		mipSubRange.layerCount = 1;

		vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue(), mipSubRange);

		vulkanApp->blitImage(textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, VK_FILTER_LINEAR,
			imageBlit,	vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
		
		vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue(), mipSubRange);

		
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevel;
	subresourceRange.layerCount = 1;

	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue(), subresourceRange);

	vkDestroyBuffer(vulkanApp->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), stagingBufferMemory, nullptr);
}

void Texture::loadTexture2DArrayImage(std::string path, std::string extension)
{
	std::string tempPath = path + "_0" + extension;

	stbi_uc* pixels = stbi_load(tempPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	int texDepth = texWidth;

	VkDeviceSize imageArraySize = texWidth * texHeight * texDepth * 4;
	
	vulkanApp->createBuffer(imageArraySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	vulkanApp->createImage(VK_IMAGE_TYPE_3D, texWidth, texHeight, texDepth, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	stbi_image_free(pixels);


	VkDeviceSize imageSize = texWidth * texHeight * 4;

	for (int i = 0; i < texDepth; i++)
	{
		tempPath = path + "_" + std::to_string(i) + extension;

		pixels = stbi_load(tempPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		void* data;
		vkMapMemory(vulkanApp->getDevice(), stagingBufferMemory, imageSize * i, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(vulkanApp->getDevice(), stagingBufferMemory);

		stbi_image_free(pixels);
	}	

	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
	vulkanApp->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(texDepth), 0);
	vulkanApp->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());
	
	vkDestroyBuffer(vulkanApp->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), stagingBufferMemory, nullptr);

	vulkanApp->createImageView(textureImage, VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, textureImageView);
	vulkanApp->createTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 0.0f, 0.0f, textureSampler);
}