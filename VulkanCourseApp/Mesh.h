#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

using std::vector;

class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, vector<Vertex> *vertices);

	int getVertexCount();
	VkBuffer getVertexBuffer();

	void destroyVertexBuffer();

	~Mesh();
private:
	int _vertexCount;
	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	void createVertexBuffer(vector<Vertex> *vertices);
	uint32_t findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags properties);
};

