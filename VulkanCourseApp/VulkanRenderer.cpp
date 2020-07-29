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
	}
	catch (const std::runtime_error &e) {
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::destroy() {
	// setup validation layer for destruction.
	if (_enableValidationLayers) {
		auto destroyMessengerFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyMessengerFunc != nullptr) {
			destroyMessengerFunc(_instance, _debugMessenger, nullptr);
		}
	}

	vkDestroyDevice(_mainDevice.logicalDevice, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);
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
	}
	else {
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
	SwapChainDetails swapChainDetails{ getSwapChainDetails(_mainDevice.physicalDevice) }; 
	
	// Find optimal surface values.
	// Choose best surface format.
	VkSurfaceFormatKHR surfaceFormat{ chooseBestSurfaceFormat(swapChainDetails.formats) };
	// Choose best presentation mode.
	VkPresentModeKHR presentMode{ chooseBestPresMode(swapChainDetails.presentationModes) };
	// Choose swap chain image resolution.
	VkExtent2D extent{ chooseSwapExtent(swapChainDetails.surfaceCapabilities) };

	// Creation info for swap chain.
	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
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

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
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

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {
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

	SwapChainDetails swapChainDetails{ getSwapChainDetails(device) };
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

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
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

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
	SwapChainDetails swapChainDetails;
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
