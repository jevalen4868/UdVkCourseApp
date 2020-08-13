#include "Mesh.h"

Mesh::Mesh() {
}

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device,
	VkQueue transferQueue, VkCommandPool transferCommandPool,
	vector<Vertex> *vertices, vector<uint32_t> *indices) {
	_vertexCount = vertices->size();
	_indexCount = indices->size();
	_physicalDevice = physicalDevice;
	_device = device;
	createVertexBuffer(transferQueue, transferCommandPool, vertices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);
}

int Mesh::getVertexCount() {
	return _vertexCount;
}

int Mesh::getIndexCount() {
	return _indexCount;
}

VkBuffer Mesh::getVertexBuffer() {
	return _vertexBuffer;
}

VkBuffer Mesh::getIndexBuffer() {
	return _indexBuffer;
}

void Mesh::destroyBuffers() {
	vkDestroyBuffer(_device, _vertexBuffer, nullptr);
	vkFreeMemory(_device, _vertexBufferMemory, nullptr);
	vkDestroyBuffer(_device, _indexBuffer, nullptr);
	vkFreeMemory(_device, _indexBufferMemory, nullptr);
}

Mesh::~Mesh() {
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, vector<Vertex> *vertices) {
	// Get size of buffer needed for vertices.
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	// Temporary buffer to "stage" vertex data before transferring to GPU.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufMem;

	// Create staging buffer and allocate memory to it.
	createBuffer(_physicalDevice, _device, bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufMem);

	// Map memory to vertex buffer.
	void *data; // - Create pointer to point in normal memory.
	vkMapMemory(_device, stagingBufMem, 0, bufferSize, 0, &data); // - Map vertex buffer memory to data point.
	memcpy(data, vertices->data(), (size_t)bufferSize); // - Copy memory from vertices vector to the data point.
 	vkUnmapMemory(_device, stagingBufMem); // Unmap the buffer memory.

	// Create buffer with transfer dst bit to mark recipient of transfer data. (also VERTEX_BUFFER)
	// Buffer memory is DEVICE_LOCAL_BIT which means memory is on the GPU and only accessible by it and not the CPU(HOST).
	createBuffer(_physicalDevice, _device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT
		| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&_vertexBuffer, &_vertexBufferMemory
		);

	// Graphics family per vulkan standard should include a transfer family.

	// Copy staging buffer to vertex buffer on GPU.
	copyBuffer(_device, transferQueue, transferCommandPool, stagingBuffer, _vertexBuffer, bufferSize);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingBufMem, nullptr);
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, vector<uint32_t> *indices) {
	// Get size of buffer needed for indices.
	VkDeviceSize bufferSize{ sizeof(uint32_t) * indices->size() };

	// Temporary buffer to "stage" index data before transferring to GPU.
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufMem;
	createBuffer(_physicalDevice, _device, bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		&stagingBuffer, &stagingBufMem);

	// Map memory to index buffer.
	void *data; // - Create pointer to point in normal memory.
	vkMapMemory(_device, stagingBufMem, 0, bufferSize, 0, &data); // - Map vertex buffer memory to data point.
	memcpy(data, indices->data(), (size_t)bufferSize); // - Copy memory from indices vector to the data point.
	vkUnmapMemory(_device, stagingBufMem); // Unmap the buffer memory.

	// Create buffer for index data in GPU access only area.
	createBuffer(_physicalDevice, _device, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT
		| VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&_indexBuffer, &_indexBufferMemory);

	// Copy staging buffer to index buffer on GPU.
	copyBuffer(_device, transferQueue, transferCommandPool, stagingBuffer, _indexBuffer, bufferSize);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingBufMem, nullptr);
}
