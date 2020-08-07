#include "Mesh.h"

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, vector<Vertex> *vertices) 
	: _vertexCount{ vertices->size() },
	_physicalDevice{ physicalDevice },
	_device{ device } {
	createVertexBuffer(vertices);
}

int Mesh::getVertexCount() {
	return _vertexCount;
}

VkBuffer Mesh::getVertexBuffer() {
	return _vertexBuffer;
}

void Mesh::createVertexBuffer(vector<Vertex> *vertices) {
	// Information to create a buffer. (Doesn't include assigning memory.
	VkBufferCreateInfo info{};
}
