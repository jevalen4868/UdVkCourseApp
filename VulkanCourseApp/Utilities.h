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
const int MAX_OBJECTS = 10;

const vector<const char *> DEVICE_EXTENSIONS{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

// Scene Settings
struct UboViewProjection {
	glm::mat4 proj;
	glm::mat4 view;
};

struct Vertex {
	glm::vec3 pos; // vertex position (x, y, z)
	glm::vec3 col; // vertex color (r, g, b)
	glm::vec2 tex; // Texture coords (u, v)
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

	throw std::runtime_error("Failed to create a memory type index.");
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

static VkCommandBuffer beginCommandBuffer(VkDevice device, VkCommandPool commandPool) {
	// Command buffer to hodl transfer command.
	VkCommandBuffer commandBuffer;

	// Command buffer details
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool.
	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate transfer command buffer.");
	}

	// Information to begin the command buffer record.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // We're only using the command buffer once. Set up for one time use.

	// Begin recording transfer commands.
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin transfer command buffer.");
	}

	return commandBuffer;
}

static void endAndSubmitCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to end transfer command buffer.");
	}

	// Queue submission info.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Submit transfer commands and wait until complete
	if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit transfer queue.");
	}

	if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for queue.");
	}

	// Free temp cmd buf back to pool
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static void copyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer srcBuf, VkBuffer dstBuf, VkDeviceSize bufSize) {	
	// Create buffer.
	VkCommandBuffer transferCommandBuffer = beginCommandBuffer(device, transferCommandPool);

	// Region of data to copy from and to.
	VkBufferCopy bufferCopyRegion{};
	bufferCopyRegion.srcOffset = 0; // Copy from the beginning.
	bufferCopyRegion.dstOffset = 0; // Copy to the start of the dst.
	bufferCopyRegion.size = bufSize;

	// Command to copy our source buf to dst buf.
	vkCmdCopyBuffer(transferCommandBuffer, srcBuf, dstBuf, 1, &bufferCopyRegion);

	endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue, transferCommandBuffer);
}

static void copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, 
	VkBuffer srcBuf, VkImage dstImg, uint32_t width, uint32_t height) {
	VkCommandBuffer transferCommandBuffer = beginCommandBuffer(device, transferCommandPool);

	VkBufferImageCopy imageRegion{};
	imageRegion.bufferOffset = 0; // Offset into data.
	imageRegion.bufferRowLength = 0; // Row length of data to calculate data spacing.
	imageRegion.bufferImageHeight = 0; // Image height to calculate data spacing. Tightly packed pixels.
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Which aspect of image to copy.
	imageRegion.imageSubresource.mipLevel = 0; // Mipmap level to copy.
	imageRegion.imageSubresource.baseArrayLayer = 0; // Starting array layer (if array)
	imageRegion.imageSubresource.layerCount = 1; // Number of layers to copy starting at baseArrayLayer.
	imageRegion.imageOffset = { 0, 0, 0 }; // Offset into image, as opposed to raw data in buffer offset.
	imageRegion.imageExtent = { width, height, 1 }; // Size of region to copy as (x, y, z) values.
	
	// Copy buffer to given image.
	vkCmdCopyBufferToImage(transferCommandBuffer, 
		srcBuf, 
		dstImg,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Needs to be formattted so it's optimal to have data transferred to it.
		1,
		&imageRegion);

	endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue, transferCommandBuffer);
}

static void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image,
	VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = beginCommandBuffer(device, commandPool);

	VkImageMemoryBarrier imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.oldLayout = oldLayout;
	imageBarrier.newLayout = newLayout;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family to transition from.
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family to transition to.
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of image being altered.
	imageBarrier.subresourceRange.baseMipLevel = 0; // First mip level to start alterations on.
	imageBarrier.subresourceRange.levelCount = 1; // Number of mip levels to alter starting from baseMipLevel.
	imageBarrier.subresourceRange.baseArrayLayer = 0; // First layer to start alterations on.
	imageBarrier.subresourceRange.layerCount = 1; // Number of layers to alter starting from base array layer.

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// If transitioning to from new image to image ready to receive data.
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageBarrier.srcAccessMask = 0x00000000; // Memory access stage transition must happen after...
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Memory access stage transition must happen before...

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// If transitioning from transfer dest to shader readable...
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Memory access stage transition must happen after...
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Memory access stage transition must happen before...

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer,
		srcStage, dstStage, // Pipeline stages, (mast to src and dst access masks.)
		0,
		0,
		nullptr, // memory barriers
		0,
		nullptr, // buffer memory barrier.
		1,
		&imageBarrier);

	endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);

}

// refer to https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers for higher levels of configuration.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData) {
	printf("validation layer=%s\n", pCallbackData->pMessage);
	return VK_FALSE;
}