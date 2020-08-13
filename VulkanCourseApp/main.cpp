#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <chrono>

#include "VulkanRenderer.h"

using std::string;
using std::vector;

GLFWwindow *window;
VulkanRenderer vulkanRenderer;
float fps{ 1000.0f / 144.0f };

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

	auto startTime{ std::chrono::high_resolution_clock::now() };
	auto endTime{ std::chrono::high_resolution_clock::now() };
	std::chrono::duration<float, std::milli> runTime(endTime - startTime); 
	// game loop
	while (!glfwWindowShouldClose(window)) {
		startTime = std::chrono::high_resolution_clock::now();

		glfwPollEvents();
		vulkanRenderer.draw();

		endTime = std::chrono::high_resolution_clock::now();
		runTime = endTime - startTime;		
		
		#ifdef NDEBUG
		if (runTime.count() > 0.05f) {
			// TODO make sure 5 milli is an appropriate value.
			runTime = std::chrono::duration<float, std::milli>(5.0f);
		}
		#endif	
		
		while (runTime.count() < fps) {
			// printf("%f", runTime.count());
			endTime = std::chrono::high_resolution_clock::now();
			runTime = endTime - startTime;
		}
	}

	vulkanRenderer.destroy();

	// destroy window
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}