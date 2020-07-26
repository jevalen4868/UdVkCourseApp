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
	// - get functions
	void getPhysicalDevice();
	// - support functions
	// -- checker functions
	bool checkInstanceExtensionSupport(const vector<const char *> *checkExtensions);
	bool checkDeviceSuitable(VkPhysicalDevice device);
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

};

