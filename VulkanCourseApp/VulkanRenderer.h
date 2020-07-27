#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

#include "Utilities.h"

using std::vector;

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	int init(GLFWwindow *newWindow);
	void destroy();
private:
	// FUNCTIONS
	// vulkan functions
	// - create functions
	void createInstance();
	void createLogicalDevice();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void createDebugMessengerExtension();
	// - get functions
	void getPhysicalDevice();
	vector<const char *> getRequiredExtensions();
	// - support functions
	// -- checker functions
	bool checkInstanceExtensionSupport(const vector<const char *> *checkExtensions);
	bool checkDeviceSuitable(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	// -- Getter functions.
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

	// VARS
	GLFWwindow *_window;

	// vulkan components
	VkInstance _instance;
	VkQueue _graphicsQueue;
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} _mainDevice;
	VkDebugUtilsMessengerEXT _debugMessenger;

	// variable length vars.
	const vector<const char *> _validationLayers {
		"VK_LAYER_KHRONOS_validation",
	};

	#ifdef NDEBUG
		const bool _enableValidationLayers = false;
	#else
		const bool _enableValidationLayers = true;
	#endif	
};

