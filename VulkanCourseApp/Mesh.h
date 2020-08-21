#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

using std::vector;

struct UboModel {
	glm::mat4 model;
};

class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, 
		VkQueue transferQueue, VkCommandPool transferCommandPool, 
		vector<Vertex> *vertices, vector<uint32_t> *indices);

	void setModel(glm::mat4 model);
	UboModel getModel();

	int getVertexCount();
	int getIndexCount();
	VkBuffer getVertexBuffer();
	VkBuffer getIndexBuffer();

	void destroyBuffers();

	~Mesh();
private:
	int _vertexCount;
	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	// variable data.
	int _indexCount;
	VkBuffer _indexBuffer;
	VkDeviceMemory _indexBufferMemory;

	UboModel _uboModel;

	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, vector<Vertex> *vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, vector<uint32_t> *indices);
};

