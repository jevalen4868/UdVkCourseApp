#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	

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
	void updateModel(const size_t &modelId, const glm::mat4 &model);
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
	void createDescriptorSetLayout();
	void createPushConstantRange();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void recordCommands(const uint32_t &currentImage);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	void createDebugMessengerExtension();
	void createSync();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateUniformBuffers(const uint32_t &imageIndex);
	void allocateDynamicBufferTransferSpace();
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

	// UTILITY
	VkFormat _swapchainImageFormat;
	VkExtent2D _swapchainExtent;
	VkDebugUtilsMessengerEXT _debugMessenger;
	

	// PIPELINE
	VkRenderPass _renderPass;
	VkPipelineLayout _pipelineLayout;
	VkPipeline _graphicsPipeline;

	// POOLS
	VkCommandPool _graphicsCommandPool;

	// DESCRIPTORS
	VkDescriptorSetLayout _descSetLayout;
	VkDescriptorPool _descPool;

	VkDeviceSize _minUniBufOffset;
	size_t _modelUniAlignment;
	Model *_modelTransferSpace;

	VkPushConstantRange _pushConstRange;

	// Scene Settings
	struct UboViewProjection {
		glm::mat4 proj;
		glm::mat4 view;
	} _uboViewProj;

	const size_t _uboViewProjSize = sizeof(UboViewProjection);

	// variable length vars.
	// Scene Objects
	vector<Mesh> _meshes;

	// - Need one for each command buffer
	vector<VkBuffer> _vpUniformBuffers;
	vector<VkDeviceMemory> _vpUniformBufMems;
	
	vector<VkBuffer> _modelDynUniformBuffers;
	vector<VkDeviceMemory> _modelDynUniformBufMems;
	
	vector<VkDescriptorSet> _descSets;

	// SYNC
	vector<VkSemaphore> _imageAvailable;
	vector<VkSemaphore> _renderFinished;
	vector<VkFence> _drawFences;

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

