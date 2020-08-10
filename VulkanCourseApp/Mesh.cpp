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
	// Information to create a buffer. (Doesn't include assigning memory.
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = sizeof(Vertex) * vertices->size(); // Size of buffer (size of 1 vertex * num of vertices).
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // Multiple types of buffer possible. We want vertex buffer.
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Similar to swapchain images, can share vertex buffers.

	if (vkCreateBuffer(_device, &bufferCreateInfo, nullptr, &_vertexBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a vertex buffer.");
	}

	// GET BUFFER MEMORY REQS
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(_device, _vertexBuffer, &memReqs);

	// ALLOCATE MEM TO BUFFER.	
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(memReqs.memoryTypeBits, // Index of memory type on Phys Device that has requried bit flags.
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT // Is memory visible to the host(cpu)?
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Data, after being mapped, is placed directly in the buffer. (otherwise have to flush manually).
	);

	// Allocate memory to vk device memory.
	if (vkAllocateMemory(_device, &memAllocInfo, nullptr, &_vertexBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate vertex buffer memory.");
	}
		
	// Allocate memory to given vertex buffery.
	if (vkBindBufferMemory(_device, _vertexBuffer, _vertexBufferMemory, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind to vertex buffer to memory.");
	}

	// Map memory to vertex buffer.
	void *data; // - Create pointer to point in normal memory.
	vkMapMemory(_device, _vertexBufferMemory, 0, bufferCreateInfo.size, 0, &data); // - Map vertex buffer memory to data point.
	memcpy(data, vertices->data(), (size_t)bufferCreateInfo.size); // - Copy memory from vertices vector to the data point.
	vkUnmapMemory(_device, _vertexBufferMemory); // Unmap the buffer memory.
}

uint32_t Mesh::findMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propFlags) {
	// Get the properties of my physical device memory.
	VkPhysicalDeviceMemoryProperties memProps{};
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProps);
	
	for (uint32_t i{ 0 }; i < memProps.memoryTypeCount; i++) {
		if ((allowedTypes & (1 << i)) // Index of memory type much match corresponding bit in allowedTypes.
			&& (memProps.memoryTypes[i].propertyFlags & propFlags) == propFlags) { // Desired property bit flags are part of memory type's propFlags.
			return i;
		}
	}
}
