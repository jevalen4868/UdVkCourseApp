#include "VulkanRenderer.h"

// refer to https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers for higher levels of configuration.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) {
	printf("validation layer=%s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

VulkanRenderer::VulkanRenderer() {
}

VulkanRenderer::~VulkanRenderer() {
}

int VulkanRenderer::init(GLFWwindow *newWindow) {
	_window = newWindow;
	try {
		createInstance();
		createDebugMessengerExtension();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createRenderPass();
		createGraphicsPipeline();
		createCommandPool();
	}
	catch (const std::runtime_error &e) {
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::destroy() {
	
	vkDestroyCommandPool(_mainDevice.logicalDevice, _graphicsCommandPool, nullptr);
	for (const auto &framebuffer : _swapchainFramebuffers) {
		vkDestroyFramebuffer(_mainDevice.logicalDevice, framebuffer, nullptr);
	}
	vkDestroyPipeline(_mainDevice.logicalDevice, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_mainDevice.logicalDevice, _pipelineLayout, nullptr);
	vkDestroyRenderPass(_mainDevice.logicalDevice, _renderPass, nullptr);
	for (auto &image : _swapchainImages) {
		vkDestroyImageView(_mainDevice.logicalDevice, image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(_mainDevice.logicalDevice, _swapchain, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyDevice(_mainDevice.logicalDevice, nullptr);
	// setup validation layer for destruction.
	if (_enableValidationLayers) {
		auto destroyMessengerFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyMessengerFunc != nullptr) {
			destroyMessengerFunc(_instance, _debugMessenger, nullptr);
		}
	}
	vkDestroyInstance(_instance, nullptr);
}

void VulkanRenderer::createInstance() {
	// check if we should enable validation layers and if they are supported.
	if (_enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Info about the app itself.
	// Most data here doesn't affect the program and is for dev convenience.
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App"; // custom name of the app.
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Custom version of the app.
	appInfo.pEngineName = "No Engine"; // Custom engine name.
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0); // Custom engine version.
	appInfo.apiVersion = VK_API_VERSION_1_2;

	// creation info for a vk instance.
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Create list to hold the instance extensions.
	vector<const char *> instanceExtensions{ getRequiredExtensions() };

	// Check instance extensions supported.
	if (!checkInstanceExtensionSupport(&instanceExtensions)) {
		throw std::runtime_error("VkInstance does not support required extensions.");
	}

	// pp* means pointer to pointer, array of pointers, etc.
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// Set up the validation layers the instance will use.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
		createInfo.ppEnabledLayerNames = _validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Create instance.
	VkResult result{ vkCreateInstance(&createInfo, nullptr, &_instance) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a vkInstance.");
	}
}

void VulkanRenderer::createLogicalDevice() {
	// Get the queue family indices for the chosen Physical Device.
	QueueFamilyIndices indices{ getQueueFamilies(_mainDevice.physicalDevice) };

	// Vector for queue creation information. Set for family indices.
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<int> queueFamilyIndices{ indices.graphicsFamily, indices.presentationFamily };

	// Queues the logical device needs to create and info to do so.
	for (int queueFamilyIndex : queueFamilyIndices) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex; // index of the family to create a queue from.
		queueCreateInfo.queueCount = 1; // Number of queues to create.
		float priority{ 1.0f };
		queueCreateInfo.pQueuePriorities = &priority; // Vulkan needs priority to handle priority for multiple queues.
		// add to vec
		queueCreateInfos.push_back(queueCreateInfo);
	}
	// Info to create logica device (sometimes called "device").
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()); // number of queue create infos.
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data(); // List of queue create info so the device can create required queues.
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()); // number of enabled logical device extensions.
	deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(); // List of enabled logical device extensions.

	// Physical device features the logical device will be using.
	VkPhysicalDeviceFeatures deviceFeatures{};
	// deviceFeatures.depthClamp = VK_TRUE; // If we want to enable depth clamping later.
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Create the logical device for the given physical device.
	VkResult result{ vkCreateDevice(_mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &_mainDevice.logicalDevice) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a vk device.");
	}

	// Queues are created at the same time as the device. Get handle to queues.
	// From given logical device, of given queue family, of given queue index (0 since only one queue), place reference in given VkQueue.
	vkGetDeviceQueue(_mainDevice.logicalDevice, indices.graphicsFamily, 0, &_graphicsQueue);
	vkGetDeviceQueue(_mainDevice.logicalDevice, indices.presentationFamily, 0, &_presentationQueue);
}

void VulkanRenderer::createSurface() {
	// Create Surface, creates a surface create info struct, runs the create surface function
	VkResult result{ glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a surface.");
	}
}

void VulkanRenderer::createSwapChain() {
	// Get swap chain details so we can pick best settings.
	SwapchainDetails swapChainDetails{ getSwapChainDetails(_mainDevice.physicalDevice) }; 
	
	// Find optimal surface values for our swapchain.
	// Choose best surface format.
	VkSurfaceFormatKHR surfaceFormat{ chooseBestSurfaceFormat(swapChainDetails.formats) };
	// Choose best presentation mode.
	VkPresentModeKHR presentMode{ chooseBestPresMode(swapChainDetails.presentationModes) };
	// Choose swap chain image resolution.
	VkExtent2D extent{ chooseSwapExtent(swapChainDetails.surfaceCapabilities) };

	// How many images are in the swap chain? Get 1 more than the minimum to allow triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;
	// If 0, then it's limitless, otherwise, if imageCount higher than max, clamp down to max.
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount) {
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}
	// Creation info for swap chain.
	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.imageFormat = surfaceFormat.format; // swapChain format.
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace; // swapChain color space.
	swapChainCreateInfo.presentMode = presentMode; // Swapchain pres mode.
	swapChainCreateInfo.imageExtent = extent; // Swapchain image extent.
	swapChainCreateInfo.minImageCount = imageCount; // Min images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1; // Number of layers each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // What attachment images will be used as.
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform; // Transform to perform on swap chain images.
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // How to handle blending images with external graphics, ie. other windows.
	swapChainCreateInfo.clipped = VK_TRUE; // Whether to clip parts of the image not in view, e.g. covered by other window, offscreen, etc.
	
	// get queue family indices
	QueueFamilyIndices indices{ getQueueFamilies(_mainDevice.physicalDevice) };
	// if graphics and pres families are different, then swapchain must let images be shared between families.
	if (indices.graphicsFamily != indices.presentationFamily) {
		uint32_t queueFamilyIndices []{
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily,
		};
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2; // Number of queues to share images between.
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // Array of queues to share between.
	} else {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 0; // Number of queues to share images between.
		swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Array of queues to share between.
	}

	swapChainCreateInfo.surface = _surface; // swapchain surface.
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE; //pass values from old swapchain. If old one is destroyed can pass responsibilities.

	// Create SWAP CHAIN!	
	VkResult result{ vkCreateSwapchainKHR(_mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &_swapchain) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a swapchain.");
	}

	// Store swapchain image format and extent.
	_swapchainImageFormat = surfaceFormat.format;
	_swapchainExtent = extent;

	// Get swapchain images. 
	uint32_t swapchainImageCount;
	vkGetSwapchainImagesKHR(_mainDevice.logicalDevice, _swapchain, &swapchainImageCount, nullptr);
	vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(_mainDevice.logicalDevice, _swapchain, &swapchainImageCount, images.data());
	for (const auto &image : images) {
		// Store the image handle.
		SwapchainImage swapchainImage{};
		swapchainImage.image = image;
		// Create image view.
		swapchainImage.imageView = createImageView(image, _swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		_swapchainImages.push_back(swapchainImage);
	}
}

void VulkanRenderer::createGraphicsPipeline() {
	// Read in SPIR-V shader code.
	auto vertexShader = readFile("Shaders/vert.spv");
	auto fragmentShader = readFile("Shaders/frag.spv");
	VkShaderModule vertexShaderModule{ createShaderModule(vertexShader) };
	VkShaderModule fragShaderModule{ createShaderModule(fragmentShader) };

	// SHADER stage creation information
	// - Vertex Stage Creation info.
	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // shader stage name
	vertexShaderStageCreateInfo.module = vertexShaderModule; // shader module to be used.
	vertexShaderStageCreateInfo.pName = "main"; // Entry point into shader

	// - Fragment stage.
	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // shader stage name
	fragmentShaderStageCreateInfo.module = fragShaderModule; // shader module to be used.
	fragmentShaderStageCreateInfo.pName = "main"; // Entry point into shader	

	// Put shader stage creation into array.
	// Graphics pipeline creation info requires array of creates.
	VkPipelineShaderStageCreateInfo shaderStages [] { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };
	
	// - VERTEX INPUT - (TODO) put in vertex descriptions when resources created.
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // List of vertex binding descriptions. (data spacing / stride info)
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // List of vertex attribute descriptions. (data format and where to bind to/from).

	// - INPUT ASSEMBLY -
	VkPipelineInputAssemblyStateCreateInfo pipelineInputStateCreateInfo{};
	pipelineInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // How to assemble the vertices.
	pipelineInputStateCreateInfo.primitiveRestartEnable = VK_FALSE; // Allow overriding of "strip" topology to start new primitives.
	
	// - VIEWPORT & SCISSOR - 
	VkViewport viewport{};
	viewport.x = 0.0f; // X start coord.
	viewport.y = 0.0f; // Y start coord.
	viewport.width = (float)_swapchainExtent.width; // Width of viewport.
	viewport.height = (float)_swapchainExtent.height; // Height of viewport.
	viewport.minDepth = 0.0f; // Min framebuffer depth.
	viewport.maxDepth = 1.0f; // Max framebuffer depth.

	VkRect2D scissor{};
	scissor.offset = { 0, 0 }; // Offset to use region from.
	scissor.extent = _swapchainExtent; // Extent to describe region to use, starting at offset.

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	// - DYNAMIC STATES - 
	// Dynamic states to enable
	vector<VkDynamicState> dynamicStateEnables(2);
	// Need to consider resizing swapchain and depth buffer.
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT); // Dynamic viewport : can resize in command buffer. vkCmdSetViewport(cmdBuf, 0, 1, &vp);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR); // Dynamic scissor : can resize in command buffer. vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t> (dynamicStateEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

	// - RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo{};
	rasterStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterStateCreateInfo.depthClampEnable = VK_FALSE; // Change if fragments beyond near/bar planes are clipped (default) or clamped to plane.
	rasterStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // Enable if you don't want to output to framebuffer and skip raster.
	rasterStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // How to handle filling points between vertices.
	rasterStateCreateInfo.lineWidth = 1.0f; // How thick lines should be when drawn.
	rasterStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // Which face of a triangle to cull.
	rasterStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Winding to determine which side is front.
	rasterStateCreateInfo.depthBiasEnable = VK_FALSE; // Whether to add depth bias to fragments ( good for stopping "shadow acne" in shadow mapping )

	// - MULTISAMPLING -
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE; // Enable multisample sahding or not.
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Number of samles to use per fragment.

	// - BLENDING - 
	// Blending decides how to blend a new color being written to a fragment, with the old value.
	// Blend attachment state (how blending is handled)
	VkPipelineColorBlendAttachmentState colorBlendState{};
	colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT // Which colors to apply blending to. (all colors)
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendState.blendEnable = VK_TRUE; // Enable blending.

	// Blending uses equation: (srcColorBlendFactor * newColor) colorBlendOp (destColorBlendFactor * oldColor)
	colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA; 
	colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
	// Summarized: (VK_BLEND_FACTOR_SRC_APLHA * newColor) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * oldColor)
	//				(newColorAlpha * newColor) + ((1 - newColorAlpha) * oldColor);

	colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarized: (1 * newAlpha) + (0 * oldAlpha) = newAlpha

	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.logicOpEnable = VK_FALSE; // Alternative to calculations is to use logical operations.
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendState;
	
	// - PIPELINE LAYOUT: (TODO, APPLYO FUTURE DESCRIPTOR SET LAYOUTS) -
	VkPipelineLayoutCreateInfo pipelineLayoutCreteInfo{};
	pipelineLayoutCreteInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreteInfo.setLayoutCount = 0;
	pipelineLayoutCreteInfo.pSetLayouts = nullptr;
	pipelineLayoutCreteInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreteInfo.pPushConstantRanges = nullptr;
	
	// Create pipeline layout;
	{
		VkResult result{ vkCreatePipelineLayout(_mainDevice.logicalDevice, &pipelineLayoutCreteInfo, nullptr, &_pipelineLayout) };
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create an image view.");
		}
	}

	// - DEPTH STENCIL TESTING.
	// TODO

	// Create pipeline.
	{
		VkGraphicsPipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.stageCount = 2; // Number of shader stages.
		createInfo.pStages = shaderStages; // List of shader stages.
		createInfo.pVertexInputState = &vertexInputStateCreateInfo; // All the fixed fucntion pipeline states.
		createInfo.pInputAssemblyState = &pipelineInputStateCreateInfo;
		createInfo.pViewportState = &viewportStateCreateInfo;
		createInfo.pDynamicState = nullptr; // &dynamicStateCreateInfo;
		createInfo.pRasterizationState = &rasterStateCreateInfo;
		createInfo.pMultisampleState = &multisampleCreateInfo;
		createInfo.pColorBlendState = &colorBlendStateInfo;
		createInfo.pDepthStencilState = nullptr; // TODO
		createInfo.layout = _pipelineLayout;
		createInfo.renderPass = _renderPass; // Render pass description the pipeline is compatible with.
		createInfo.subpass = 0; // Subpass of render pass to use with pipeline.
		// Pipeline derivatives : can create multiple pipelines that derive from one another for optimization.
		createInfo.basePipelineHandle = VK_NULL_HANDLE; // Existing pipeline to derive from. OR
		createInfo.basePipelineIndex = -1; // or index pipeline being create to derive from.

		VkPipelineCache pipelineCache = VK_NULL_HANDLE; // You can save pipelines to a cache.

		VkResult result{ vkCreateGraphicsPipelines(_mainDevice.logicalDevice, pipelineCache, 1, &createInfo, nullptr, &_graphicsPipeline) };
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a graphics pipeline.");
		}
	}

	vkDestroyShaderModule(_mainDevice.logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(_mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

void VulkanRenderer::createFramebuffers() {
	// Should equal image count.
	_swapchainFramebuffers.resize(_swapchainImages.size());
	for (size_t i{ 0 }; i < _swapchainFramebuffers.size(); i++) {
		array<VkImageView, 1> attachments{
			_swapchainImages[i].imageView
		};
		VkFramebufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = _renderPass; // Render Pass layout the framebuffer will be used with.
		info.attachmentCount = static_cast<uint32_t>(attachments.size());
		info.pAttachments = attachments.data(); // List of attachments 1:1 with render pass.
		info.width = _swapchainExtent.width;
		info.height = _swapchainExtent.height;
		info.layers = 1; // Framebuffer layers.

		VkResult result{ vkCreateFramebuffer(_mainDevice.logicalDevice, &info, nullptr, &_swapchainFramebuffers[i]) };
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a frame buffer.");
		}
	}
}

void VulkanRenderer::createCommandPool() {
	QueueFamilyIndices indices = getQueueFamilies(_mainDevice.physicalDevice);

	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = indices.graphicsFamily; // Queue family type that buffers from this command pool will use.

	// Create a graphics queue family command pool.
	VkResult result{ vkCreateCommandPool(_mainDevice.logicalDevice, &info, nullptr, &_graphicsCommandPool) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a command pool.");
	}
}

void VulkanRenderer::createCommandBuffers() {
	// 1:1, Resize command buffer
	_commandBuffers.resize(_swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = _graphicsCommandPool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // PRIMARY Executed by queue, can't be called by other buffers. 
	// SECONDARY means buffer can't be called directly, called by other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer.
	info.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

	// Allocate command buffers and place handles in array of buffers.
	VkResult result{ vkAllocateCommandBuffers(_mainDevice.logicalDevice, &info, _commandBuffers.data()) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers..");
	}
}

void VulkanRenderer::recordCommands() {
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // buffer can be resubmitted when it is already submitted and waiting execution.

	// Info about how to begin the render pass. Only needed for graphical applications.
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.renderArea.offset = { 0, 0 }; // Start point of render pass in pixels.
	renderPassBeginInfo.renderArea.extent = _swapchainExtent; // Size of region to run render pass on (starting at offset).
	VkClearValue clearValues[] = {
		{0.6f, 0.65f, 0.5f, 1.0f}
	};
	renderPassBeginInfo.pClearValues = clearValues; // TODO add depth attachment clear value.
	renderPassBeginInfo.clearValueCount = 1; 

	for (size_t i{ 0 }; i < _commandBuffers.size(); i++) {
		renderPassBeginInfo.framebuffer = _swapchainFramebuffers[i];

		// Start recording commands.
		if (vkBeginCommandBuffer(_commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS) {
				throw std::runtime_error("Failed to start recording a command buffer.");
		}		

		vkCmdBeginRenderPass(_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind pipeline to be used in render pass.
		vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		// Execute our pipeline.
		vkCmdDraw(_commandBuffers[i], 3, 1, 0, 0);
		// gl_InstanceIndex can be used in the shader for the instance count.

		vkCmdEndRenderPass(_commandBuffers[i]);

		// Stop recording		
		if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to stop recording a command buffer.");
		}	
	}
}

void VulkanRenderer::createRenderPass() {
	// Color attachment of render pass.
	VkAttachmentDescription colorAtt{};
	colorAtt.format = _swapchainImageFormat; // Format to use for attachment. 
	colorAtt.samples = VK_SAMPLE_COUNT_1_BIT; // Number of samples to write for multisampling.
	colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Similar to glClear Op. Describes what to do with attachment before rendering.
	colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Describes what to do with attachment after rendering. Store so then it will be drawn.
	colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Describes what to do with stencil before rendering.
	colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Describes what to do with stencil afterrendering.

	// Framebuffer data will be stored as an image, but images can be given dfferent data layouts.
	// to give optimal use for certain operations.
	colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Image data layout before render pass starts.
	// there will be a subpass layout in between initial and final.
	colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Image data layout after render pass. (to change to)

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo.
	VkAttachmentReference colorAttRef{};
	colorAttRef.attachment = 0; // Use first attachment in list passed to render pass.
	colorAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	// Info about a particular subpass that the render pass is using.
	vector<VkSubpassDescription> subpasses(1);
	{
		VkSubpassDescription subpass {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Pipeline type subpass is to be bound to.
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttRef;
		subpasses.push_back(subpass);
	}

	// Need to determine when layout transitions occur using subpass dependencies.
	array<VkSubpassDependency, 2> subpassDependencies;
	
	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
	// Transition must happen after ...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // Subpass index, subpass external means anything that takes place outside of render pass.
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Pipeline stage: End of subpass external.
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // Stage access mask: Being read when presented to screen.
	// Transition must happen before ...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	// Transition must happen after ...
	subpassDependencies[1].srcSubpass = 0; // Subpass index, subpass external means anything that takes place outside of render pass.
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// Transition must happen before ...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	// Create info for render pass.
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAtt;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = subpasses.data();
	createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	createInfo.pDependencies = subpassDependencies.data();
		
	VkResult result{ vkCreateRenderPass(_mainDevice.logicalDevice, &createInfo, nullptr, &_renderPass) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create the render pass.");
	}
	
}

VkImageView VulkanRenderer::createImageView(const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags) {
	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = image; // Image to create view for.
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // Type of image (1d, 2d, 3d, cube, etc)
	viewCreateInfo.format = format;  // Format of image data.
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Allows remapping of rgba components to other values.
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	// Subresources allow the view to view only a part of an image.
	viewCreateInfo.subresourceRange.aspectMask = aspectFlags; // Which aspect of the image to view. (e.g. COLOR_BIT for viewing color.
	viewCreateInfo.subresourceRange.baseMipLevel = 0; // Start mipmap level to view from.
	viewCreateInfo.subresourceRange.levelCount = 1; // Number of mipmap levels to view.
	viewCreateInfo.subresourceRange.baseArrayLayer = 0; // Start array level to view from.
	viewCreateInfo.subresourceRange.layerCount = 1; // number of array levels to view.

	// Create the image view.
	VkImageView imageView;
	VkResult result{ vkCreateImageView(_mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create an image view.");
	}
	return imageView;
}

VkShaderModule VulkanRenderer::createShaderModule(const vector<char> &code) {
	// Shader module create info.
	VkShaderModuleCreateInfo shaderCreateInfo{};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = code.size(); // size of code.
	// reinterpret what pointer is pointing to.
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data()); // pointer to coe (uint32_t pointer type)
	VkShaderModule shaderModule;
	VkResult result{ vkCreateShaderModule(_mainDevice.logicalDevice, &shaderCreateInfo, nullptr, &shaderModule) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a shader module.");
	}
	return shaderModule;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
	// Create the info as usual.	
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // could pass pointer to any class.
}

void VulkanRenderer::createDebugMessengerExtension() {
	if (!_enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	// Create the debug messenger.
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		VkResult result{ func(_instance, &createInfo, nullptr, &_debugMessenger) };
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create the debug messenger.");
		}
	} else {
		throw std::runtime_error("Failed to create the debug messenger.");
		// return VK_ERROR_EXTENSION_NOT_PRESENT; could use this in a diff impl.
	}
}

void VulkanRenderer::getPhysicalDevice() {
	// enumerate physical devices the vkInstance can access.
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	// Check that devices were found.
	if (deviceCount == 0) {
		throw std::runtime_error("No vulkan compatible GPU devices found.");
	}

	// Get list of physical devices.
	vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

	// Pick the first suitable device.
	for (const auto &device : physicalDevices) {
		if (checkDeviceSuitable(device)) {
			_mainDevice.physicalDevice = device;
		}
	}	
}

vector<const char *> VulkanRenderer::getRequiredExtensions() {
	// Set up extensions the instance will use.
	uint32_t glfwExtensionCount{ 0 }; // glfw may require multiple extensions.
	const char **glfwExtensions; // extensions passed as array of c strings, so we need array of pointers.

	// Get glfw extensions.
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add glfw extensions to list of extensions.
	vector<const char *> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	
	if (_enableValidationLayers) {
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return instanceExtensions;
}

bool VulkanRenderer::checkInstanceExtensionSupport(const vector<const char *> *checkExtensions) {
	// Get the number of extensions to create an array of correct size.
	uint32_t extensionCount{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Create a list of VkExtensionPropertes
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Check if given extensions are in list of available extensions.
	for (const auto &checkExtension : *checkExtensions) {
		bool hasExtension{ false };
		for (const auto &extension : extensions) {
			if (strcmp(checkExtension, extension.extensionName) == 0) {
				hasExtension = true;
				break;
			}
		}
		// The extension is not available :(.
		if (!hasExtension) {
			return false;
		}
	}
	return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(const VkPhysicalDevice &device) {
	// Get device extension count.
	uint32_t extensionCount{ 0 };
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// If no extensions return failure.
	if (extensionCount == 0) return false;

	// Populate extensions.
	vector<VkExtensionProperties> extensions{ extensionCount };
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	// Check for extension.
	for (const auto &deviceExtension : DEVICE_EXTENSIONS) {
		bool hasExtension{ false };
		for (const auto &extension : extensions) {
			if (strcmp(deviceExtension, extension.extensionName) == 0) {
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension) {
			return false;
		}
	}
	return true;
}

bool VulkanRenderer::checkDeviceSuitable(const VkPhysicalDevice &device) {
	/*
	// Info about the device. (id, name, type, vendor ,etc)
	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(device, &deviceProps);

	// Info about what the device can do. (geo shader, tess shader, wide lines, etc)
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	*/
	
	QueueFamilyIndices indices{ getQueueFamilies(device) };

	bool extensionsSupported{ checkDeviceExtensionSupport(device) };

	SwapchainDetails swapChainDetails{ getSwapChainDetails(device) };
	bool swapChainValid{ !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty() };

	return indices.isValid() && extensionsSupported && swapChainValid;
}

bool VulkanRenderer::checkValidationLayerSupport() {
	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : _validationLayers) {
		bool layerFound{ false };
		for (const auto &layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(const VkPhysicalDevice &device)
{
	QueueFamilyIndices indices;

	// Get all queue family property info for the given device.
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProps.data());

	// Go through each queue family and check if it has at least one of the required types of queue.
	int i{ 0 };
	for (const auto &queueFamily : queueFamilyProps) {
		// First check if queue family has at least 1 queue in the family(could have no queues)
		// Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_* to check for required type.
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i; // If queue family is valid, get the index.
		}

		// Check if queue family supports presentation.
		VkBool32 presentationSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentationSupport);
		// Check if queue is presentation type ( can be both graphics and presentation )
		if (queueFamily.queueCount > 0 && presentationSupport) {
			indices.presentationFamily = i;
		}

		// Check if queue family indices are in a valid state.
		if (indices.isValid()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapchainDetails VulkanRenderer::getSwapChainDetails(const VkPhysicalDevice &device) {
	SwapchainDetails swapChainDetails;
	// Get surface capabilities for given device and surface.
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &swapChainDetails.surfaceCapabilities);

	// Get formats
	uint32_t formatCount{ 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
	if (formatCount != 0) {
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, swapChainDetails.formats.data());
	}

	// Presentation modes.
	uint32_t presModeCount{ 0 };
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presModeCount, nullptr);
	if (presModeCount != 0) {
		swapChainDetails.presentationModes.resize(presModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presModeCount, swapChainDetails.presentationModes.data());
	}
	return swapChainDetails;
}

// Best format is subjective, ours will be:
// Format : VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM as backup
// Color Space : VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const vector<VkSurfaceFormatKHR> &formats) {
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		// all formats are available!
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If restricted, search for optimal format.
	for (const auto &format : formats) {
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	// If can't find format, just return whatever is first.
	return formats[0];
}

// We prefer mailbox pres mode, if can't find, use FIF as Vulkan spec says it must be present.
VkPresentModeKHR VulkanRenderer::chooseBestPresMode(const vector<VkPresentModeKHR> presModes) {
	for (const auto &presMode : presModes) {
		if (presMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities) {
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
	if (surfaceCapabilities.currentExtent.width != numeric_limits<uint32_t>::max()) {
		return surfaceCapabilities.currentExtent;
	} else {
		// If value can vary, need to set it manually.
		// Get window size.
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		// Create new extent using window size.
		VkExtent2D newExtent{};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure it's within boundaries by clamping.
		newExtent.width = max(surfaceCapabilities.minImageExtent.width, min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = max(surfaceCapabilities.minImageExtent.height, min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}
