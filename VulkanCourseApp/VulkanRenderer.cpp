#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer() {
}

VulkanRenderer::~VulkanRenderer() {
}

int VulkanRenderer::init(GLFWwindow *newWindow) {
	_window = newWindow;
	try {
		createInstance();
		getPhysicalDevice();
		createLogicalDevice();
	}
	catch (const std::runtime_error &e) {
		printf("error: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::destroy() {
	vkDestroyDevice(_mainDevice.logicalDevice, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

void VulkanRenderer::createInstance() {
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
	vector<const char *> instanceExtensions = vector<const char *>();

	// Set up extensions the instance will use.
	uint32_t glfwExtensionCount{ 0 }; // glfw may require multiple extensions.
	const char **glfwExtensions; // extensions passed as array of c strings, so we need array of pointers.

	// Get glfw extensions.
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Add glfw extensions to list of extensions.
	for (size_t i{ 0 }; i < glfwExtensionCount; i++) {
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	// Check instance extensions supported.
	if (!checkInstanceExtensionSupport(&instanceExtensions)) {
		throw std::runtime_error("VkInstance does not support required extensions.");
	}

	// pp* means pointer to pointer, array of pointers, etc.
	createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// TODO: Set up the validation layers the instance will use.
	createInfo.enabledLayerCount = 0; // No validation for now.
	createInfo.ppEnabledLayerNames = nullptr;

	// Create instance.
	VkResult result{ vkCreateInstance(&createInfo, nullptr, &_instance) };
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a vkInstance.");
	}
}

void VulkanRenderer::createLogicalDevice() {
	// Get the queue family indices for the chosen Physical Device.
	QueueFamilyIndices indices{ getQueueFamilies(_mainDevice.physicalDevice) };

	// Queues the logical device needs to create and info to do so. (only 1 for now)
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; // index of the family to create a queue from.
	queueCreateInfo.queueCount = 1; // Number of queues to create.
	float priority{ 1.0f };
	queueCreateInfo.pQueuePriorities = &priority; // Vulkan needs priority to handle priority for multiple queues.

	// Info to create logica device (sometimes called "device").
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1; // number of queue create infos.
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; // List of queue create info so the device can create required queues.
	deviceCreateInfo.enabledExtensionCount = 0; // number of enabled logical device extensions.
	deviceCreateInfo.ppEnabledExtensionNames = nullptr; // List of enabled logical device extensions.

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
			if (strcmp(checkExtension, extension.extensionName)) {
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
	return indices.isValid();
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

		// Check if queue family indices are in a valid state.
		if (indices.isValid()) {
			break;
		}

		i++;
	}

	return indices;
}
