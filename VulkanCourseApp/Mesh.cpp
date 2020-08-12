#include "Mesh.h"

Mesh::Mesh() {
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, vector<Vertex> *vertices) {
	_vertexCount = vertices->size();
	_physicalDevice = physicalDevice;
	_device = device;
	createVertexBuffer(vertices);
}

int Mesh::getVertexCount() {
	return _vertexCount;
}

VkBuffer Mesh::getVertexBuffer() {
	return _vertexBuffer;
}

void Mesh::destroyVertexBuffer() {
	vkDestroyBuffer(_device, _vertexBuffer, nullptr);
	vkFreeMemory(_device, _vertexBufferMemory, nullptr);
}

Mesh::~Mesh() {
}

void Mesh::createVertexBuffer(vector<Vertex> *vertices) {
	// Get size of buffer needed for vertices.
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	// Create buffer and allocate memory to it.
	createBuffer(_physicalDevice, _device, bufferSize, 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&_vertexBuffer, &_vertexBufferMemory);

	// Map memory to vertex buffer.
	void *data; // - Create pointer to point in normal memory.
	vkMapMemory(_device, _vertexBufferMemory, 0, bufferSize, 0, &data); // - Map vertex buffer memory to data point.
	memcpy(data, vertices->data(), (size_t)bufferSize); // - Copy memory from vertices vector to the data point.
	vkUnmapMemory(_device, _vertexBufferMemory); // Unmap the buffer memory.
}