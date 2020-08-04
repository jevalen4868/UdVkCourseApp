#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "VulkanRenderer.h"

using std::string;
using std::vector;

GLFWwindow *window;
VulkanRenderer vulkanRenderer;

void initWindow(string wName = "Test Window", const int width = 800, const int height = 600) {
	// init glfw
	if (!glfwInit()) {

	}
	// set glfw to not work with opengl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
}

int main() {
	// create window
	initWindow("Test Window", 1920, 1080);

	// create vulkan renderer instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	// game loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		vulkanRenderer.draw();
	}

	vulkanRenderer.destroy();

	// destroy window
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}