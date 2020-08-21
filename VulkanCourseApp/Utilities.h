#pragma once

#include <vector>
#include <fstream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using std::vector;
using std::string;
using std::ifstream;
using std::ios;

const int MAX_FRAME_DRAWS = 3;
const int MAX_OBJECTS = 1000;

const vector<const char *> DEVICE_EXTENSIONS{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct Vertex {
	glm::vec3 pos; // vertex position (x, y, z)
	glm::vec3 col; // vertex color (r, g, b)
};

// Indices (locations) of Queue Families (if the exist at all)
struct QueueFamilyIndices {
	int graphicsFamily{ -1 }; // location of graphics queue family.
	int presentationFamily{ -1 }; // Location of pres queue family.
	// Check if queue families are valid.
	bool isValid() {
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapchainDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities; // Surface properties.
	vector<VkSurfaceFormatKHR> formats; // Surface image formats.
	vector<VkPresentModeKHR> presentationModes; // How images should be presented to screen.
};

struct SwapchainImage {
	VkImage image;
	VkImageView imageView;
};

static vector<char> readFile(const string &filename) {
	// Open stream from given file.
	// ios::binary tells stream to read as binary, ios::ate tells stream to start reading from the end.
	ifstream file(filename, ios::binary | ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file=" + filename);
	}

	// Get current read position and use to resize file buffer.
	size_t fileSize = (size_t)file.tellg();
	vector<char> fileBuffer(fileSize);
	file.seekg(0); // seek to the start of the file.

	// Read the file data into the buffer (stream "fileSize" in total).
	file.read(fileBuffer.data(), fileSize);

	file.close();

	return fileBuffer;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice phyDev, uint32_t allowedTypes, VkMemoryPropertyFlags propFlags) {
	// Get the properties of my physical device memory.
	VkPhysicalDeviceMemoryProperties memProps{};
	vkGetPhysicalDeviceMemoryProperties(phyDev, &memProps);

	for (uint32_t i{ 0 }; i < memProps.memoryTypeCount; i++) {
		if ((allowedTypes & (1 << i)) // Index of memory type much match corresponding bit in allowedTypes.
			&& (memProps.memoryTypes[i].propertyFlags & propFlags) == propFlags) { // Desired property bit flags are part of memory type's propFlags.
			return i;
		}
	}
}

static void createBuffer(VkPhysicalDevice phyDev, VkDevice device, VkDeviceSize bufferSize, 
	VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags bufferPropFlags, 
	VkBuffer *buffer, VkDeviceMemory *bufferMemory) {

	// Information to create a buffer. (Doesn't include assigning memory.
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferSize; // Size of buffer (size of 1 vertex * num of vertices).
	bufferCreateInfo.usage = bufferUsageFlags; // Multiple types of buffer possible.
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Similar to swapchain images, can share vertex buffers.

	if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a vertex buffer.");
	}

	// GET BUFFER MEMORY REQS
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, *buffer, &memReqs);

	// ALLOCATE MEM TO BUFFER.	
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(phyDev, memReqs.memoryTypeBits, // Index of memory type on Phys Device that has requried bit flags.
		bufferPropFlags
		/*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT // Is memory visible to the host(cpu)?
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Data, after being mapped, is placed directly in the buffer. (otherwise have to flush manually).*/
	);

	// Allocate memory to vk device memory.
	if (vkAllocateMemory(device, &memAllocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate vertex buffer memory.");
	}

	// Allocate memory to given vertex buffery.
	if (vkBindBufferMemory(device, *buffer, *bufferMemory, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind to vertex buffer to memory.");
	}

}

static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer srcBuf, VkBuffer dstBuf, VkDeviceSize bufSize) {
	// Command buffer to hodl transfer command.
	VkCommandBuffer transferCommandBuffer;
		
	// Command buffer details
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = transferCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool.
	if (vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate transfer command buffer.");
	}
		
	// Information to begin the command buffer record.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // We're only using the command buffer once. Set up for one time use.

	// Begin recording transfer commands.
	if (vkBeginCommandBuffer(transferCommandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin transfer command buffer.");
	}

	// Region of data to copy from and to.
	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.srcOffset = 0; // Copy from the beginning.
	bufferCopyRegion.dstOffset = 0; // Copy to the start of the dst.
	bufferCopyRegion.size = bufSize;

	// Command to copy our source buf to dst buf.
	vkCmdCopyBuffer(transferCommandBuffer, srcBuf, dstBuf, 1, &bufferCopyRegion);

	if (vkEndCommandBuffer(transferCommandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end transfer command buffer.");
	}	

	// Queue submission info.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;

	// Submit transfer commands and wait until complete
	if (vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit transfer queue.");
	}

	if (vkQueueWaitIdle(transferQueue) != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for queue.");
	}

	// Free temp cmd buf back to pool
	vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);

}

// refer to https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers for higher levels of configuration.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) {
	printf("validation layer=%s\n", pCallbackData->pMessage);
	return VK_FALSE;
}