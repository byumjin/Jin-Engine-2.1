#include "Material.h"

#include "../Asset/AssetDB.h"

Material::~Material()
{
	//shutDown();
}

void Material::addTexture(Texture* texture)
{
	textures.push_back(texture);
}

void Material::addBuffer(VkBuffer* buffer)
{
	buffers.push_back(buffer);
}

VkShaderModule Material::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(vulkanApp->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void Material::setShaderPaths(std::string v, std::string f, std::string c, std::string e, std::string g, std::string p)
{
	vertexShaderPath = v;
	fragmentShaderPath = f;
	tessellationControlShaderPath = c;
	tessellationEvaluationShaderPath = e;
	geometryShaderPath = g;
	computeShaderPath = p;
}

void Material::LoadFromFilename(Vulkan *vulkanAppParam, std::string pathParam)
{
	vulkanApp = vulkanAppParam;
	path = pathParam;
}

void Material::createLayoutBinding(VkDescriptorSetLayoutBinding &layoutBinding, uint32_t binding, uint32_t descCount, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
	layoutBinding.binding = binding;
	layoutBinding.descriptorCount = descCount;
	layoutBinding.descriptorType = type;
	layoutBinding.pImmutableSamplers = nullptr;
	layoutBinding.stageFlags = stageFlags;
}

void Material::createDescriptorPool(std::vector<VkDescriptorPoolSize> &descPool, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(descPool.size());
	poolInfo.pPoolSizes = descPool.data();
	poolInfo.maxSets = maxSets;

	if (vkCreateDescriptorPool(vulkanApp->getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Material::createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vulkanApp->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void Material::createDescriptorSet(std::vector<VkDescriptorSetLayout> &layouts)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(vulkanApp->getDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}
}

void Material::updateDescriptorSet(std::vector<VkWriteDescriptorSet> &descriptorWrites)
{
	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Material::releasePipeline()
{
	vkDestroyPipeline(vulkanApp->getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(vulkanApp->getDevice(), pipelineLayout, nullptr);
}

void Material::shutDown()
{
	releasePipeline();

	vkDestroyDescriptorPool(vulkanApp->getDevice(), descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(vulkanApp->getDevice(), descriptorSetLayout, nullptr);
}



void Material::colorBlendAttachmentState(VkPipelineColorBlendAttachmentState &colorBlendAttachmentState, VkBool32 blendEnable,
	VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorblendOp,
	VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp
)
{
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = blendEnable;
	colorBlendAttachmentState.srcColorBlendFactor = srcColorBlendFactor;
	colorBlendAttachmentState.dstColorBlendFactor = dstColorBlendFactor;
	colorBlendAttachmentState.colorBlendOp = colorblendOp;
	colorBlendAttachmentState.srcAlphaBlendFactor = srcAlphaBlendFactor;
	colorBlendAttachmentState.dstAlphaBlendFactor = dstAlphaBlendFactor;
	colorBlendAttachmentState.alphaBlendOp = alphaBlendOp;
}

void Material::depthStencilState(VkPipelineDepthStencilStateCreateInfo &depthStencilInfo, VkBool32 depthTestEnable, VkBool32 depthWriteEnable,
	VkBool32 depthBoundsTestEnable, VkBool32 stencilTestEnable)
{
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = depthTestEnable;
	depthStencilInfo.depthWriteEnable = depthWriteEnable;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = depthBoundsTestEnable;
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = stencilTestEnable;
	depthStencilInfo.front = {};
	depthStencilInfo.back = {};
}


void Material::createComputePipeline()
{
	auto compShaderCode = readFile(computeShaderPath);

	VkShaderModule compShaderModule = createShaderModule(compShaderCode);

	VkPipelineShaderStageCreateInfo compShaderStageInfo = {};
	compShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compShaderStageInfo.module = compShaderModule;
	compShaderStageInfo.pName = "main";

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(vulkanApp->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkComputePipelineCreateInfo computePipelineInfo = {};
	computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineInfo.pNext = NULL;
	computePipelineInfo.flags = 0;
	computePipelineInfo.stage = compShaderStageInfo;
	computePipelineInfo.layout = pipelineLayout;
	computePipelineInfo.basePipelineHandle = 0;
	computePipelineInfo.basePipelineIndex = 0;

	if (vkCreateComputePipelines(vulkanApp->getDevice(), VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(vulkanApp->getDevice(), compShaderModule, nullptr);
}



void Material::createGraphicsPipeline(VkPolygonMode polygonMode, float lineWidth, VkCullModeFlags cullMode, VkFrontFace frontFace, VkSampleCountFlagBits sampleCountFlag,
	std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments, VkBool32 bDepthStencil, VkPipelineDepthStencilStateCreateInfo &depthStencilInfo,
	float blendingConstant, VkRenderPass &renderPass)
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkShaderModule vertShaderModule = NULL;
	VkShaderModule tescShaderModule = NULL;
	VkShaderModule teseShaderModule = NULL;
	VkShaderModule geomShaderModule = NULL;
	VkShaderModule fragShaderModule = NULL;

	if (vertexShaderPath != "")
	{
		auto vertShaderCode = readFile(vertexShaderPath);
		vertShaderModule = createShaderModule(vertShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		shaderStages.push_back(vertShaderStageInfo);
	}

	if (tessellationControlShaderPath != "")
	{
		auto tescShaderCode = readFile(tessellationControlShaderPath);
		tescShaderModule = createShaderModule(tescShaderCode);

		VkPipelineShaderStageCreateInfo tescShaderStageInfo = {};
		tescShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		tescShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		tescShaderStageInfo.module = tescShaderModule;
		tescShaderStageInfo.pName = "main";

		shaderStages.push_back(tescShaderStageInfo);
	}

	if (tessellationEvaluationShaderPath != "")
	{
		auto teseShaderCode = readFile(tessellationEvaluationShaderPath);
		teseShaderModule = createShaderModule(teseShaderCode);

		VkPipelineShaderStageCreateInfo teseShaderStageInfo = {};
		teseShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		teseShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		teseShaderStageInfo.module = teseShaderModule;
		teseShaderStageInfo.pName = "main";

		shaderStages.push_back(teseShaderStageInfo);
	}

	if (geometryShaderPath != "")
	{
		auto geomShaderCode = readFile(geometryShaderPath);
		geomShaderModule = createShaderModule(geomShaderCode);

		VkPipelineShaderStageCreateInfo geomShaderStageInfo = {};
		geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geomShaderStageInfo.module = geomShaderModule;
		geomShaderStageInfo.pName = "main";

		shaderStages.push_back(geomShaderStageInfo);
	}

	if (fragmentShaderPath != "")
	{
		auto fragShaderCode = readFile(fragmentShaderPath);
		fragShaderModule = createShaderModule(fragShaderCode);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		shaderStages.push_back(fragShaderStageInfo);
	}


	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = screenOffset.x;
	viewport.y = screenOffset.y;
	viewport.width = sizeScale.x / sizeScale.z;
	viewport.height = sizeScale.y / sizeScale.w;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkExtent2D extent;
	extent.width = (uint32_t)viewport.width;
	extent.height = (uint32_t)viewport.height;

	VkRect2D scissor = {};
	scissor.offset = { static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y) };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = polygonMode;
	rasterizer.lineWidth = lineWidth;
	rasterizer.cullMode = cullMode;
	rasterizer.frontFace = frontFace;//  VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = sampleCountFlag;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = blendingConstant;
	colorBlending.blendConstants[1] = blendingConstant;
	colorBlending.blendConstants[2] = blendingConstant;
	colorBlending.blendConstants[3] = blendingConstant;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(vulkanApp->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (bDepthStencil)
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
	else
		pipelineInfo.pDepthStencilState = NULL;

	if (vkCreateGraphicsPipelines(vulkanApp->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(vulkanApp->getDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(vulkanApp->getDevice(), geomShaderModule, nullptr);
	vkDestroyShaderModule(vulkanApp->getDevice(), teseShaderModule, nullptr);
	vkDestroyShaderModule(vulkanApp->getDevice(), tescShaderModule, nullptr);
	vkDestroyShaderModule(vulkanApp->getDevice(), vertShaderModule, nullptr);
}

void Material::createDescriptorWrite(VkWriteDescriptorSet &writeDescriptorSet, uint32_t index, uint32_t binding, VkDescriptorType type,
	VkDescriptorImageInfo *imageInfo, VkDescriptorBufferInfo *bufferInfo, VkBufferView TexelBufferView)
{
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.descriptorCount = 1;

	if (type == VK_DESCRIPTOR_TYPE_SAMPLER || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	{
		writeDescriptorSet.pImageInfo = imageInfo;
	}
	else if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	{
		writeDescriptorSet.pBufferInfo = bufferInfo;
	}
	else  if (type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
	{
		writeDescriptorSet.pTexelBufferView = &TexelBufferView;
	}
}

void Material::createImageInfo(VkDescriptorImageInfo &imageInfo, VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler)
{
	imageInfo.imageLayout = imageLayout;
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;

}

void Material::createBufferInfo(VkDescriptorBufferInfo &bufferInfo, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize bufferSize)
{
	bufferInfo.buffer = buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = bufferSize;
}


void Material::createColorBlendAttachmentState(VkPipelineColorBlendAttachmentState &colorBlendAttachment, VkBool32 blendEnable,
	VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendOp colorBlend,
	VkBlendFactor srcAlpha, VkBlendFactor dstAlpha, VkBlendOp alphaBlend)
{
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = blendEnable;
	colorBlendAttachment.srcColorBlendFactor = srcColor; // Optional
	colorBlendAttachment.dstColorBlendFactor = dstColor; // Optional
	colorBlendAttachment.colorBlendOp = colorBlend; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = srcAlpha; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = dstAlpha; // Optional
	colorBlendAttachment.alphaBlendOp = alphaBlend; // Optional
}

void Material::createDepthStencilState(VkPipelineDepthStencilStateCreateInfo &depthStencilInfo)
{
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.pNext = NULL;
	depthStencilInfo.flags = 0;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;
	depthStencilInfo.stencilTestEnable = VK_TRUE;
	depthStencilInfo.front = {};
	depthStencilInfo.back = {};
}

VkDescriptorPool Material::getDescPool()
{
	return descriptorPool;
}

VkDescriptorSetLayout Material::getDescSetLayout()
{
	return descriptorSetLayout;
}

VkDescriptorSet Material::getDescSet()
{
	return descriptorSet;
}

VkDescriptorSet* Material::getDescSetPointer()
{
	return &descriptorSet;
}

VkPipeline Material::getPipeline()
{
	return pipeline;
}

VkPipelineLayout Material::getPipelineLayout()
{
	return pipelineLayout;
}


void GbufferMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(6);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[3].descriptorCount = 1;

	descPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[4].descriptorCount = 1;

	descPoolSize[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[5].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[1], 1, 1, descPoolSize[1].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[2], 2, 1, descPoolSize[2].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[3], 3, 1, descPoolSize[3].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[4], 4, 1, descPoolSize[4].type, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[5], 5, 1, descPoolSize[5].type, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(4);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, textures[BASIC_COLOR]->textureImageView, textures[BASIC_COLOR]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, textures[SPECULAR_COLOR]->textureImageView, textures[SPECULAR_COLOR]->textureSampler);
	createImageInfo(ImageInfos[2], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, textures[NORMAL_COLOR]->textureImageView, textures[NORMAL_COLOR]->textureSampler);
	createImageInfo(ImageInfos[3], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, textures[EMISSIVE_COLOR]->textureImageView, textures[EMISSIVE_COLOR]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(2);

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(objectBuffer));
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, &ImageInfos[2], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, &ImageInfos[3], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[4], 4, 4, descPoolSize[4].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[5], 5, 5, descPoolSize[5].type, nullptr, &bufferInfos[1], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void GbufferMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture(AssetDatabase::GetInstance()->LoadAsset<Texture>(albedo));
	addTexture(AssetDatabase::GetInstance()->LoadAsset<Texture>(specular));
	addTexture(AssetDatabase::GetInstance()->LoadAsset<Texture>(normal));
	addTexture(AssetDatabase::GetInstance()->LoadAsset<Texture>(emissive));

	addBuffer(objectBuffer);
	addBuffer(cameraBuffer);

	setShaderPaths("Shader/gbuffers.vert.spv", "Shader/gbuffers.frag.spv", "", "", "", "");
	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(NUM_GBUFFERS);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[1], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[2], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[3], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil;
	createDepthStencilState(depthStencil);
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_TRUE, depthStencil, 0.0f, renderPass);
}

void GbufferMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(NUM_GBUFFERS);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[1], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[2], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);
	createColorBlendAttachmentState(colorBlendAttachments[3], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil;
	createDepthStencilState(depthStencil);
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_TRUE, depthStencil, 0.0f, renderPass);
}

void FrustumCullingMaterial::createLocalBuffer()
{
	vulkanApp->createBuffer(sizeof(BoundingBox)* MAX_CULLING, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		AABBBuffers, AABBBufferMem);

	vulkanApp->createBuffer(sizeof(FrustumInfo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		frustumInfoBuffer, frustumInfoBufferMem);

	vulkanApp->createBuffer(sizeof(objectBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		selectedObjBuffer, selectedObjBufferMem);
}

void FrustumCullingMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(4);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[3].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_COMPUTE_BIT);
	createLayoutBinding(descLayoutBinding[1], 1, 1, descPoolSize[1].type, VK_SHADER_STAGE_COMPUTE_BIT);
	createLayoutBinding(descLayoutBinding[2], 2, 1, descPoolSize[2].type, VK_SHADER_STAGE_COMPUTE_BIT);
	createLayoutBinding(descLayoutBinding[3], 3, 1, descPoolSize[3].type, VK_SHADER_STAGE_COMPUTE_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(4);

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(BoundingBox) * MAX_CULLING);
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(FrustumInfo));
	createBufferInfo(bufferInfos[2], *buffers[2], 0, sizeof(objectBuffer));
	createBufferInfo(bufferInfos[3], *buffers[3], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, nullptr, &bufferInfos[1], NULL);
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, nullptr, &bufferInfos[2], NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, nullptr, &bufferInfos[3], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void FrustumCullingMaterial::createPipeline(std::string name,
	std::string albedo, std::string specular, std::string normal, std::string emissive,
	VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	createLocalBuffer();

	addBuffer(&AABBBuffers);
	addBuffer(&frustumInfoBuffer);
	//addBuffer(&ubos);
	addBuffer(&selectedObjBuffer);
	addBuffer(cameraBuffer);

	setShaderPaths("", "", "", "", "", "Shader/frustumCulling.comp.spv");
	createDescriptor(ScreenOffsets, SizeScale);

	createComputePipeline();

}

void FrustumCullingMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);
	createComputePipeline();
}

void UberMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(8);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[3].descriptorCount = 1;

	descPoolSize[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[4].descriptorCount = 1;

	descPoolSize[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[5].descriptorCount = 1;

	descPoolSize[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[6].descriptorCount = 1;

	descPoolSize[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[7].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[1], 1, 1, descPoolSize[1].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[2], 2, 1, descPoolSize[2].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[3], 3, 1, descPoolSize[3].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[4], 4, 1, descPoolSize[4].type, VK_SHADER_STAGE_FRAGMENT_BIT);

	createLayoutBinding(descLayoutBinding[5], 5, 1, descPoolSize[5].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[6], 6, 1, descPoolSize[6].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[7], 7, 1, descPoolSize[7].type, VK_SHADER_STAGE_FRAGMENT_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(5);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[BASIC_COLOR]->textureImageView, textures[BASIC_COLOR]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[SPECULAR_COLOR]->textureImageView, textures[SPECULAR_COLOR]->textureSampler);
	createImageInfo(ImageInfos[2], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[NORMAL_COLOR]->textureImageView, textures[NORMAL_COLOR]->textureSampler);
	createImageInfo(ImageInfos[3], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[EMISSIVE_COLOR]->textureImageView, textures[EMISSIVE_COLOR]->textureSampler);
	createImageInfo(ImageInfos[4], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[4]->textureImageView, textures[4]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());
	
	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(LightInfo) * numPointLights);
	createBufferInfo(bufferInfos[2], *buffers[2], 0, sizeof(LightInfo) * numDirectionalLights);


	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, &ImageInfos[2], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, &ImageInfos[3], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[4], 4, 4, descPoolSize[4].type, &ImageInfos[4], nullptr, NULL);

	createDescriptorWrite(descriptorWrites[5], 5, 5, descPoolSize[5].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[6], 6, 6, descPoolSize[6].type, nullptr, &bufferInfos[1], NULL);
	createDescriptorWrite(descriptorWrites[7], 7, 7, descPoolSize[7].type, nullptr, &bufferInfos[2], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void UberMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{

	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture((*renderTarget)[BASIC_COLOR]);
	addTexture((*renderTarget)[SPECULAR_COLOR]);
	addTexture((*renderTarget)[NORMAL_COLOR]);
	addTexture((*renderTarget)[EMISSIVE_COLOR]);
	addTexture(pDepthImageView);
	addBuffer(cameraBuffer);
	addBuffer(pointLightBuffer);
	addBuffer(directionalLightBuffer);

	setShaderPaths("Shader/pbr.vert.spv", "Shader/pbr.frag.spv", "", "", "", "");

	//glm::vec2 screenOffsets = glm::vec2(0.0);
	//glm::vec4 sizeScale = glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0);
	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void UberMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}


/////////////////////// ScreenSpaceProjection ///////////////////////////////////

void ScreenSpaceProjectionMaterial::createLocalBuffer()
{
	vulkanApp->createBuffer(sizeof(PlaneInfoPack), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		planeInfoBuffer, planeInfoBufferMem);
}

void ScreenSpaceProjectionMaterial::updatePlaneInfoPackBuffer(PlaneInfoPack &planeInfoPack)
{
	VkDeviceSize bufferSize = sizeof(PlaneInfoPack);

	void* data;
	vkMapMemory(vulkanApp->getDevice(), planeInfoBufferMem, 0, bufferSize, 0, &data);
	memcpy(data, &planeInfoPack, static_cast<size_t>(bufferSize));
	vkUnmapMemory(vulkanApp->getDevice(), planeInfoBufferMem);
}

void ScreenSpaceProjectionMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(5);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[3].descriptorCount = 1;

	descPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[4].descriptorCount = 1;	

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[1]->textureImageView, textures[1]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(uint32_t) * MAX_SCREEN_WIDTH* MAX_SCREEN_HEIGHT);
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(cameraBuffer));
	createBufferInfo(bufferInfos[2], *buffers[2], 0, sizeof(PlaneInfoPack));
	
	

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);
	
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, nullptr, &bufferInfos[1], NULL);
	createDescriptorWrite(descriptorWrites[4], 4, 4, descPoolSize[4].type, nullptr, &bufferInfos[2], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void ScreenSpaceProjectionMaterial::createPipeline(std::string name,
	std::string albedo, std::string specular, std::string normal, std::string emissive,
	VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	createLocalBuffer();

	addTexture((*renderTarget)[0]); //Scene
	addTexture(pDepthImageView);

	addBuffer(cameraBuffer);
	addBuffer(&planeInfoBuffer);
	

	setShaderPaths("", "", "", "", "", "Shader/SSRP.comp.spv");
	createDescriptor(ScreenOffsets, SizeScale);

	createComputePipeline();

}



void ScreenSpaceProjectionMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);
	createComputePipeline();
}

/////////////////////////////////////////////////// SSR ///////////////////////////////////////////////////////

void ScreenSpaceReflectionMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(10);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	/*
	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[2].descriptorCount = 1;
	*/

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[3].descriptorCount = 1;

	descPoolSize[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[4].descriptorCount = 1;

	descPoolSize[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[5].descriptorCount = 1;

	descPoolSize[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[6].descriptorCount = 1;

	descPoolSize[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[7].descriptorCount = 1;

	descPoolSize[8].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descPoolSize[8].descriptorCount = 1;

	descPoolSize[9].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[9].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);
	//createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[1]->textureImageView, textures[1]->textureSampler);
	//createImageInfo(ImageInfos[2], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[2]->textureImageView, textures[2]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[1]->textureImageView, textures[1]->textureSampler);
	createImageInfo(ImageInfos[2], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[2]->textureImageView, textures[2]->textureSampler);
	createImageInfo(ImageInfos[3], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[3]->textureImageView, textures[3]->textureSampler);
	createImageInfo(ImageInfos[4], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[4]->textureImageView, textures[4]->textureSampler);
	createImageInfo(ImageInfos[5], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[5]->textureImageView, textures[5]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	//createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(SSRDepthInfo) * (MAX_SCREEN_WIDTH / (uint32_t)sizeScale.z)* (MAX_SCREEN_HEIGHT / (uint32_t)sizeScale.w));
	//createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(uint32_t) * (MAX_SCREEN_WIDTH / (uint32_t)sizeScale.z)* (MAX_SCREEN_HEIGHT / (uint32_t)sizeScale.w));
	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(PlaneInfoPack));
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(glm::vec4));
	createBufferInfo(bufferInfos[2], *buffers[2], 0, sizeof(uint32_t) * MAX_SCREEN_WIDTH* MAX_SCREEN_HEIGHT);
	createBufferInfo(bufferInfos[3], *buffers[3], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, &ImageInfos[2], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, &ImageInfos[3], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[4], 4, 4, descPoolSize[4].type, &ImageInfos[4], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[5], 5, 5, descPoolSize[5].type, &ImageInfos[5], nullptr, NULL);

	//createDescriptorWrite(descriptorWrites[6], 6, 6, descPoolSize[6].type, &ImageInfos[6], nullptr, NULL);
	//createDescriptorWrite(descriptorWrites[7], 7, 7, descPoolSize[7].type, &ImageInfos[7], nullptr, NULL);
	
	createDescriptorWrite(descriptorWrites[6], 6, 6, descPoolSize[6].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[7], 7, 7, descPoolSize[7].type, nullptr, &bufferInfos[1], NULL);
	createDescriptorWrite(descriptorWrites[8], 8, 8, descPoolSize[8].type, nullptr, &bufferInfos[2], NULL);
	createDescriptorWrite(descriptorWrites[9], 9, 9, descPoolSize[9].type, nullptr, &bufferInfos[3], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void ScreenSpaceReflectionMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	//createLocalBuffer();

	for (size_t i = 0; i < renderTarget->size(); i++)
	{
		addTexture((*renderTarget)[i]);
	}

	addTexture(pDepthImageView);

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/SSRR.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void ScreenSpaceReflectionMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}
/////

void SkyRenderingMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(6);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[2].descriptorCount = 1;

	descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[3].descriptorCount = 1;

	descPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[4].descriptorCount = 1;

	descPoolSize[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[5].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[1], 1, 1, descPoolSize[1].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[2], 2, 1, descPoolSize[2].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[3], 3, 1, descPoolSize[3].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[4], 4, 1, descPoolSize[4].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[5], 5, 1, descPoolSize[5].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(4);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[1]->textureImageView, textures[1]->textureSampler);
	createImageInfo(ImageInfos[2], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[2]->textureImageView, textures[2]->textureSampler);
	createImageInfo(ImageInfos[3], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[3]->textureImageView, textures[3]->textureSampler);
	
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));
	createBufferInfo(bufferInfos[1], *buffers[1], 0, sizeof(perframeBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, &ImageInfos[2], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[3], 3, 3, descPoolSize[3].type, &ImageInfos[3], nullptr, NULL);

	createDescriptorWrite(descriptorWrites[4], 4, 4, descPoolSize[4].type, nullptr, &bufferInfos[0], NULL);
	createDescriptorWrite(descriptorWrites[5], 5, 5, descPoolSize[5].type, nullptr, &bufferInfos[1], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void SkyRenderingMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{

	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture((*renderTarget)[0]);
	addTexture((*renderTarget)[1]);
	addTexture((*renderTarget)[2]);
	addTexture(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/cloudTextures/wheather.tga"));

	addBuffer(cameraBuffer);
	addBuffer(perFrameBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/sky.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void SkyRenderingMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void HolePatchingMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(2);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, nullptr, &bufferInfos[0], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void HolePatchingMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	for (size_t i = 0; i < renderTarget->size(); i++)
	{
		addTexture((*renderTarget)[i]);
	}

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/holePatching.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, sizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void HolePatchingMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void HorizontalBlurMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(2);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, nullptr, &bufferInfos[0], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void HorizontalBlurMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	for (size_t i = 0; i < renderTarget->size(); i++)
	{
		addTexture((*renderTarget)[i]);
	}

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/horizontalBlur.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, sizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void HorizontalBlurMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void VerticalBlurMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(2);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, nullptr, &bufferInfos[0], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VerticalBlurMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	for (size_t i = 0; i < renderTarget->size(); i++)
	{
		addTexture((*renderTarget)[i]);
	}

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/horizontalBlur.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, sizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void VerticalBlurMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}


void CompositePostProcessMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(3);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[1].descriptorCount = 1;

	descPoolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[2].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	for (uint32_t i = 0; i < static_cast<uint32_t>(descLayoutBinding.size()); i++)
	{
		createLayoutBinding(descLayoutBinding[i], i, descPoolSize[i].descriptorCount, descPoolSize[i].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	createDescriptorSetLayout(descLayoutBinding);

	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(textures.size());

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);
	createImageInfo(ImageInfos[1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[1]->textureImageView, textures[1]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(cameraBuffer));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, &ImageInfos[1], nullptr, NULL);

	createDescriptorWrite(descriptorWrites[2], 2, 2, descPoolSize[2].type, nullptr, &bufferInfos[0], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void CompositePostProcessMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 sizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{
	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);

	for (size_t i = 0; i < renderTarget->size(); i++)
	{
		addTexture((*renderTarget)[i]);
	}

	//addTexture(pDepthImageView);

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/compositePostProcess.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, sizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void CompositePostProcessMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}


void ToneMappingMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(1);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(1);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void ToneMappingMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{

	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture((*renderTarget)[0]);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/toneMapping.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void ToneMappingMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void PresentMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(1);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(1);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void PresentMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{

	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture((*renderTarget)[0]);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/postProcess.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void PresentMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void DepthMipmapMaterial::createDescriptor(glm::vec2 screenOffsetParam, glm::vec4 sizeScaleParam)
{
	Material::createDescriptor(screenOffsetParam, sizeScaleParam);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	descPoolSize.resize(2);

	descPoolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descPoolSize[0].descriptorCount = 1;
	descPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize[1].descriptorCount = 1;

	createDescriptorPool(descPoolSize);

	std::vector<VkDescriptorSetLayoutBinding> descLayoutBinding;
	descLayoutBinding.resize(descPoolSize.size());

	createLayoutBinding(descLayoutBinding[0], 0, 1, descPoolSize[0].type, VK_SHADER_STAGE_FRAGMENT_BIT);
	createLayoutBinding(descLayoutBinding[1], 1, 1, descPoolSize[1].type, VK_SHADER_STAGE_FRAGMENT_BIT);

	createDescriptorSetLayout(descLayoutBinding);


	std::vector<VkDescriptorImageInfo> ImageInfos;
	ImageInfos.resize(1);

	createImageInfo(ImageInfos[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textures[0]->textureImageView, textures[0]->textureSampler);

	std::vector<VkDescriptorBufferInfo> bufferInfos;
	bufferInfos.resize(buffers.size());

	createBufferInfo(bufferInfos[0], *buffers[0], 0, sizeof(glm::vec4));

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	descriptorSetLayouts[0] = descriptorSetLayout;

	createDescriptorSet(descriptorSetLayouts);

	std::vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(descPoolSize.size());

	createDescriptorWrite(descriptorWrites[0], 0, 0, descPoolSize[0].type, &ImageInfos[0], nullptr, NULL);
	createDescriptorWrite(descriptorWrites[1], 1, 1, descPoolSize[1].type, nullptr, &bufferInfos[0], NULL);

	vkUpdateDescriptorSets(vulkanApp->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void DepthMipmapMaterial::createPipeline(std::string name, std::string albedo, std::string specular, std::string normal, std::string emissive, VkBuffer *objectBuffer, VkBuffer *cameraBuffer,
	VkBuffer *pointLightBuffer, size_t numPointLight, VkBuffer *directionalLightBuffer, size_t numDirectionalLight, VkBuffer *perFrameBuffer,
	glm::vec2 ScreenOffsets, glm::vec4 SizeScale, VkRenderPass renderPass, std::vector<Texture*> *renderTarget, Texture *pDepthImageView)
{

	numPointLights = static_cast<uint32_t>(numPointLight);
	numDirectionalLights = static_cast<uint32_t>(numDirectionalLight);

	AssetDatabase::GetInstance()->materialList.push_back(name);
	AssetDatabase::GetInstance()->SaveAsset<Material>(this, name);

	LoadFromFilename(vulkanApp, name);
	addTexture(pDepthImageView);

	addBuffer(cameraBuffer);

	setShaderPaths("Shader/postProcess.vert.spv", "Shader/depthMipmap.frag.spv", "", "", "", "");

	createDescriptor(ScreenOffsets, SizeScale);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}

void DepthMipmapMaterial::updatePipeline(glm::vec2 screenOffsetParam, glm::vec4 sizeScalescreenOffsetParam, VkRenderPass renderPass)
{
	createDescriptor(screenOffsetParam, sizeScalescreenOffsetParam);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	colorBlendAttachments.resize(1);

	createColorBlendAttachmentState(colorBlendAttachments[0], VK_FALSE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	createGraphicsPipeline(VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_SAMPLE_COUNT_1_BIT, colorBlendAttachments, VK_FALSE, depthStencil, 0.0f, renderPass);
}