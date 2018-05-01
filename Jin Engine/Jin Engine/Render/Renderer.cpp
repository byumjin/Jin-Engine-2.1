#include "Renderer.h"

#include "../Asset/AssetDB.h"

void Renderer::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = vulkanApp->querySwapChainSupport(vulkanApp->getPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = vulkanApp->chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = vulkanApp->chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = vulkanApp->chooseSwapExtent(swapChainSupport.capabilities, interface);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vulkanApp->getSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = layerCount;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = vulkanApp->findQueueFamilies(vulkanApp->getPhysicalDevice(), vulkanApp->getSurface());
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily,  (uint32_t)indices.transferFamily, (uint32_t)indices.computeFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if ( vkCreateSwapchainKHR(vulkanApp->getDevice(), &createInfo, nullptr, &swapChain ) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}	

	//Retrieving the swap chain images
	vkGetSwapchainImagesKHR(vulkanApp->getDevice(), swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanApp->getDevice(), swapChain, &imageCount, swapChainImages.data());



	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Renderer::assignRenderpassID(Material* pMat, VkRenderPass renderPass, uint32_t postProcessIndex)
{
	if (renderPass == gbufferRenderPass)
	{
		pMat->renderPassID = RenderPassID::GBUFFER;
	}
	else if (renderPass == mainRenderPass)
	{
		pMat->renderPassID = RenderPassID::MAIN;
	}
	else
	{
		pMat->renderPassID = postProcessIndex + RenderPassID::MAIN + 1;
	}
}

VkRenderPass Renderer::getAssignedRenderpassID(Material* pMat)
{
	if (pMat->renderPassID == RenderPassID::GBUFFER)
	{
		return gbufferRenderPass;
	}
	else if (pMat->renderPassID == RenderPassID::MAIN)
	{
		return mainRenderPass;
	}
	else
	{
		uint32_t renderPassID = pMat->renderPassID;
		return postProcessChain[renderPassID - (RenderPassID::MAIN + 1)]->getRenderPass();
	}
}

void Renderer::setGlobalObjs()
{
	{
		//sponza
		{
			Object *sponza = new Object;
			sponza->initialize(vulkanApp, "sponza", "Asset/Object/sponza/sponza.obj", true);
			sponza->scale = glm::vec3(0.01f);
			sponza->updateObjectBuffer();

			//Texture
			//arch
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/arch/arch_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/arch/arch_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/arch/arch_norm.tga");

			//bricks
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/bricks/bricks_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/bricks/bricks_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/bricks/bricks_norm.tga");

			//celing
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/ceiling/ceiling_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/ceiling/ceiling_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/ceiling/ceiling_norm.tga");

			//column
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_a_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_a_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_a_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_b_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_b_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_b_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_c_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_c_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/column/column_c_norm.tga");

			//curtain
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_blue_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_green_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_red_albedo.tga");

			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_blue_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_green_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_red_spec.tga");

			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/curtain/sponza_curtain_norm.tga");

			//detail
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/detail/detail_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/detail/detail_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/detail/detail_norm.tga");

			//fabric
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_blue_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_blue_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_green_albedo.tga");

			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_green_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_red_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_red_spec.tga");

			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/fabric/fabric_norm.tga");

			//flagpole
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/flagpole/flagpole_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/flagpole/flagpole_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/flagpole/flagpole_norm.tga");

			//floor
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/floor/floor_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/floor/floor_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/floor/floor_norm.tga");

			//lion
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion/lion_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion/lion_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion/lion_spec.tga");

			//lion_back
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion_background/lion_background_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion_background/lion_background_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/lion_background/lion_background_norm.tga");

			//plant
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/plant/vase_plant_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/plant/vase_plant_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/plant/vase_plant_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/plant/vase_plant_emiss.tga");

			//roof
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/roof/roof_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/roof/roof_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/roof/roof_norm.tga");

			//thorn
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/thorn/sponza_thorn_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/thorn/sponza_thorn_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/thorn/sponza_thorn_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/thorn/sponza_thorn_emis.tga");

			//vase
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase/vase_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase/vase_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase/vase_norm.tga");

			//vase others
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase_hanging/vase_hanging_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase_hanging/vase_round_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase_hanging/vase_round_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/vase_hanging/vase_round_norm.tga");

			//chain
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/chain/chain_albedo.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/chain/chain_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/chain/chain_norm.tga");

			//no Emissive
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/no_emis.tga");


			//Material

			glm::vec2 screenOffsets = glm::vec2(0.0);
			glm::vec4 sizeScale = glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0);

			//arch
			GbufferMaterial* temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("arch", "Asset/Texture/sponza/arch/arch_albedo.tga", "Asset/Texture/sponza/arch/arch_spec.tga", "Asset/Texture/sponza/arch/arch_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//bricks
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("bricks", "Asset/Texture/sponza/bricks/bricks_albedo.tga", "Asset/Texture/sponza/bricks/bricks_spec.tga", "Asset/Texture/sponza/bricks/bricks_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//ceiling
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("ceiling", "Asset/Texture/sponza/ceiling/ceiling_albedo.tga", "Asset/Texture/sponza/ceiling/ceiling_spec.tga", "Asset/Texture/sponza/ceiling/ceiling_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//chain
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("chain", "Asset/Texture/sponza/chain/chain_albedo.tga", "Asset/Texture/sponza/chain/chain_spec.tga", "Asset/Texture/sponza/chain/chain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_a
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_a", "Asset/Texture/sponza/column/column_a_albedo.tga", "Asset/Texture/sponza/column/column_a_spec.tga", "Asset/Texture/sponza/column/column_a_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_b
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_b", "Asset/Texture/sponza/column/column_b_albedo.tga", "Asset/Texture/sponza/column/column_b_spec.tga", "Asset/Texture/sponza/column/column_b_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_c
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_c", "Asset/Texture/sponza/column/column_c_albedo.tga", "Asset/Texture/sponza/column/column_c_spec.tga", "Asset/Texture/sponza/column/column_c_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_blue
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_blue", "Asset/Texture/sponza/curtain/sponza_curtain_blue_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_blue_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_green
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_green", "Asset/Texture/sponza/curtain/sponza_curtain_green_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_green_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_red
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_red", "Asset/Texture/sponza/curtain/sponza_curtain_red_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_red_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//detail
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("detail", "Asset/Texture/sponza/detail/detail_albedo.tga", "Asset/Texture/sponza/detail/detail_spec.tga", "Asset/Texture/sponza/detail/detail_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_blue
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_blue", "Asset/Texture/sponza/fabric/fabric_blue_albedo.tga", "Asset/Texture/sponza/fabric/fabric_blue_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_green
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_green", "Asset/Texture/sponza/fabric/fabric_green_albedo.tga", "Asset/Texture/sponza/fabric/fabric_green_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_red
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_red", "Asset/Texture/sponza/fabric/fabric_red_albedo.tga", "Asset/Texture/sponza/fabric/fabric_red_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//flagpole
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("flagpole", "Asset/Texture/sponza/flagpole/flagpole_albedo.tga", "Asset/Texture/sponza/flagpole/flagpole_spec.tga", "Asset/Texture/sponza/flagpole/flagpole_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//floor
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("floor", "Asset/Texture/sponza/floor/floor_albedo.tga", "Asset/Texture/sponza/floor/floor_spec.tga", "Asset/Texture/sponza/floor/floor_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//lion
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("lion", "Asset/Texture/sponza/lion/lion_albedo.tga", "Asset/Texture/sponza/lion/lion_spec.tga", "Asset/Texture/sponza/lion/lion_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//lion_back
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("lion_back", "Asset/Texture/sponza/lion_background/lion_background_albedo.tga", "Asset/Texture/sponza/lion_background/lion_background_spec.tga", "Asset/Texture/sponza/lion_background/lion_background_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//plant
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("plant", "Asset/Texture/sponza/plant/vase_plant_albedo.tga", "Asset/Texture/sponza/plant/vase_plant_spec.tga", "Asset/Texture/sponza/plant/vase_plant_norm.tga", "Asset/Texture/sponza/plant/vase_plant_emiss.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//roof
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("roof", "Asset/Texture/sponza/roof/roof_albedo.tga", "Asset/Texture/sponza/roof/roof_spec.tga", "Asset/Texture/sponza/roof/roof_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//thorn
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("thorn", "Asset/Texture/sponza/thorn/sponza_thorn_albedo.tga", "Asset/Texture/sponza/thorn/sponza_thorn_spec.tga", "Asset/Texture/sponza/thorn/sponza_thorn_norm.tga", "Asset/Texture/sponza/thorn/sponza_thorn_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase", "Asset/Texture/sponza/vase/vase_albedo.tga", "Asset/Texture/sponza/vase/vase_spec.tga", "Asset/Texture/sponza/vase/vase_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase_hanging
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase_hanging", "Asset/Texture/sponza/vase_hanging/vase_hanging_albedo.tga", "Asset/Texture/sponza/vase_hanging/vase_round_spec.tga", "Asset/Texture/sponza/vase_hanging/vase_round_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase_round
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase_round", "Asset/Texture/sponza/vase_hanging/vase_round_albedo.tga", "Asset/Texture/sponza/vase_hanging/vase_round_spec.tga", "Asset/Texture/sponza/vase_hanging/vase_round_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("curtain_red")); //missing 1
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("thorn"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("vase_round"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("plant"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("lion_back"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("bricks"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("arch"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("ceiling"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("column_a"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("floor"));

			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("column_c"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("detail"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("column_b"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("lion_back")); //missing 2
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("flagpole"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("fabric_red"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("fabric_blue"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("fabric_green"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("curtain_red"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("curtain_green"));

			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("curtain_blue"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("chain"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("vase_hanging"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("vase"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("lion"));
			sponza->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("roof"));

			AssetDatabase::GetInstance()->objectManager.push_back(sponza);
		}

		//cerberus
		/*
		{
			Object *cerberus = new Object;
			cerberus->initialize(vulkanApp, "Cerberus", "Asset/Object/Cerberus/Cerberus.obj", true);
			cerberus->scale = glm::vec3(1.5f);
			cerberus->position = glm::vec3(0.0f, 3.0f, 0.0f);

			cerberus->updateObjectBuffer();

			//Texture			
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/Cerberus/Cerberus_A.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/Cerberus/Cerberus_E.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/Cerberus/Cerberus_N.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/Cerberus/Cerberus_S.tga");


			//Material
			glm::vec2 screenOffsets = glm::vec2(0.0);
			glm::vec4 sizeScale = glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0);

			//Cerberus
			GbufferMaterial* temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("Cerberus", "Asset/Texture/Cerberus/Cerberus_A.tga", "Asset/Texture/Cerberus/Cerberus_S.tga",
				"Asset/Texture/Cerberus/Cerberus_N.tga", "Asset/Texture/Cerberus/Cerberus_E.tga",
				&cerberus->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			cerberus->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("Cerberus"));

			AssetDatabase::GetInstance()->objectManager.push_back(cerberus);
		}
		*/

		/*
		//Chromie
		{
			Object *Chromie = new Object;
			Chromie->initialize(vulkanApp, "Chromie", "Asset/Object/Chromie.obj", true);
			Chromie->scale = glm::vec3(0.1f);
			Chromie->position = glm::vec3(0.0f, -0.04f, 0.0f);
			Chromie->updateOrbit(0.0f, 85.0f, 0.0);
			Chromie->updateObjectBuffer();
			Chromie->bRoll = true;
			//Texture			
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/storm_hero_chromie_ultimate_diff.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/storm_hero_chromie_ultimate_spec.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/storm_hero_chromie_ultimate_norm.tga");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/storm_hero_chromie_ultimate_emis.tga");

			//Material
			glm::vec2 screenOffsets = glm::vec2(0.0);
			glm::vec4 sizeScale = glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0);

			//Chromie
			GbufferMaterial* temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("Chromie", "Asset/Texture/storm_hero_chromie_ultimate_diff.tga", "Asset/Texture/storm_hero_chromie_ultimate_spec.tga",
				"Asset/Texture/storm_hero_chromie_ultimate_norm.tga", "Asset/Texture/storm_hero_chromie_ultimate_emis.tga",
				&Chromie->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			Chromie->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("Chromie"));

			AssetDatabase::GetInstance()->objectManager.push_back(Chromie);
		}
		*/

		//Lion
		{
			Object *Lion = new Object;
			Lion->initialize(vulkanApp, "LionStatue", "Asset/Object/lionhh/lionhh.obj", true);
			Lion->scale = glm::vec3(0.08f);
			Lion->position = glm::vec3(0.0f, -0.1f, -0.5f);
			Lion->updateOrbit(0.0f, 90.0f, 0.0);
			Lion->updateObjectBuffer();
			//Lion->bRoll = true;
			//Texture			
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/lionhh/lion_albedo.png");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/lionhh/lion_specular.png");
			AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/Default_Normal.tga");
			//AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/sponza/no_emis.tga");

			//Material
			glm::vec2 screenOffsets = glm::vec2(0.0);
			glm::vec4 sizeScale = glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0);

			//Lion
			GbufferMaterial* temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("LionStatue_Mat", "Asset/Texture/lionhh/lion_albedo.png", "Asset/Texture/lionhh/lion_specular.png",
				"Asset/Texture/Default_Normal.tga", "Asset/Texture/sponza/no_emis.tga",
				&Lion->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			Lion->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("LionStatue_Mat"));

			AssetDatabase::GetInstance()->objectManager.push_back(Lion);
		}
		
	}
}

void Renderer::setGlobalLights()
{
	PointLight *pPoint01 = new PointLight;

	pPoint01->lightInfo.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	pPoint01->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 20.0);
	pPoint01->position = glm::vec3(0.0, 5.0, 0.0);
	pPoint01->lightInfo.worldPos = glm::vec4(pPoint01->position, 1.0);

	pointLights.push_back(pPoint01);

	pointLightInfo.push_back(pPoint01->lightInfo);


	PointLight *pPoint02 = new PointLight;

	pPoint02->lightInfo.color = glm::vec4(1.0, 0.6, 0.3, 3.0);
	pPoint02->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 3.0);
	pPoint02->position = glm::vec3(4.9, 1.5, 1.5);
	pPoint02->lightInfo.worldPos = glm::vec4(pPoint02->position, 1.0);

	pointLights.push_back(pPoint02);

	pointLightInfo.push_back(pPoint02->lightInfo);


	PointLight *pPoint03 = new PointLight;

	pPoint03->lightInfo.color = glm::vec4(1.0, 0.6, 0.3, 3.0);
	pPoint03->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 3.0);
	pPoint03->position = glm::vec3(4.9, 1.5, -2.0);
	pPoint03->lightInfo.worldPos = glm::vec4(pPoint03->position, 1.0);

	pointLights.push_back(pPoint03);

	pointLightInfo.push_back(pPoint03->lightInfo);


	PointLight *pPoint04 = new PointLight;

	pPoint04->lightInfo.color = glm::vec4(1.0, 0.6, 0.3, 3.0);
	pPoint04->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 3.0);
	pPoint04->position = glm::vec3(-5.9, 1.5, 1.5);
	pPoint04->lightInfo.worldPos = glm::vec4(pPoint04->position, 1.0);

	pointLights.push_back(pPoint04);

	pointLightInfo.push_back(pPoint04->lightInfo);


	PointLight *pPoint05 = new PointLight;

	pPoint05->lightInfo.color = glm::vec4(1.0, 0.6, 0.3, 3.0);
	pPoint05->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 3.0);
	pPoint05->position = glm::vec3(-5.9, 1.5, -2.0);
	pPoint05->lightInfo.worldPos = glm::vec4(pPoint05->position, 1.0);

	pointLights.push_back(pPoint05);

	pointLightInfo.push_back(pPoint05->lightInfo);


	PointLight *pPoint06 = new PointLight;

	pPoint06->lightInfo.color = glm::vec4(1.0, 0.0, 0.0, 3.0);
	pPoint06->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 6.0);
	pPoint06->position = glm::vec3(11.0, 1.5, 4.0);
	pPoint06->lightInfo.worldPos = glm::vec4(pPoint06->position, 1.0);

	pointLights.push_back(pPoint06);
	pointLightInfo.push_back(pPoint06->lightInfo);



	PointLight *pPoint07 = new PointLight;

	pPoint07->lightInfo.color = glm::vec4(0.0, 1.0, 0.0, 3.0);
	pPoint07->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 6.0);
	pPoint07->position = glm::vec3(11.0, 1.5, -4.5);
	pPoint07->lightInfo.worldPos = glm::vec4(pPoint07->position, 1.0);

	pointLights.push_back(pPoint07);
	pointLightInfo.push_back(pPoint07->lightInfo);



	PointLight *pPoint08 = new PointLight;

	pPoint08->lightInfo.color = glm::vec4(0.0, 0.0, 1.0, 3.0);
	pPoint08->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 6.0);
	pPoint08->position = glm::vec3(-12.0, 1.5, 4.0);
	pPoint08->lightInfo.worldPos = glm::vec4(pPoint08->position, 1.0);

	pointLights.push_back(pPoint08);
	pointLightInfo.push_back(pPoint08->lightInfo);


	PointLight *pPoint09 = new PointLight;

	pPoint09->lightInfo.color = glm::vec4(1.0, 0.0, 0.0, 3.0);
	pPoint09->lightInfo.direction = glm::vec4(0.0, 0.0, 0.0, 6.0);
	pPoint09->position = glm::vec3(-12.0, 1.5, -4.5);
	pPoint09->lightInfo.worldPos = glm::vec4(pPoint09->position, 1.0);

	pointLights.push_back(pPoint09);
	pointLightInfo.push_back(pPoint09->lightInfo);

	createPointLightBuffer();

	//Sky System
	directionalLights.clear();

	skySystem.sun.updateOrbit(45.0f, 0.0f, 0.0);
	skySystem.sun.lightInfo.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	skySystem.sun.lightInfo.direction = skySystem.sun.getViewVector4();

	directionalLights.push_back(&skySystem.sun);
	directionalLightInfo.push_back(skySystem.sun.lightInfo);

	createDirectionalLightBuffer();
}

void Renderer::initialize(Vulkan* pVulkanApp)
{
	interface.initWindow();

	if (pVulkanApp == NULL)
	{
		pVulkanApp = new Vulkan;
		pVulkanApp->initialize(interface);
	}

	vulkanApp = pVulkanApp;

	//gui.initGUI(vulkanApp);
	
	createSemaphores();
	createQueues();

	createSwapChain();
	createSwapChainImageViews();	

	createSingleTriangularVertexBuffer();

	createGbufferCommandPool();
	createFrustumCullingCommandPool();

	//createGUICommandPool();

	createMainCommandPool();	

	createGbuffers();

	depthTexture = new Texture;
	depthTexture->vulkanApp = vulkanApp;

	SSRDepthTexture = new Texture;
	SSRDepthTexture->vulkanApp = vulkanApp;

	//createGUICanvas();

	/*
	depthMipmapTexture = new Texture;
	depthMipmapTexture->vulkanApp = vulkanApp;
	
	depthMipTexture.resize(DEPTH_MIP_SIZE);

	for (size_t i = 0; i < depthMipTexture.size(); i++)
	{
		depthMipTexture[i] = new Texture;
		depthMipTexture[i]->vulkanApp = vulkanApp;
	}
	

	depthMipSizeBuffer.resize(DEPTH_MIP_SIZE);
	depthMipSizeBufferMemory.resize(DEPTH_MIP_SIZE);
	*/

	createDepthResources();
	createSSRDepthResources();

	createGbufferRenderPass();
	createMainRenderPass();

	//initialize Resurces
	AssetDatabase::GetInstance()->setVulkanApp(vulkanApp);

	mainCamera.setCamera(vulkanApp, mainCamera.position, mainCamera.focusPosition, mainCamera.upVector,  45.0f,
		static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), NEAR_PLANE, FAR_PLANE);

	
	mainCamera.updateOrbit(90.0f, -3.0f, 0.0f);
	//mainCamera.updatePosition(0.4f, 0.4f, 10.0f);
	mainCamera.updatePosition(1.5f, 0.4f, 11.0f);
	mainCamera.updateOrbit(15.0f, 0.0f, 0.0f);
	

	//FrustumCullingMaterial
	{
		FrustumCullingMaterial* frustumCulling_Mat = new FrustumCullingMaterial;
		frustumCulling_Mat->createPipeline("frustumCulling", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL, glm::vec2(0.0), glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0),
			 NULL,NULL, NULL);

		pfrustumCullingMaterial = frustumCulling_Mat;
		// dynamic_cast<FrustumCullingMaterial*>(DBInstance->FindAsset<Material>("frustumCulling"));
	}

	//updateDepthMipmapBuffers();

	/*
	//Depth Mipmap
	{
		//VkFormat depthFormat = vulkanApp->findDepthFormat();

		VkFormat depthFormat = VK_FORMAT_R32_SFLOAT;

		for (int d = 0; d < DEPTH_MIP_SIZE; d++)
		{
			DepthMipmapMaterial* depth_mipmap_Mat = new DepthMipmapMaterial;

			PostProcess *Depth_mip_PP = new PostProcess(vulkanApp, "depth_mat_" + std::to_string(d), depthFormat, 1, singleTriangularVertexBuffer, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, false);

			//for ReleaseMode
			glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width)/(uint32_t)glm::max(pow(2, d-1), 1.0), static_cast<float>(swapChainExtent.height)/ (uint32_t)glm::max(pow(2, d - 1), 1.0), 1.0f, 1.0f);

			Depth_mip_PP->initialize(sizeScale);

			depth_mipmap_Mat->createPipeline("depth_mat_" + std::to_string(d), "", "", "", "", NULL, &depthMipSizeBuffer[(uint32_t)glm::max(float(d-1), 0.0f)],
				NULL, 0, NULL, 0, NULL,
				glm::vec2(0.0), Depth_mip_PP->sizeScale, Depth_mip_PP->getRenderPass(), NULL, d == 0 ? depthTexture : postProcessChain[DEPTH_MIP_POSTPROCESS + d-1]->renderTargets[0]); // !!!!!
			assignRenderpassID(depth_mipmap_Mat, Depth_mip_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

			Depth_mip_PP->recordCommandBuffer();

			postProcessChain.push_back(Depth_mip_PP);
		}
	}
	*/

	setGlobalObjs();
	setGlobalLights();	

	createPerFrameBuffer();

	//PBR material
	{
		UberMaterial* temp_uber_Mat = new UberMaterial;
		
		PostProcess *PBR_PP = new PostProcess(vulkanApp, "uber_mat", VK_FORMAT_R16G16B16A16_SFLOAT, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);

		PBR_PP->initialize(sizeScale);

		temp_uber_Mat->createPipeline("uber_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(),&directionalLightUniformBuffer, directionalLightInfo.size(), NULL,
			glm::vec2(0.0), PBR_PP->sizeScale, PBR_PP->getRenderPass(), &gbuffers, depthTexture);
		assignRenderpassID(temp_uber_Mat, PBR_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		PBR_PP->recordCommandBuffer();

		postProcessChain.push_back(PBR_PP);
	}

	PlaneInfoPack planeInfoPack;
	planeInfoPack.numPlanes = 1;

	planeInfoPack.planeInfo[0].centerPoint = glm::vec4(0.0, 0.0, 0.0, 0.0);
	planeInfoPack.planeInfo[0].size = glm::vec4(100.0);

	//planeInfoPack.planeInfo[0].centerPoint = glm::vec4(4.5, 0.0, -0.5, 0.0);
	//planeInfoPack.planeInfo[0].size = glm::vec4(7.0, 1.5, 0.0, 0.0);

	planeInfoPack.planeInfo[1].centerPoint = glm::vec4(4.5, 0.3, 1.0, 0.0);
	planeInfoPack.planeInfo[1].size = glm::vec4(7.0, 1.5, 0.0, 0.0);

	planeInfoPack.planeInfo[2].centerPoint = glm::vec4(4.5, 0.3, -2.0, 0.0);
	planeInfoPack.planeInfo[2].size = glm::vec4(7.0, 1.5, 0.0, 0.0);

	glm::mat4 basicMat;
	basicMat[0] = glm::vec4(1.0, 0.0, 0.0, 0.0); //tan
	basicMat[1] = glm::vec4(0.0, 1.0, 0.0, 0.0); //bitan
	basicMat[2] = glm::vec4(0.0, 0.0, 1.0, 0.0); //normal
	basicMat[3] = glm::vec4(0.0, 0.0, 0.0, 1.0);

	planeInfoPack.planeInfo[0].rotMat = glm::rotate(basicMat, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));

	planeInfoPack.planeInfo[1].rotMat = glm::rotate(basicMat, glm::radians(-110.0f), glm::vec3(1.0, 0.0, 0.0));

	planeInfoPack.planeInfo[2].rotMat = glm::rotate(basicMat, glm::radians(-70.0f), glm::vec3(1.0, 0.0, 0.0));

	createSSRInfoBuffer();

	if (interface.bUseBruteForce)
	{
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0, 1.0);

		BruteForceMaterial * BR_Mat = new BruteForceMaterial;

		PostProcess *BR_PP = new PostProcess(vulkanApp, "BR_mat", VK_FORMAT_R16G16B16A16_SFLOAT, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		BR_PP->initialize(sizeScale);

		//BR_Mat->addBuffer(&(BR_PP->planeInfoBuffer));
		BR_Mat->addBuffer(&SSRInfoBuffer);

		std::vector<Texture*> tempRenderTargets;

		tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]);  //Scene
		//tempRenderTargets.push_back( gbuffers[NORMAL_COLOR]);  //world Normal

		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_albedo.tga"));
		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_spec.tga"));
		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_norm.tga"));


		BR_Mat->createPipeline("BR_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), NULL,
			glm::vec2(0.0), BR_PP->sizeScale, BR_PP->getRenderPass(), &tempRenderTargets, depthTexture);
		assignRenderpassID(BR_Mat, BR_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		BR_Mat->updatePlaneInfoPackBuffer(planeInfoPack);

		BR_PP->recordCommandBuffer();

		postProcessChain.push_back(BR_PP);

	}
	else //SSR material
	{		
		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0, 1.0);

		//SSRP
		ScreenSpaceProjectionMaterial* SSRP_Mat = new ScreenSpaceProjectionMaterial;
		//ScreenSpaceProjectionMaterial2* SSRP_Mat = new ScreenSpaceProjectionMaterial2;

		PostProcess *SSRP_PP = new PostProcess(vulkanApp, "ssrp_mat", VK_FORMAT_R32_UINT, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, true, 1);
		//PostProcess *SSRP_PP = new PostProcess(vulkanApp, "ssrp_mat", VK_FORMAT_R8G8B8A8_UNORM, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		SSRP_PP->initialize(sizeScale);
		
		std::vector<Texture*> tempSSRPRenderTargets;
		tempSSRPRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //Scene image
		tempSSRPRenderTargets.push_back(SSRP_PP->renderTargets[0]); //Scene image

		
		SSRP_Mat->createPipeline("ssrp_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL,
			glm::vec2(0.0), sizeScale,
			NULL, &tempSSRPRenderTargets, depthTexture);

		

		SSRP_Mat->updatePlaneInfoPackBuffer(planeInfoPack);

		SSRP_PP->recordCommandBuffer();

		postProcessChain.push_back(SSRP_PP);


		//SSR
		

		std::string noiseTex = "Asset/Texture/noise/SSR_Noise.tga";
		AssetDatabase::GetInstance()->SaveTexture(noiseTex);

		ScreenSpaceReflectionMaterial* temp_ssr_Mat = new ScreenSpaceReflectionMaterial;

		PostProcess *SSR_PP = new PostProcess(vulkanApp, "ssr_mat", VK_FORMAT_R16G16B16A16_SFLOAT, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);



		SSR_PP->initialize(sizeScale);

		temp_ssr_Mat->addBuffer(&(SSRP_Mat->planeInfoBuffer));
		temp_ssr_Mat->addBuffer(&SSRInfoBuffer);

		std::vector<Texture*> tempRenderTargets;

		tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 2]->renderTargets[0]);  //Scene
		tempRenderTargets.push_back(SSRP_PP->renderTargets[0]);  //Scene

		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_albedo.tga"));
		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_spec.tga"));
		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>("Asset/Texture/sponza/floor/floor_norm.tga"));
		tempRenderTargets.push_back(AssetDatabase::GetInstance()->LoadAsset<Texture>(noiseTex));


		temp_ssr_Mat->createPipeline("ssr_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), NULL,
			glm::vec2(0.0), SSR_PP->sizeScale, SSR_PP->getRenderPass(), &tempRenderTargets, depthTexture);
		assignRenderpassID(temp_ssr_Mat, SSR_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		glm::vec4 info = glm::vec4(1.0, 1.0, 1.0, 1.0);
		

		SSR_PP->recordCommandBuffer();

		postProcessChain.push_back(SSR_PP);	
	
		//HolePatching
		HolePatchingMaterial * temp_hole_Mat = new HolePatchingMaterial;

		PostProcess *HPP = new PostProcess(vulkanApp, "HolePatchingMat", VK_FORMAT_R16G16B16A16_SFLOAT, 1,
			singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		//for ReleaseMode
		//glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);

		HPP->initialize(sizeScale);

		std::vector<Texture*> tempRenderTargets2;

		tempRenderTargets2.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //SSR

		temp_hole_Mat->addBuffer(&SSRInfoBuffer);

		temp_hole_Mat->createPipeline("HolePatchingMat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), &perFrameBuffer,
			glm::vec2(0.0), glm::vec4(HPP->getExtent().width, HPP->getExtent().height, 1.0, 1.0), HPP->getRenderPass(), &tempRenderTargets2, NULL);

		

		assignRenderpassID(temp_hole_Mat, HPP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		HPP->recordCommandBuffer();

		postProcessChain.push_back(HPP);
	}

	
	

	/*
	//Blur
	{
		HorizontalBlurMaterial * temp_horizon_Mat = new HorizontalBlurMaterial;

		PostProcess *HBP = new PostProcess(vulkanApp, "HorizonBlurMat", VK_FORMAT_R16G16B16A16_SFLOAT, 1,
			singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);

		HBP->initialize(sizeScale);

		std::vector<Texture*> tempRenderTargets;
		
		tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //SSR

		temp_horizon_Mat->createPipeline("HorizonBlurMat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), &perFrameBuffer,
			glm::vec2(0.0), glm::vec4(HBP->getExtent().width, HBP->getExtent().height, 1.0, 1.0), HBP->getRenderPass(), &tempRenderTargets, NULL);

		assignRenderpassID(temp_horizon_Mat, HBP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		HBP->recordCommandBuffer();

		postProcessChain.push_back(HBP);
	}

	/*

	//Blur
	{
		VerticalBlurMaterial * temp_vertical_Mat = new VerticalBlurMaterial;

		PostProcess *VBP = new PostProcess(vulkanApp, "VerticalBlurMat", VK_FORMAT_R16G16B16A16_SFLOAT, 1,
			singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);

		VBP->initialize(sizeScale);

		std::vector<Texture*> tempRenderTargets;

		tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //SSR

		temp_vertical_Mat->createPipeline("VerticalBlurMat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), &perFrameBuffer,
			glm::vec2(0.0), glm::vec4(VBP->getExtent().width, VBP->getExtent().height, 1.0, 1.0), VBP->getRenderPass(), &tempRenderTargets, NULL);

		assignRenderpassID(temp_vertical_Mat, VBP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		VBP->recordCommandBuffer();

		postProcessChain.push_back(VBP);
	}
	
	*/

	skySystem.lowFreqTexture->connectDevice(pVulkanApp);
	skySystem.lowFreqTexture->loadTexture2DArrayImage("Asset/Texture/cloudTextures/lowFreq/lowFreq", ".tga");

	skySystem.highFreqTexture->connectDevice(pVulkanApp);
	skySystem.highFreqTexture->loadTexture2DArrayImage("Asset/Texture/cloudTextures/highFreq/highFreq", ".tga");

	AssetDatabase::GetInstance()->SaveTexture("Asset/Texture/cloudTextures/wheather.tga");

	/*
	//SkyRendering material
	{			
		SkyRenderingMaterial* temp_sky_Mat = new SkyRenderingMaterial;

		PostProcess *SKY_PP = new PostProcess(vulkanApp, "SkyRendering_mat", VK_FORMAT_R16G16B16A16_SFLOAT, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);
		
		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);
		
		SKY_PP->initialize(sizeScale);

		std::vector<Texture*> tempRenderTargets;

		tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]);
		tempRenderTargets.push_back(skySystem.lowFreqTexture);
		tempRenderTargets.push_back(skySystem.highFreqTexture);

		temp_sky_Mat->createPipeline("SkyRendering_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), &perFrameBuffer,
			glm::vec2(0.0), glm::vec4(SKY_PP->getExtent().width, SKY_PP->getExtent().height, 1.0, 1.0), SKY_PP->getRenderPass(), &tempRenderTargets, NULL);
		assignRenderpassID(temp_sky_Mat, SKY_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		SKY_PP->recordCommandBuffer();

		postProcessChain.push_back(SKY_PP);
	}
	*/

	//Composite Post Process
	{
		CompositePostProcessMaterial* temp_cpp_Mat = new CompositePostProcessMaterial;

		PostProcess *C_PP = new PostProcess(vulkanApp, "CompositePostProcessMat", VK_FORMAT_R16G16B16A16_SFLOAT, 1,
			singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false, 1);

		createSSRBuffer();

		temp_cpp_Mat->addBuffer(&SSRDepthBuffer);

		//for ReleaseMode
		glm::vec4 sizeScale = glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f);

		C_PP->initialize(sizeScale);

		std::vector<Texture*> tempRenderTargets;

		if (interface.bUseBruteForce)
		{
			tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 2]->renderTargets[0]); //Scene
			tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //SSR
		}
		else
		{
			tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 4]->renderTargets[0]); //Scene
			tempRenderTargets.push_back(postProcessChain[postProcessChain.size() - 1]->renderTargets[0]); //SSR
		}

		

		temp_cpp_Mat->createPipeline("CompositePostProcessMat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(), &directionalLightUniformBuffer, directionalLightInfo.size(), &perFrameBuffer,
			glm::vec2(0.0), glm::vec4(C_PP->getExtent().width, C_PP->getExtent().height, 1.0, 1.0), C_PP->getRenderPass(), &tempRenderTargets, NULL);

		assignRenderpassID(temp_cpp_Mat, C_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		C_PP->recordCommandBuffer();

		postProcessChain.push_back(C_PP);
	}

	/*
	//ToneMapping material
	{
		ToneMappingMaterial* temp_tone_Mat = new ToneMappingMaterial;

		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VK_FORMAT_R16G16B16A16_SFLOAT
		PostProcess *tone_PP = new PostProcess(vulkanApp, "tonemapping_mat", swapChainImageFormat, 1, singleTriangularVertexBuffer, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR);
		tone_PP->initialize(glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 1.0f, 1.0f));

		temp_tone_Mat->createPipeline("tonemapping_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(),
			glm::vec2(0.0), tone_PP->sizeScale, tone_PP->getRenderPass(), &postProcessChain[postProcessChain.size() - 1]->renderTargets, NULL);
		assignRenderpassID(temp_tone_Mat, tone_PP->getRenderPass(), static_cast<uint32_t>(postProcessChain.size()));

		tone_PP->recordCommandBuffer();

		postProcessChain.push_back(tone_PP);
	}
	*/

	//Present material
	{
		PresentMaterial* temp_last_Mat = new PresentMaterial;

		temp_last_Mat->createPipeline("present_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), NULL,
			glm::vec2(0.0), glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0), mainRenderPass, &postProcessChain[postProcessChain.size()-1]->renderTargets, NULL);
		assignRenderpassID(temp_last_Mat, mainRenderPass, 0);
	}

	/*
	std::vector<Texture*> guiCanvas;
	std::vector<VkFramebuffer> guiFramebuffers;

	std::vector<VkImageView> guiImageViews;
	guiImageViews.resize(guiCanvas.size());

	for (size_t i = 0; i < guiImageViews.size(); i++)
	{
		guiImageViews[i] = guiCanvas[i]->textureImageView;
	}
	
	createGUIFrameBuffers();	
	*/

	createGbufferFramebuffers();
	createMainFramebuffers();
	

	createGbufferCommandBuffers();
	createFrustumCullingCommandBuffers();
	createMainCommandBuffers();

	//createGUICommandBuffers();


	//record static CommandBuffers;
	recordMainCommandBuffers();
	//recordGUICommandBuffers();
}

/*
void Renderer::updateDepthMipmapBuffers()
{
	for (int d = 0; d < DEPTH_MIP_SIZE; d++)
	{
		glm::vec4 bufferInfo;
		bufferInfo = glm::vec4(static_cast<float>(swapChainExtent.width) / (uint32_t)pow(2, d), static_cast<float>(swapChainExtent.height) / (uint32_t)pow(2, d), 0.0, 0.0);
		
		if (d == 0)
			bufferInfo.w = 1.0f;

		vulkanApp->updateBuffer(&bufferInfo, depthMipSizeBufferMemory[d], sizeof(glm::vec4));
	}
}
*/

void Renderer::createPointLightBuffer()
{
	vulkanApp->createBuffer(sizeof(LightInfo) * pointLightInfo.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		pointLightUniformBuffer, pointLightUniformMemory);
	
	updatePointLightBuffer();
}

void Renderer::updatePointLightBuffer()
{
	vulkanApp->updateBuffer(pointLightInfo.data(), pointLightUniformMemory, sizeof(LightInfo)* pointLightInfo.size());
}

void Renderer::createDirectionalLightBuffer()
{
	vulkanApp->createBuffer(sizeof(LightInfo) * directionalLightInfo.size() , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		directionalLightUniformBuffer, directionalLightUniformMemory);

	updateDirectionalLightBuffer();
}

void Renderer::updateDirectionalLightBuffer()
{
	vulkanApp->updateBuffer(&directionalLights[0]->lightInfo, directionalLightUniformMemory, sizeof(LightInfo) * directionalLightInfo.size());
}

void Renderer::createSSRBuffer()
{
	VkDeviceSize bufferSize = sizeof(uint32_t);

	vulkanApp->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		SSRDepthBuffer, SSRDepthBufferMemory);
}

void Renderer::updateSSRBuffer()
{
	VkDeviceSize bufferSize = sizeof(uint32_t);
	
	vulkanApp->updateBuffer(&interface.SSRVisibility, SSRDepthBufferMemory, sizeof(uint32_t));
}

void Renderer::createSSRInfoBuffer()
{
	vulkanApp->createBuffer(sizeof(glm::vec4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		SSRInfoBuffer, SSRInfoBufferMem);

	updateSSRInfoBuffer();
}

void Renderer::updateSSRInfoBuffer()
{
	glm::vec4 tempSSRInfo = glm::vec4(interface.gRoughness, interface.gIntensity, interface.bUseNormalMap == true ? 1.0 : 0.0, interface.bUseHolePatching == true ? 1.0 : 0.0);
	vulkanApp->updateBuffer(&tempSSRInfo, SSRInfoBufferMem, sizeof(glm::vec4));
}

void Renderer::createPerFrameBuffer()
{
	vulkanApp->createBuffer(sizeof(perframeBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		perFrameBuffer, perFrameBufferMemory);

	updatePerFrameBuffer();

}

void Renderer::updatePerFrameBuffer()
{
	perframeBuffer perFrameBuffer;
	perFrameBuffer.timeInfo = glm::vec4(currentTime*0.001f, deltaTime*0.001f, 0.0f, 0.0f);
	vulkanApp->updateBuffer(&perFrameBuffer, perFrameBufferMemory, sizeof(perframeBuffer));
}




void Renderer::reInitializeRenderer()
{
	releaseRenderPart();
	shutdownDepthResources();

	AssetDatabase::GetInstance()->cleanUp();

	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		postProcessChain[i]->shutDown();
	}

	postProcessChain.clear();

	deleteSemaphores();

	vkDestroyCommandPool(vulkanApp->getDevice(), gbufferCmdPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), frustumCullingPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), mainCmdPool, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), singleTriangularVertexBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), singleTriangularVertexMemory, nullptr);

	mainCamera.shutDown();

	//delete[] SSROforReset;
	//delete[] SSRDforReset;
	
	vkDestroyBuffer(vulkanApp->getDevice(), SSRDepthBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), SSRDepthBufferMemory, nullptr);

	//vkDestroyBuffer(vulkanApp->getDevice(), SSROffsetBuffer, nullptr);
	//vkFreeMemory(vulkanApp->getDevice(), SSROffsetBufferMemory, nullptr);


	vkDestroyBuffer(vulkanApp->getDevice(), directionalLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), directionalLightUniformMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), pointLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), pointLightUniformMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), perFrameBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), perFrameBufferMemory, nullptr);

	DELETE_SAFE(vulkanApp);

	initialize(NULL);

	interface.windowResetFlag = false;
}

void Renderer::culling()
{
	AssetDatabase* DBInstance = AssetDatabase::GetInstance();

	//Culling
	for (size_t i = 0; i < DBInstance->objectManager.size(); i++)
	{
		Object *thisOBJ = DBInstance->objectManager[i];

		glm::mat4 modelViewMat = mainCamera.viewMat * thisOBJ->modelMat;

		BoundingBox viewAABB = mainCamera.getViewAABB(thisOBJ->AABB, modelViewMat);

		if (mainCamera.frustum.checkBox(viewAABB))
		{
			thisOBJ->AABB.cullingInfo.x = 0.0f;

			//ViewFrustum culling with ComputeShader
			if (USE_GPU_CULLING)
			{
				objectBuffer thisUBO;
				thisUBO.modelMat = thisOBJ->modelMat;
				thisUBO.InvTransposeMat = thisOBJ->InvTransposeMat;

				pfrustumCullingMaterial->updateLocalBuffer(thisOBJ->geomAABB, static_cast<int>(i), mainCamera.frustum.frustumInfo, thisUBO);

				recordFrustumCullingCommandBuffers(static_cast<int>(thisOBJ->geomAABB.size()), 1, 1);

				//ViewFrustum culling
				VkSubmitInfo submitInfo = {};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				submitInfo.waitSemaphoreCount = 0;
				submitInfo.pWaitSemaphores = NULL;
				submitInfo.pWaitDstStageMask = NULL;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &frustumCmd[0];
				submitInfo.signalSemaphoreCount = 0;
				submitInfo.pSignalSemaphores = NULL;

				if (vkQueueSubmit(frustumQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to submit draw command buffer!");
				}

				pfrustumCullingMaterial->mapCullingInfo(thisOBJ->geomAABB);

				vkQueueWaitIdle(frustumQueue);
			}
			else
			{
				for (size_t j = 0; j < thisOBJ->geoms.size(); j++)
				{
					Geometry *thisGeom = thisOBJ->geoms[j];
					BoundingBox viewGeoAABB = mainCamera.getViewAABB(thisGeom->AABB, modelViewMat);

					if (mainCamera.frustum.checkBox(viewGeoAABB))
					{
						thisOBJ->AABB.cullingInfo.x = 0.0f;
					}
					else
						thisOBJ->AABB.cullingInfo.x = 1.0f;
				}
			}
		}
		else
			thisOBJ->AABB.cullingInfo.x = 1.0f;
	}
}

void Renderer::mainloop()
{
	unsigned int simulationTime = 0;

	//mainCamera.updateOrbit(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(interface.getWindow()))
	{
		glfwPollEvents();

		if (interface.windowResetFlag)
		{
			reInitializeRenderer();
		}

		unsigned int realTime = timer.getTime();
		deltaTime = realTime - previousTime;
		currentTime += deltaTime;


		interface.fpstracker++;

		double fpsElapsedTime = (realTime - fpsPreviosTime) * 0.001;

		if (fpsElapsedTime >= 1.0)
		{

			interface.fps = static_cast<int>(interface.fpstracker / fpsElapsedTime);
			interface.fpstracker = 0;
			fpsPreviosTime = realTime;

			std::string title = "Jin Engine | " + std::to_string(interface.fps) + " fps | " + std::to_string(1000.0 / (double)interface.fps) + " ms";
			interface.setWindowTitle(title);
		}


		double sensitivity = 20.0;
		double deltaTimeSec = deltaTime * 0.001;
		double currentTimeSec = currentTime * 0.001;

		if (interface.mouseDeltaX != 0.0 || interface.mouseDeltaY != 0.0 || interface.mouseDeltaZ != 0.0)
		{
			mainCamera.updateOrbit(static_cast<float>(interface.mouseDeltaX * deltaTimeSec * sensitivity),
				static_cast<float>(interface.mouseDeltaY * deltaTimeSec * sensitivity),
					static_cast<float>(interface.mouseDeltaZ * deltaTimeSec * sensitivity));
			
			interface.mouseDeltaX = 0.0;
			interface.mouseDeltaY = 0.0;
			interface.mouseDeltaZ = 0.0;
		}
		
		interface.getAsynckeyState();

		if( (interface.bFoward || interface.bBackward)) 
		{
			if (interface.bFoward)
			{
				mainCamera.updatePosition(0.0f, 0.0f, -static_cast<float>(deltaTimeSec * sensitivity));
			}
			else if (interface.bBackward)
			{
				mainCamera.updatePosition(0.0f, 0.0f, static_cast<float>(deltaTimeSec * sensitivity));
			}

			interface.bFoward = false;
			interface.bBackward = false;
		}

		if ((interface.bLeft || interface.bRight))
		{
			if (interface.bLeft)
			{
				mainCamera.updatePosition(-static_cast<float>(deltaTimeSec * sensitivity), 0.0f, 0.0f);
			}
			else if (interface.bRight)
			{
				mainCamera.updatePosition(static_cast<float>(deltaTimeSec * sensitivity), 0.0f, 0.0f);
			}

			interface.bLeft = false;
			interface.bRight = false;
		}

		if(interface.bRotate)
		{
			//interface.gRoughness = (glm::sin(currentTimeSec * 0.25f) + 1.f) * 0.3f;
			mainCamera.updateOrbit(static_cast<float>( glm::sin(-currentTimeSec * 0.25f) * deltaTimeSec * 9.0f), 0.0f, 0.0f);
		}

		if (interface.bMoveForward)
		{
			mainCamera.updatePosition(0.0f, 0.0f, static_cast<float>(glm::sin(currentTimeSec * 0.5f) * deltaTimeSec * 2.0f));
		}
		
		culling();

		AssetDatabase* DBInstance = AssetDatabase::GetInstance();

		for (size_t i = 0; i < DBInstance->objectManager.size(); i++)
		{
			Object *thisOBJ = DBInstance->objectManager[i];

			if (thisOBJ->bRoll)
			{
				thisOBJ->updateOrbit(0.0f, deltaTime * 0.05f, 0.0f);
				thisOBJ->updateObjectBuffer();
			}
		}

		//update SkySystem
		//skySystem.sun.updateOrbit(deltaTime * 0.05f, 0.0f, 0.0f);
		skySystem.sun.lightInfo.direction = skySystem.sun.getViewVector4();
		updateDirectionalLightBuffer();
		
		
		updateSSRBuffer();
		updateSSRInfoBuffer();
		


		updatePerFrameBuffer();

		//record it per everyframe but can do frustum culling
		recordGbufferCommandBuffers();
		

		draw(deltaTime);

		previousTime = realTime;
	}

	vkDeviceWaitIdle(vulkanApp->getDevice());
}

void Renderer::draw(unsigned int deltaTime)
{
	uint32_t imageIndex;
	int result = vkAcquireNextImageKHR(vulkanApp->getDevice(), swapChain, std::numeric_limits<uint64_t>::max(), gbufferSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		reCreateRenderer();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}


	//Draw G-buffer
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &gbufferSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gbufferCmd[0];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &pbrSemaphore;

	if (vkQueueSubmit(gbufferQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSemaphore *prevSM = &pbrSemaphore;
	VkSemaphore *currentSM = NULL;	
	
	//PostProcess
	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = prevSM;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &postProcessChain[i]->cmds[0];
		submitInfo.signalSemaphoreCount = 1;

		currentSM = postProcessChain[i]->getFirstSM();

		submitInfo.pSignalSemaphores = currentSM;

		if (vkQueueSubmit(postProcessChain[i]->getFirstQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		prevSM = currentSM;
	}

	//guiQueue
	/*
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = prevSM;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &guiCmd[0];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &guiSemaphore;

	vkQueueWaitIdle(guiQueue);

	if (vkQueueSubmit(guiQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	*/


	//frameQueue
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = prevSM;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mainCmd[imageIndex];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &presentSemaphore;

	vkQueueWaitIdle(pbrQueue);

	if (vkQueueSubmit(pbrQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	


	//presentQueue
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentSemaphore;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		vkDeviceWaitIdle(vulkanApp->getDevice());
		//reInitializeRenderer();
		reCreateRenderer();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkQueueWaitIdle(presentQueue);

}

void Renderer::reCreateRenderer()
{
	vkDeviceWaitIdle(vulkanApp->getDevice());
	
	releaseRenderPart();
	releaseDepthResources();
	releaseSSRDepthResources();

	createSwapChain();
	createSwapChainImageViews();	
	
	updateGbuffers();

	createDepthResources();
	createSSRDepthResources();

	//updateDepthMipmapBuffers();

	createGbufferRenderPass();
	createMainRenderPass();

	//gui.setRenderPass();
	//gui.setDescriptorPool();

	//resources
	mainCamera.updateAspectRatio(static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0, 0.0));

	AssetDatabase *pAssetDB = AssetDatabase::GetInstance();

	//postProcess
	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		postProcessChain[i]->initialize(glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), postProcessChain[i]->sizeScale.z, postProcessChain[i]->sizeScale.w));
	}

	

	
	//materials
	for (uint32_t i = 0; i < pAssetDB->materialList.size(); i++)
	{
		Material* pMaterial = pAssetDB->FindAsset<Material>(pAssetDB->materialList[i]);
		if(pMaterial->isComputeShader())
			pMaterial->updatePipeline(pMaterial->screenOffset, glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), pMaterial->sizeScale.z, pMaterial->sizeScale.w), getAssignedRenderpassID(pMaterial));
		else
			pMaterial->updatePipeline(pMaterial->screenOffset, glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), pMaterial->sizeScale.z, pMaterial->sizeScale.w), NULL);

	}
	

	//postProcess
	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		postProcessChain[i]->recordCommandBuffer();
	}

	

	createGbufferFramebuffers();
	createMainFramebuffers();
	//createGUIFrameBuffers();

	createGbufferCommandBuffers();
	createFrustumCullingCommandBuffers();
	createMainCommandBuffers();
	//createGUICommandBuffers();

	//record static CommandBuffers;
	recordMainCommandBuffers();
	//recordGUICommandBuffers();
}





void Renderer::releaseRenderPart()
{
	AssetDatabase::GetInstance()->releaseRenderingPart();

	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		postProcessChain[i]->releaseRenderingPart();
	}

	for (size_t i = 0; i < gbufferCmd.size(); i++)
	{
		vkFreeCommandBuffers(vulkanApp->getDevice(), gbufferCmdPool, 1, &gbufferCmd[i]);
		gbufferCmd[i] = NULL;
	}

	gbufferCmd.clear();

	vkDestroyRenderPass(vulkanApp->getDevice(), gbufferRenderPass, nullptr);


	for (size_t i = 0; i < frustumCmd.size(); i++)
	{
		vkFreeCommandBuffers(vulkanApp->getDevice(), frustumCullingPool, 1, &frustumCmd[i]);
		frustumCmd[i] = NULL;
	}

	frustumCmd.clear();


	for (size_t i = 0; i < gbufferFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanApp->getDevice(), gbufferFramebuffers[i], nullptr);
		gbufferFramebuffers[i] = NULL;
	}

	gbufferFramebuffers.clear();

	releaseGbuffers();

	/*
	releaseGUICanvas();

	for (size_t i = 0; i <  guiCmd.size(); i++)
	{
		vkFreeCommandBuffers(vulkanApp->getDevice(), guiCmdPool, 1, &guiCmd[i]);
		guiCmd[i] = NULL;
	}

	guiCmd.clear();

	gui.cleanUp();

	for (size_t i = 0; i < guiFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanApp->getDevice(), guiFramebuffers[i], nullptr);
		guiFramebuffers[i] = NULL;
	}

	guiFramebuffers.clear();
	*/


	for (size_t i = 0; i < mainCmd.size(); i++)
	{
		vkFreeCommandBuffers(vulkanApp->getDevice(), mainCmdPool, 1, &mainCmd[i]);
		mainCmd[i] = NULL;
	}

	mainCmd.clear();

	vkDestroyRenderPass(vulkanApp->getDevice(), mainRenderPass, nullptr);


	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanApp->getDevice(), swapChainFramebuffers[i], nullptr);
		swapChainFramebuffers[i] = NULL;
	}

	swapChainFramebuffers.clear();

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(vulkanApp->getDevice(), swapChainImageViews[i], nullptr);
		swapChainImageViews[i] = NULL;
	}

	swapChainImageViews.clear();

	vkDestroySwapchainKHR(vulkanApp->getDevice(), swapChain, nullptr);
}


void Renderer::shutDown()
{	
	releaseRenderPart();
	shutdownDepthResources();
	shutdownSSRDepthResources();

	AssetDatabase::GetInstance()->cleanUp();

	for (size_t i = 0; i < postProcessChain.size(); i++)
	{
		postProcessChain[i]->shutDown();
	}

	postProcessChain.clear();

	deleteSemaphores();		

	vkDestroyCommandPool(vulkanApp->getDevice(), gbufferCmdPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), frustumCullingPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), mainCmdPool, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), singleTriangularVertexBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), singleTriangularVertexMemory, nullptr);

	mainCamera.shutDown();

	//except Sun
	for (size_t i = 1; i < directionalLights.size(); i++)
	{
		delete directionalLights[i];
	}

	directionalLights.clear();
	directionalLightInfo.clear();

	for (size_t i = 0; i < pointLights.size(); i++)
	{
		delete pointLights[i];
	}

	std::vector<LightInfo> directionalLightInfo;
	pointLightInfo.clear();

	//createDepthResources(); // this should be fixed
	//depthTexture->shutDown();

	vkDestroyBuffer(vulkanApp->getDevice(), directionalLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), directionalLightUniformMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), pointLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), pointLightUniformMemory, nullptr);

	//delete[] SSROforReset;
	//delete[] SSRDforReset;

	vkDestroyBuffer(vulkanApp->getDevice(), SSRDepthBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), SSRDepthBufferMemory, nullptr);

	//vkDestroyBuffer(vulkanApp->getDevice(), SSROffsetBuffer, nullptr);
	//vkFreeMemory(vulkanApp->getDevice(), SSROffsetBufferMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), perFrameBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), perFrameBufferMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), SSRInfoBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), SSRInfoBufferMem, nullptr);
	
	skySystem.shutDown();

	DELETE_SAFE(vulkanApp);
	

	interface.shutDown();
}

void Renderer::createSwapChainImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++)
	{
		vulkanApp->createImageView(swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount, swapChainImageViews[i]);
	}
}

/*
void Renderer::createGUICanvas()
{
	guiCanvas.resize(1);

	for (uint32_t i = 0; i < guiCanvas.size(); i++)
	{
		guiCanvas[i] = new Texture;
		guiCanvas[i]->connectDevice(vulkanApp);
		vulkanApp->createImage(VK_IMAGE_TYPE_2D, swapChainExtent.width, swapChainExtent.height, 1, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			guiCanvas[i]->textureImage, guiCanvas[i]->textureImageMemory);

		vulkanApp->createImageView(guiCanvas[i]->textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount, guiCanvas[i]->textureImageView);

		vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
			VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, guiCanvas[i]->textureSampler);
	}
}

void Renderer::createGUIFrameBuffers( )
{
	std::vector<VkImageView> guiImageViews;
	guiImageViews.resize(guiCanvas.size());

	for (size_t i = 0; i < guiImageViews.size(); i++)
	{
		guiImageViews[i] = guiCanvas[i]->textureImageView;
	}

	vulkanApp->createFramebuffers(guiImageViews, NULL, guiFramebuffers, gui.init_data.render_pass, swapChainExtent.width, swapChainExtent.height, layerCount, 1);
}
*/

void Renderer::createGbuffers()
{
	gbuffers.resize(NUM_GBUFFERS);

	for (uint32_t i = 0; i < gbuffers.size(); i++)
	{

		gbuffers[i] = new Texture;
		gbuffers[i]->connectDevice(vulkanApp);
		vulkanApp->createImage(VK_IMAGE_TYPE_2D, swapChainExtent.width, swapChainExtent.height, 1, 1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			gbuffers[i]->textureImage, gbuffers[i]->textureImageMemory);

		vulkanApp->createImageView(gbuffers[i]->textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount, gbuffers[i]->textureImageView);

		vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
			VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, gbuffers[i]->textureSampler);
	}
}

void Renderer::updateGbuffers()
{
	for (uint32_t i = 0; i < gbuffers.size(); i++)
	{
		vulkanApp->createImage(VK_IMAGE_TYPE_2D, swapChainExtent.width, swapChainExtent.height, 1, 1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			gbuffers[i]->textureImage, gbuffers[i]->textureImageMemory);

		vulkanApp->createImageView(gbuffers[i]->textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount, gbuffers[i]->textureImageView);

		vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
			VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, gbuffers[i]->textureSampler);
	}
}

/*
void Renderer::releaseGUICanvas()
{
	for (size_t i = 0; i < guiCanvas.size(); i++)
	{
		guiCanvas[i]->shutDown();
	}
}

void Renderer::deleteGUICanvas()
{
	for (size_t i = 0; i < guiCanvas.size(); i++)
	{
		guiCanvas[i]->shutDown();
	}

	guiCanvas.clear();
}
*/

void Renderer::releaseGbuffers()
{
	for (size_t i = 0; i < gbuffers.size(); i++)
	{
		gbuffers[i]->shutDown();
	}
}

void Renderer::deleteGbuffers()
{
	for (size_t i = 0; i < gbuffers.size(); i++)
	{
		gbuffers[i]->shutDown();
	}

	gbuffers.clear();
}

void  Renderer::createSingleTriangularVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * 3;

	vulkanApp->createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, singleTriangularVertexBuffer, singleTriangularVertexMemory);

	//Vulkan's Y coord is opposite

	Vertex Vertices[3];
	Vertices[0].positions = glm::vec4(-1.0, -3.0, 0.5, 1.0);
	Vertices[0].texcoords = glm::vec2(0.0, -1.0);

	Vertices[1].positions = glm::vec4(-1.0, 1.0, 0.5, 1.0);
	Vertices[1].texcoords = glm::vec2(0.0, 1.0);

	Vertices[2].positions = glm::vec4(3.0, 1.0, 0.5, 1.0);
	Vertices[2].texcoords = glm::vec2(2.0, 1.0);

	void* data;
	vkMapMemory(vulkanApp->getDevice(), singleTriangularVertexMemory, 0, bufferSize, 0, &data);
	memcpy(data, Vertices, (size_t)bufferSize);
	vkUnmapMemory(vulkanApp->getDevice(), singleTriangularVertexMemory);
}


void Renderer::createGbufferCommandPool()
{
	vulkanApp->createCommandPool(gbufferCmdPool);
}

void Renderer::createGbufferCommandBuffers()
{
	vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, gbufferFramebuffers, gbufferCmd, gbufferCmdPool);
}

void Renderer::recordGbufferCommandBuffers()
{
	std::vector<VkClearValue> clearValues;
	clearValues.resize(NUM_GBUFFERS + 1);
	clearValues[BASIC_COLOR].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[SPECULAR_COLOR].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[NORMAL_COLOR].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[EMISSIVE_COLOR].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[NUM_GBUFFERS].depthStencil = { 1.0f, 0 };

	vulkanApp->recordCommandBuffers(&gbufferCmd, gbufferCmdPool, &gbufferFramebuffers, "", gbufferRenderPass, swapChainExtent, &clearValues, 1, NULL, 0, 0, 0, 0, 0);
}

void Renderer::createFrustumCullingCommandPool()
{
	vulkanApp->createCommandPool(frustumCullingPool);
}

void Renderer::createFrustumCullingCommandBuffers()
{
	std::vector<VkFramebuffer> dummyFrameBuffer;
	dummyFrameBuffer.resize(1);

	vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, dummyFrameBuffer, frustumCmd, frustumCullingPool);
}

void Renderer::recordFrustumCullingCommandBuffers(int groupSizeX, int groupSizeY, int groupSizeZ)
{
	vulkanApp->recordCommandBuffers(&frustumCmd, frustumCullingPool, NULL, "frustumCulling", NULL, swapChainExtent, NULL, 1, NULL, 0, 0, 500, 1, 1);
}

void Renderer::createGbufferRenderPass()
{
	std::vector<VkAttachmentReference> attachmentRefs = {};
	attachmentRefs.resize(NUM_GBUFFERS);
	attachmentRefs[BASIC_COLOR].attachment = BASIC_COLOR;
	attachmentRefs[BASIC_COLOR].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentRefs[SPECULAR_COLOR].attachment = SPECULAR_COLOR;
	attachmentRefs[SPECULAR_COLOR].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentRefs[NORMAL_COLOR].attachment = NORMAL_COLOR;
	attachmentRefs[NORMAL_COLOR].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentRefs[EMISSIVE_COLOR].attachment = EMISSIVE_COLOR;
	attachmentRefs[EMISSIVE_COLOR].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = NUM_GBUFFERS;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkSubpassDescription> subpasses = {};
	subpasses.resize(1);
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = static_cast<uint32_t>(attachmentRefs.size());
	subpasses[0].pColorAttachments = attachmentRefs.data();
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	std::vector<VkAttachmentDescription> attachments = {};
	attachments.resize(NUM_GBUFFERS + 1);
	attachments[BASIC_COLOR].format = VK_FORMAT_R8G8B8A8_UNORM;
	attachments[BASIC_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[BASIC_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[BASIC_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[BASIC_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[BASIC_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[BASIC_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[BASIC_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[SPECULAR_COLOR].format = VK_FORMAT_R8G8B8A8_UNORM;
	attachments[SPECULAR_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[SPECULAR_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[SPECULAR_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[SPECULAR_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[SPECULAR_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[SPECULAR_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[SPECULAR_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[NORMAL_COLOR].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attachments[NORMAL_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[NORMAL_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[NORMAL_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[NORMAL_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[NORMAL_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[NORMAL_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[NORMAL_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[EMISSIVE_COLOR].format = VK_FORMAT_R8G8B8A8_UNORM;
	attachments[EMISSIVE_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[EMISSIVE_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[EMISSIVE_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[EMISSIVE_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[EMISSIVE_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[EMISSIVE_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[EMISSIVE_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[NUM_GBUFFERS].format = vulkanApp->findDepthFormat();
	attachments[NUM_GBUFFERS].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[NUM_GBUFFERS].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[NUM_GBUFFERS].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // We will read from depth, so it's important to store the depth attachment results
	attachments[NUM_GBUFFERS].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[NUM_GBUFFERS].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[NUM_GBUFFERS].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[NUM_GBUFFERS].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // Attachment will be transitioned to shader read at render pass end

	std::vector<VkSubpassDependency> dependencies = {};
	dependencies.resize(2);
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	vulkanApp->createRenderPass(swapChainImageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, attachments, subpasses, dependencies, gbufferRenderPass);
}


void Renderer::createMainRenderPass()
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
	attachments[0].format = swapChainImageFormat;
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

	vulkanApp->createRenderPass(swapChainImageFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, attachments, subpasses, dependencies, mainRenderPass);
}


void Renderer::createDepthResources()
{
	VkFormat depthFormat = vulkanApp->findDepthFormat();	

	vulkanApp->createImage(VK_IMAGE_TYPE_2D, swapChainExtent.width, swapChainExtent.height, 1, 1, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthTexture->textureImage, depthTexture->textureImageMemory);
	vulkanApp->createImageView(depthTexture->textureImage, VK_IMAGE_VIEW_TYPE_2D, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1, depthTexture->textureImageView);
	
	vulkanApp->transitionImageLayout(depthTexture->textureImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, gbufferCmdPool, vulkanApp->getTransferQueue());

	vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, depthTexture->textureSampler);

	/*
	depthMipmapTexture->mipLevel = DEPTH_MIP_SIZE + 1;

	vulkanApp->createImage(VK_IMAGE_TYPE_2D, swapChainExtent.width, swapChainExtent.height, 1, depthMipmapTexture->mipLevel, 1, VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthMipmapTexture->textureImage, depthMipmapTexture->textureImageMemory);

	vulkanApp->createImageView(depthMipmapTexture->textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, depthMipmapTexture->textureImageView);

	vulkanApp->transitionImageLayout(depthMipmapTexture->textureImage, VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, gbufferCmdPool, vulkanApp->getTransferQueue());

	vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, depthMipmapTexture->textureSampler);

	for (size_t i = 0; i < depthMipSizeBuffer.size(); i++)
	{
		vulkanApp->createBuffer(sizeof(glm::vec4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			depthMipSizeBuffer[i], depthMipSizeBufferMemory[i]);
	}
	*/
}

void Renderer::releaseDepthResources()
{
	//depth
	vkDestroySampler(vulkanApp->getDevice(), depthTexture->textureSampler, nullptr);
	vkDestroyImage(vulkanApp->getDevice(), depthTexture->textureImage, nullptr);
	vkDestroyImageView(vulkanApp->getDevice(), depthTexture->textureImageView, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), depthTexture->textureImageMemory, nullptr);

	/*
	vkDestroySampler(vulkanApp->getDevice(), depthMipmapTexture->textureSampler, nullptr);
	vkDestroyImage(vulkanApp->getDevice(), depthMipmapTexture->textureImage, nullptr);
	vkDestroyImageView(vulkanApp->getDevice(), depthMipmapTexture->textureImageView, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), depthMipmapTexture->textureImageMemory, nullptr);
	
	
	for (size_t i = 0; i < depthMipSizeBuffer.size(); i++)
	{
		vkFreeMemory(vulkanApp->getDevice(), depthMipSizeBufferMemory[i], nullptr);

		depthMipSizeBufferMemory[i] = NULL;

		vkDestroyBuffer(vulkanApp->getDevice(), depthMipSizeBuffer[i], nullptr);		

		depthMipSizeBuffer[i] = NULL;
	}
	*/
}

void Renderer::shutdownDepthResources()
{
	depthTexture->shutDown();

	//depthMipmapTexture->shutDown();

	/*
	for (size_t i = 0; i < depthMipTexture.size(); i++)
	{
		depthMipTexture[i]->shutDown();		
	}

	for (size_t i = 0; i < depthMipSizeBuffer.size(); i++)
	{
		vkDestroyBuffer(vulkanApp->getDevice(), depthMipSizeBuffer[i], nullptr);
		vkFreeMemory(vulkanApp->getDevice(), depthMipSizeBufferMemory[i], nullptr);
	}
	*/
	
}

void Renderer::createSSRDepthResources()
{
	vulkanApp->createImage(VK_IMAGE_TYPE_2D, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT, 1, 1, 1, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_STORAGE_BIT , VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, SSRDepthTexture->textureImage, SSRDepthTexture->textureImageMemory);
	vulkanApp->createImageView(SSRDepthTexture->textureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, SSRDepthTexture->textureImageView);

	vulkanApp->transitionImageLayout(SSRDepthTexture->textureImage, VK_FORMAT_R32_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, vulkanApp->getTransferCmdPool(), vulkanApp->getTransferQueue());

	vulkanApp->createTextureSampler(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.0f, 0.0f, 0.0f, SSRDepthTexture->textureSampler);
}

void Renderer::releaseSSRDepthResources()
{
	vkDestroySampler(vulkanApp->getDevice(), SSRDepthTexture->textureSampler, nullptr);
	vkDestroyImage(vulkanApp->getDevice(), SSRDepthTexture->textureImage, nullptr);
	vkDestroyImageView(vulkanApp->getDevice(), SSRDepthTexture->textureImageView, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), SSRDepthTexture->textureImageMemory, nullptr);
}

void Renderer::shutdownSSRDepthResources()
{
	SSRDepthTexture->shutDown();
}

void Renderer::createGbufferFramebuffers()
{
	std::vector<VkImageView> gbufferImageViews;
	gbufferImageViews.resize(gbuffers.size());

	for(size_t i = 0; i < gbufferImageViews.size(); i++)
	{
		gbufferImageViews[i] = gbuffers[i]->textureImageView;
	}

	vulkanApp->createFramebuffers(gbufferImageViews, depthTexture->textureImageView, gbufferFramebuffers, gbufferRenderPass, swapChainExtent.width, swapChainExtent.height, layerCount, 4);
}

void Renderer::createMainFramebuffers()
{
	vulkanApp->createFramebuffers(swapChainImageViews, NULL, swapChainFramebuffers, mainRenderPass, swapChainExtent.width, swapChainExtent.height, layerCount, 1);
}

void Renderer::createMainCommandPool()
{
	vulkanApp->createCommandPool(mainCmdPool);
}

void Renderer::createMainCommandBuffers()
{
	vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, swapChainFramebuffers, mainCmd, mainCmdPool);
}

void Renderer::recordMainCommandBuffers()
{
	std::vector<VkClearValue> clearValues;
	clearValues.resize(2);
	clearValues[0].color = { 0.0f, 0.6f, 0.8f, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	vulkanApp->recordCommandBuffers(&mainCmd, mainCmdPool, &swapChainFramebuffers, "present_mat", mainRenderPass, swapChainExtent, &clearValues, 0, singleTriangularVertexBuffer, 0, 3, 0, 0, 0);
}