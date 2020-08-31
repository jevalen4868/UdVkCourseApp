#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>	

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <array>

#include "Utilities.h"
#include "Mesh.h"
#include "stb_image.h"
#include "MeshModel.h"

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
	int createMeshModel(string modelFile);
	UboViewProjection *getViewProj();
	void setViewProj(const UboViewProjection *viewProj);

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
	void createDepthBufferImage();
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
	void createTextureSampler();
	void updateUniformBuffers(const uint32_t &imageIndex);
	void allocateDynamicBufferTransferSpace();
	// - get functions
	void getPhysicalDevice();
	// - support functions
	VkImage createImage(const uint32_t &width, const uint32_t &height, const VkFormat &format, const VkImageTiling &tiling,
		const VkImageUsageFlags &usageFlags, const VkMemoryPropertyFlags &memPropFlags, VkDeviceMemory *imageMemory);
	VkImageView createImageView(const VkImage &image, const VkFormat &format, const VkImageAspectFlags &aspectFlags);
	VkShaderModule createShaderModule(const vector<char> &code);
	vector<const char *> getRequiredExtensions();
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
	VkFormat chooseSupportedFormat(const vector<VkFormat> &formats, const VkImageTiling &tiling, const VkFormatFeatureFlags &featureFlags);
	// -- Loader functions
	stbi_uc *loadTextureFile(const string &fileName, int *width, int *height, VkDeviceSize *imageSize);
	
	int createTextureImage(const string &fileName);
	int createTexture(const string &fileName);
	int createTextureDescriptor(VkImageView texImg);

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
	VkDescriptorSetLayout _samplerSetLayout;
	VkDescriptorPool _descPool;
	VkDescriptorPool _samplerDescPool;

	//VkDeviceSize _minUniBufOffset;
	//size_t _modelUniAlignment;
	//UboModel *_modelTransferSpace;

	VkPushConstantRange _pushConstRange;

	VkImage _depthBufImage;
	VkDeviceMemory _depthBufImageMem;
	VkImageView _depthBufImageView;
	VkFormat _depthBufFormat;

	VkSampler _textureSampler;
	
	UboViewProjection _uboViewProj;

	const size_t _uboViewProjSize = sizeof(UboViewProjection);

	// variable length vars.
	// Scene Objects
	//vector<Mesh> _meshes;

	// - Need one for each command buffer
	vector<VkBuffer> _vpUniformBuffers;
	vector<VkDeviceMemory> _vpUniformBufMems;
	
	//vector<VkBuffer> _modelDynUniformBuffers;
	//vector<VkDeviceMemory> _modelDynUniformBufMems;
	
	vector<VkDescriptorSet> _descSets;
	vector<VkDescriptorSet> _samplerDescSets;

	// - Assets
	vector<VkImage> _textureImages;
	vector<VkDeviceMemory> _textureImageMems;
	vector<VkImageView> _textureImageViews;
	vector<MeshModel> _models;

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

