#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>

#include "Utilities.h"

using std::vector;
using std::set;
using std::numeric_limits;
using std::min;
using std::max;

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
	void createSurface();
	void createSwapChain();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void createDebugMessengerExtension();
	// - get functions
	void getPhysicalDevice();
	vector<const char *> getRequiredExtensions();
	// - support functions
	// -- checker functions
	bool checkInstanceExtensionSupport(const vector<const char *> *checkExtensions);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkDeviceSuitable(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	// -- Getter functions.
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);
	// -- Choose functions.
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const vector<VkSurfaceFormatKHR> &formats);
	VkPresentModeKHR chooseBestPresMode(const vector <VkPresentModeKHR> presModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

	// VARS
	GLFWwindow *_window;

	// vulkan components
	VkInstance _instance;
	VkQueue _graphicsQueue;
	VkQueue _presentationQueue;
	struct {
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} _mainDevice;
	VkSurfaceKHR _surface;
	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	VkDebugUtilsMessengerEXT _debugMessenger;

	// variable length vars.
	vector<SwapchainImage> _swapchainImages;
	const vector<const char *> _validationLayers {
		"VK_LAYER_KHRONOS_validation",
	};

	#ifdef NDEBUG
		const bool _enableValidationLayers = false;
	#else
		const bool _enableValidationLayers = true;
	#endif	
};

