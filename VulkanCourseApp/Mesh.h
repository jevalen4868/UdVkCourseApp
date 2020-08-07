#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Utilities.h"

using std::vector;

class Mesh
{
public:
	Mesh(VkPhysicalDevice physicalDevice, VkDevice device, vector<Vertex> *vertices);

	int getVertexCount();
	VkBuffer getVertexBuffer();

	~Mesh();
private:
	int _vertexCount;
	VkBuffer _vertexBuffer;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	void createVertexBuffer(vector<Vertex> *vertices);
};

