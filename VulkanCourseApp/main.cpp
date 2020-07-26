#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main() {
	if (glfwInit() == GLFW_FALSE) {
		printf("Failed to init GLFW bro, might want to check on that bro.");
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *window{ glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr) };

	// Supported extensions.
	uint32_t extensionCount{ 0 };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);	
	printf("Extension Count=%i\n", extensionCount);

	// Required extensions.
	uint32_t requiredInstanceExtensions{ 0 };
	glfwGetRequiredInstanceExtensions(&requiredInstanceExtensions);
	printf("Required Extension Count=%i\n", requiredInstanceExtensions);

	glm::mat4 testMatrix(1.0f);
	glm::vec4 testVector(1.0f);
	auto test{ testMatrix * testVector };

	// Game loop.
	while (!glfwWindowShouldClose(window)) {		
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}