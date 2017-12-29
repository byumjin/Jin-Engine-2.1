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

void Renderer::assignRenderpassID(Material* pMat, VkRenderPass renderPass)
{
	if (renderPass == gbufferRenderPass)
	{
		pMat->renderPassID = RenderPassID::GBUFFER;
	}
	else if (renderPass == mainRenderPass)
	{
		pMat->renderPassID = RenderPassID::MAIN;
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
		return NULL;
	}
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
	
	createSemaphores();
	createQueues();

	createSwapChain();
	createSwapChainImageViews();	

	createSingleTriangularVertexBuffer();

	createGbufferCommandPool();
	createFrustumCullingCommandPool();

	createMainCommandPool();	

	createGbuffers();

	depthTexture = new Texture;
	depthTexture->vulkanApp = vulkanApp;

	createDepthResources();

	createGbufferRenderPass();
	createMainRenderPass();

	//initialize Resurces
	AssetDatabase::GetInstance()->setVulkanApp(vulkanApp);

	mainCamera.setCamera(vulkanApp, mainCamera.position, mainCamera.focusPosition, mainCamera.upVector,  45.0f,
		static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), NEAR_PLANE, FAR_PLANE);

	//FrustumCullingMaterial
	{
		FrustumCullingMaterial* frustumCulling_Mat = new FrustumCullingMaterial;

		//frustumCulling_Mat->createLocalBuffer();

		frustumCulling_Mat->createPipeline("frustumCulling", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), glm::vec2(0.0), glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0),
			NULL, NULL, NULL);

		pfrustumCullingMaterial = frustumCulling_Mat;// dynamic_cast<FrustumCullingMaterial*>(DBInstance->FindAsset<Material>("frustumCulling"));
	}


	//ObjectMaterial
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
			temp_Gbuffer_Mat->createPipeline("arch", "Asset/Texture/sponza/arch/arch_albedo.tga", "Asset/Texture/sponza/arch/arch_spec.tga", "Asset/Texture/sponza/arch/arch_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer, NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//bricks
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("bricks", "Asset/Texture/sponza/bricks/bricks_albedo.tga", "Asset/Texture/sponza/bricks/bricks_spec.tga", "Asset/Texture/sponza/bricks/bricks_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//ceiling
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("ceiling", "Asset/Texture/sponza/ceiling/ceiling_albedo.tga", "Asset/Texture/sponza/ceiling/ceiling_spec.tga", "Asset/Texture/sponza/ceiling/ceiling_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//chain
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("chain", "Asset/Texture/sponza/chain/chain_albedo.tga", "Asset/Texture/sponza/chain/chain_spec.tga", "Asset/Texture/sponza/chain/chain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_a
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_a", "Asset/Texture/sponza/column/column_a_albedo.tga", "Asset/Texture/sponza/column/column_a_spec.tga", "Asset/Texture/sponza/column/column_a_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_b
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_b", "Asset/Texture/sponza/column/column_b_albedo.tga", "Asset/Texture/sponza/column/column_b_spec.tga", "Asset/Texture/sponza/column/column_b_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//column_c
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("column_c", "Asset/Texture/sponza/column/column_c_albedo.tga", "Asset/Texture/sponza/column/column_c_spec.tga", "Asset/Texture/sponza/column/column_c_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_blue
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_blue", "Asset/Texture/sponza/curtain/sponza_curtain_blue_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_blue_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_green
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_green", "Asset/Texture/sponza/curtain/sponza_curtain_green_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_green_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//curtain_red
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("curtain_red", "Asset/Texture/sponza/curtain/sponza_curtain_red_albedo.tga", "Asset/Texture/sponza/curtain/sponza_curtain_red_spec.tga", "Asset/Texture/sponza/curtain/sponza_curtain_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//detail
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("detail", "Asset/Texture/sponza/detail/detail_albedo.tga", "Asset/Texture/sponza/detail/detail_spec.tga", "Asset/Texture/sponza/detail/detail_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_blue
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_blue", "Asset/Texture/sponza/fabric/fabric_blue_albedo.tga", "Asset/Texture/sponza/fabric/fabric_blue_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_green
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_green", "Asset/Texture/sponza/fabric/fabric_green_albedo.tga", "Asset/Texture/sponza/fabric/fabric_green_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//fabric_red
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("fabric_red", "Asset/Texture/sponza/fabric/fabric_red_albedo.tga", "Asset/Texture/sponza/fabric/fabric_red_spec.tga", "Asset/Texture/sponza/fabric/fabric_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//flagpole
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("flagpole", "Asset/Texture/sponza/flagpole/flagpole_albedo.tga", "Asset/Texture/sponza/flagpole/flagpole_spec.tga", "Asset/Texture/sponza/flagpole/flagpole_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//floor
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("floor", "Asset/Texture/sponza/floor/floor_albedo.tga", "Asset/Texture/sponza/floor/floor_spec.tga", "Asset/Texture/sponza/floor/floor_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//lion
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("lion", "Asset/Texture/sponza/lion/lion_albedo.tga", "Asset/Texture/sponza/lion/lion_spec.tga", "Asset/Texture/sponza/lion/lion_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//lion_back
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("lion_back", "Asset/Texture/sponza/lion_background/lion_background_albedo.tga", "Asset/Texture/sponza/lion_background/lion_background_spec.tga", "Asset/Texture/sponza/lion_background/lion_background_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//plant
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("plant", "Asset/Texture/sponza/plant/vase_plant_albedo.tga", "Asset/Texture/sponza/plant/vase_plant_spec.tga", "Asset/Texture/sponza/plant/vase_plant_norm.tga", "Asset/Texture/sponza/plant/vase_plant_emiss.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//roof
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("roof", "Asset/Texture/sponza/roof/roof_albedo.tga", "Asset/Texture/sponza/roof/roof_spec.tga", "Asset/Texture/sponza/roof/roof_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//thorn
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("thorn", "Asset/Texture/sponza/thorn/sponza_thorn_albedo.tga", "Asset/Texture/sponza/thorn/sponza_thorn_spec.tga", "Asset/Texture/sponza/thorn/sponza_thorn_norm.tga", "Asset/Texture/sponza/thorn/sponza_thorn_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase", "Asset/Texture/sponza/vase/vase_albedo.tga", "Asset/Texture/sponza/vase/vase_spec.tga", "Asset/Texture/sponza/vase/vase_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase_hanging
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase_hanging", "Asset/Texture/sponza/vase_hanging/vase_hanging_albedo.tga", "Asset/Texture/sponza/vase_hanging/vase_round_spec.tga", "Asset/Texture/sponza/vase_hanging/vase_round_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			//vase_round
			temp_Gbuffer_Mat = new GbufferMaterial;
			temp_Gbuffer_Mat->createPipeline("vase_round", "Asset/Texture/sponza/vase_hanging/vase_round_albedo.tga", "Asset/Texture/sponza/vase_hanging/vase_round_spec.tga", "Asset/Texture/sponza/vase_hanging/vase_round_norm.tga", "Asset/Texture/sponza/no_emis.tga", &sponza->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
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
				&cerberus->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			cerberus->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("Cerberus"));

			AssetDatabase::GetInstance()->objectManager.push_back(cerberus);
		}

		//Chromie
		{
			Object *Chromie = new Object;
			Chromie->initialize(vulkanApp, "Chromie", "Asset/Object/Chromie.obj", true);	
			Chromie->scale = glm::vec3(0.1f);
			Chromie->position = glm::vec3(0.0f, 0.0f, 0.0f);
			Chromie->updateOrbit(0.0f, 85.0f, 0.0);
			Chromie->updateObjectBuffer();

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
				&Chromie->uniformObjectBuffer, &mainCamera.uniformCameraBuffer,NULL, pointLightInfo.size(), NULL, directionalLightInfo.size(), screenOffsets, sizeScale, gbufferRenderPass, NULL, NULL);
			assignRenderpassID(temp_Gbuffer_Mat, gbufferRenderPass);

			Chromie->materials.push_back(AssetDatabase::GetInstance()->FindAsset<Material>("Chromie"));

			AssetDatabase::GetInstance()->objectManager.push_back(Chromie);
		}	
	}

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
	skySystem.sun.updateOrbit(90.0f, 0.0f, 0.0);
	skySystem.sun.lightInfo.color = glm::vec4(1.0, 1.0, 1.0, 1.0);
	skySystem.sun.lightInfo.direction = skySystem.sun.getViewVector4();

	directionalLights.push_back(&skySystem.sun);
	directionalLightInfo.push_back(skySystem.sun.lightInfo);

	createDirectionalLightBuffer();

	//PBR material
	{
		UberMaterial* temp_uber_Mat = new UberMaterial;
		temp_uber_Mat->createPipeline("uber_mat", "", "", "", "", NULL, &mainCamera.uniformCameraBuffer,
			&pointLightUniformBuffer, pointLightInfo.size(),&directionalLightUniformBuffer, directionalLightInfo.size(),
			glm::vec2(0.0), glm::vec4(swapChainExtent.width, swapChainExtent.height, 1.0, 1.0), mainRenderPass, &gbuffers, depthTexture);
		assignRenderpassID(temp_uber_Mat, mainRenderPass);
	}

	createGbufferFramebuffers();
	createMainFramebuffers();
	

	createGbufferCommandBuffers();
	createFrustumCullingCommandBuffers();
	createMainCommandBuffers();

	//record static CommandBuffers;
	recordMainCommandBuffers();

}

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

void Renderer::reInitializeRenderer()
{
	releaseRenderPart();
	shutdownDepthResources();

	AssetDatabase::GetInstance()->cleanUp();

	deleteSemaphores();

	vkDestroyCommandPool(vulkanApp->getDevice(), gbufferCmdPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), frustumCullingPool, nullptr);
	vkDestroyCommandPool(vulkanApp->getDevice(), mainCmdPool, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), singleTriangularVertexBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), singleTriangularVertexMemory, nullptr);

	mainCamera.shutDown();


	vkDestroyBuffer(vulkanApp->getDevice(), directionalLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), directionalLightUniformMemory, nullptr);

	vkDestroyBuffer(vulkanApp->getDevice(), pointLightUniformBuffer, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), pointLightUniformMemory, nullptr);


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

	mainCamera.updateOrbit(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(interface.getWindow()))
	{
		glfwPollEvents();

		if (interface.windowResetFlag)
		{
			reInitializeRenderer();
		}

		unsigned int realTime = timer.getTime();
		deltaTime = realTime - previousTime;


		interface.fpstracker++;

		double fpsElapsedTime = (realTime - fpsPreviosTime) * 0.001;

		if (fpsElapsedTime >= 1.0)
		{

			interface.fps = static_cast<int>(interface.fpstracker / fpsElapsedTime);
			interface.fpstracker = 0;
			fpsPreviosTime = realTime;

			std::string title = "VR Project | " + std::to_string(interface.fps) + " fps | " + std::to_string(1000.0 / (double)interface.fps) + " ms";
			interface.setWindowTitle(title);
		}


		double sensitivity = 20.0;
		double deltaTimeSec = deltaTime * 0.001;

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
		
		culling();

		//update SkySystem
		skySystem.sun.updateOrbit(0.0f, deltaTime * 0.05f, 0.0f);
		skySystem.sun.lightInfo.direction = skySystem.sun.getViewVector4();
		updateDirectionalLightBuffer();
		
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

	//frameQueue
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &pbrSemaphore;
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

	createSwapChain();
	createSwapChainImageViews();	

	//createGbufferCommandPool();
	//createMainCommandPool();

	updateGbuffers();
	createDepthResources();

	createGbufferRenderPass();
	createMainRenderPass();

	//resources
	mainCamera.updateAspectRatio(static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height));

	//materials
	for (uint32_t i = 0; i < AssetDatabase::GetInstance()->materialList.size(); i++)
	{
		Material* pMaterial = AssetDatabase::GetInstance()->FindAsset<Material>(AssetDatabase::GetInstance()->materialList[i]);
		pMaterial->updatePipeline(pMaterial->screenOffset, glm::vec4(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), pMaterial->sizeScale.z, pMaterial->sizeScale.w), getAssignedRenderpassID(pMaterial));
	}


	createGbufferFramebuffers();
	createMainFramebuffers();


	createGbufferCommandBuffers();
	createFrustumCullingCommandBuffers();
	createMainCommandBuffers();

	//record static CommandBuffers;
	recordMainCommandBuffers();
}

void Renderer::releaseDepthResources()
{
	//depth
	vkDestroySampler(vulkanApp->getDevice(), depthTexture->textureSampler, nullptr);
	vkDestroyImage(vulkanApp->getDevice(), depthTexture->textureImage, nullptr);
	vkDestroyImageView(vulkanApp->getDevice(), depthTexture->textureImageView, nullptr);
	vkFreeMemory(vulkanApp->getDevice(), depthTexture->textureImageMemory, nullptr);
}

void Renderer::shutdownDepthResources()
{
	depthTexture->shutDown();
}

void Renderer::releaseRenderPart()
{
	//releaseDepthResources();

	AssetDatabase::GetInstance()->releaseRenderingPart();

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
	//deleteGbuffers();


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

	AssetDatabase::GetInstance()->cleanUp();

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
	vulkanApp->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, gbufferFramebuffers, frustumCmd, frustumCullingPool);
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
	attachments[BASIC_COLOR].format = VK_FORMAT_R16G16B16A16_SFLOAT;
	attachments[BASIC_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[BASIC_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[BASIC_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[BASIC_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[BASIC_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[BASIC_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[BASIC_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[SPECULAR_COLOR].format = VK_FORMAT_R16G16B16A16_SFLOAT;
	attachments[SPECULAR_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[SPECULAR_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[SPECULAR_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[SPECULAR_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[SPECULAR_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[SPECULAR_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[SPECULAR_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[NORMAL_COLOR].format = VK_FORMAT_R16G16B16A16_SFLOAT;
	attachments[NORMAL_COLOR].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[NORMAL_COLOR].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[NORMAL_COLOR].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[NORMAL_COLOR].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[NORMAL_COLOR].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[NORMAL_COLOR].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[NORMAL_COLOR].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[EMISSIVE_COLOR].format = VK_FORMAT_R16G16B16A16_SFLOAT;
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

	vulkanApp->createTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FALSE, 1, VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 0.0f, 0.0f, depthTexture->textureSampler);
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

	vulkanApp->recordCommandBuffers(&mainCmd, mainCmdPool, &swapChainFramebuffers, "uber_mat", mainRenderPass, swapChainExtent, &clearValues, 0, singleTriangularVertexBuffer, 0, 3, 0, 0, 0);
}