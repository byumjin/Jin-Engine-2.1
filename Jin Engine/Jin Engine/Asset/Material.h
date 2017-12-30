#pragma once

#include "Asset.h"
#include "Texture.h"
#include "Geometry.h"

class Material : public Asset
{
public:

	Material():vertexShaderPath(""), tessellationControlShaderPath(""), tessellationEvaluationShaderPath(""), geometryShaderPath(""), fragmentShaderPath(""), computeShaderPath("")
	{
		
	}

	virtual ~Material();
	virtual void addTexture(Texture* texture);
	virtual void addBuffer(VkBuffer* buffer);
	virtual void setShaderPaths(std::string v, std::string f, std::string c, std::string e, std::string g, std::string p);
	virtual void LoadFromFilename(Vulkan *vulkanAppParam, std::string pathParam);
	void createLayoutBinding(VkDescriptorSetLayoutBinding &layoutBinding, uint32_t binding, uint32_t descCount, VkDescriptorType type, VkShaderStageFlags stageFlags);

	virtual void createDescriptorPool(std::vector<VkDescriptorPoolSize> &descPool, uint32_t maxSets = 1);
	virtual void createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings);
	virtual void createDescriptorSet(std::vector<VkDescriptorSetLayout> &layouts);
	virtual void updateDescriptorSet(std::vector<VkWriteDescriptorSet> &descriptorWrites);

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
	{
		screenOffset = screenOffsetParam;
		sizeScale = sizeScaleParam;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code);
	
	void colorBlendAttachmentState(VkPipelineColorBlendAttachmentState &colorBlendAttachmentState, VkBool32 blendEnable,
		VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorblendOp,
		VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp
	);

	void depthStencilState(VkPipelineDepthStencilStateCreateInfo &depthStencilInfo, VkBool32 depthTestEnable, VkBool32 depthWriteEnable,
		VkBool32 depthBoundsTestEnable, VkBool32 stencilTestEnable);

	void createComputePipeline();

	void createGraphicsPipeline(VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode, VkFrontFace frontFace, VkSampleCountFlagBits sampleCountFlag,
		std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments, VkBool32 bDepthStencil, VkPipelineDepthStencilStateCreateInfo &depthStencilInfo,
		float blendingConstant, VkRenderPass &renderPass);	

	void createDescriptorWrite(VkWriteDescriptorSet &writeDescriptorSet, uint32_t index, uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo *imageInfo, VkDescriptorBufferInfo *bufferInfo);	
	void createImageInfo(VkDescriptorImageInfo &imageInfo, VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler);
	void createBufferInfo(VkDescriptorBufferInfo &bufferInfo, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize bufferSize);

	void createColorBlendAttachmentState(VkPipelineColorBlendAttachmentState &colorBlendAttachment, VkBool32 blendEnable,
		VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendOp colorBlend,
		VkBlendFactor srcAlpha, VkBlendFactor dstAlpha, VkBlendOp alphaBlend);

	void createDepthStencilState(VkPipelineDepthStencilStateCreateInfo &depthStencilInfo);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
	{

	}

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
	{

	}

	virtual void releasePipeline();
	virtual void shutDown();

	VkDescriptorPool getDescPool();
	VkDescriptorSetLayout getDescSetLayout();	
	VkDescriptorSet getDescSet();	
	VkDescriptorSet* getDescSetPointer();	
	VkPipeline getPipeline();
	VkPipelineLayout getPipelineLayout();	

	std::vector<Texture*> textures;
	std::vector<VkBuffer*> buffers;

	uint32_t renderPassID;
	glm::vec2 screenOffset;
	glm::vec4 sizeScale;

	uint32_t numPointLights;
	uint32_t numDirectionalLights;

protected:

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	std::string vertexShaderPath;
	std::string tessellationControlShaderPath;
	std::string tessellationEvaluationShaderPath;
	std::string geometryShaderPath;
	std::string fragmentShaderPath;

	std::string computeShaderPath;
	glm::ivec3 computeDispatchSize;

	


};

class GbufferMaterial : public Material
{
public:

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);


private:
};

class FrustumCullingMaterial : public Material
{
public:

	virtual ~FrustumCullingMaterial()
	{
		vkDestroyBuffer(vulkanApp->getDevice(), AABBBuffers, nullptr);
		vkFreeMemory(vulkanApp->getDevice(), AABBBufferMem, nullptr);

		vkDestroyBuffer(vulkanApp->getDevice(), frustumInfoBuffer, nullptr);
		vkFreeMemory(vulkanApp->getDevice(), frustumInfoBufferMem, nullptr);

		vkDestroyBuffer(vulkanApp->getDevice(), selectedObjBuffer, nullptr);
		vkFreeMemory(vulkanApp->getDevice(), selectedObjBufferMem, nullptr);

		Material::~Material();
	}

	virtual void shutDown()
	{
		Material::shutDown();
	}

	void updateLocalBuffer(std::vector<BoundingBox> &AABBArray, int objIndex, FrustumInfo &frustumInfo, objectBuffer &selectedOnjInfo)
	{
			VkDeviceSize bufferSize = sizeof(BoundingBox) * AABBArray.size();

			void* data;
			vkMapMemory(vulkanApp->getDevice(), AABBBufferMem, 0, bufferSize, 0, &data);
			memcpy(data, AABBArray.data(), static_cast<size_t>(bufferSize));
			vkUnmapMemory(vulkanApp->getDevice(), AABBBufferMem);

			bufferSize = sizeof(FrustumInfo);
			data = NULL;
			vkMapMemory(vulkanApp->getDevice(), frustumInfoBufferMem, 0, bufferSize, 0, &data);
			memcpy(data, &frustumInfo, static_cast<size_t>(bufferSize));
			vkUnmapMemory(vulkanApp->getDevice(), frustumInfoBufferMem);

			bufferSize = sizeof(objectBuffer);
			data = NULL;
			vkMapMemory(vulkanApp->getDevice(), selectedObjBufferMem, 0, bufferSize, 0, &data);
			memcpy(data, &selectedOnjInfo, static_cast<size_t>(bufferSize));
			vkUnmapMemory(vulkanApp->getDevice(), selectedObjBufferMem);
	}

	void mapCullingInfo(std::vector<BoundingBox> &AABBArray)
	{
		VkDeviceSize bufferSize = sizeof(BoundingBox) * AABBArray.size();

		void* data;
		vkMapMemory(vulkanApp->getDevice(), AABBBufferMem, 0, bufferSize, 0, &data);
		memcpy(&AABBArray[0], data, static_cast<size_t>(bufferSize));
		vkUnmapMemory(vulkanApp->getDevice(), AABBBufferMem);
	}

	void createLocalBuffer();

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);

private:

	VkBuffer AABBBuffers;
	VkDeviceMemory AABBBufferMem;

	VkBuffer frustumInfoBuffer;
	VkDeviceMemory frustumInfoBufferMem;
	
	VkBuffer selectedObjBuffer;
	VkDeviceMemory selectedObjBufferMem;
	//VkBuffer ubos;
	//VkDeviceMemory ubosMem;
};

class UberMaterial : public Material
{
public:

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);

private:
};

class SkyRenderingMaterial : public Material
{
public:

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);

private:
};

class ToneMappingMaterial : public Material
{
public:

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);

private:
};

class PresentMaterial : public Material
{
public:

	virtual void createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam);

	virtual void createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
		VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight,
		glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView);

	virtual void updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass);

private:
};