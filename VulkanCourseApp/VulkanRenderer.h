#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>
#include "Utilities.h"
#include "Mesh.h"

using std::vector;
using std::set;
using std::numeric_limits;
using std::array;
using std::min;
using std::max;

class VulkanRenderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();
	int init(GLFWwindow *newWindow);
	void draw();
	void destroy();
private:
	// FUNCTIONS
	// vulkan functions
	// - create functions
	void createInstance();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void recordCommands();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void createDebugMessengerExtension();
	void createSync();
	VkImageView createImageView(const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags);
	VkShaderModule createShaderModule(const vector<char> &code);
	// - get functions
	void getPhysicalDevice();
	vector<const char *> getRequiredExtensions();
	// - support functions
	// -- checker functions
	bool checkInstanceExtensionSupport(const vector<const char *> *checkExtensions);
	bool checkDeviceExtensionSupport(const VkPhysicalDevice &device);
	bool checkDeviceSuitable(const VkPhysicalDevice &device);
	bool checkValidationLayerSupport();
	// -- Getter functions.
	QueueFamilyIndices getQueueFamilies(const VkPhysicalDevice &device);
	SwapchainDetails getSwapChainDetails(const VkPhysicalDevice &device);
	// -- Choose functions.
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const vector<VkSurfaceFormatKHR> &formats);
	VkPresentModeKHR chooseBestPresMode(const vector <VkPresentModeKHR> presModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

	// VARS
	int _currentFrame{ 0 };
	GLFWwindow *_window;

	// Scene Objects
	vector<Mesh> _meshes;

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
	
	// PIPELINE
	VkRenderPass _renderPass;
	VkPipelineLayout _pipelineLayout;
	VkPipeline _graphicsPipeline;

	// POOLS
	VkCommandPool _graphicsCommandPool;

	// SYNC
	vector<VkSemaphore> _imageAvailable;
	vector<VkSemaphore> _renderFinished;
	vector<VkFence> _drawFences;

	// variable length vars.
	vector<SwapchainImage> _swapchainImages;
	vector<VkFramebuffer> _swapchainFramebuffers;
	vector<VkCommandBuffer> _commandBuffers;

	const vector<const char *> _validationLayers {
		"VK_LAYER_KHRONOS_validation",
	};

	#ifdef NDEBUG
		const bool _enableValidationLayers = false;
	#else
		const bool _enableValidationLayers = true;
	#endif	
};

