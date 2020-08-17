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

	float angle{ 0.0f };
	float deltaTime{ 0.0f };
	float lastTime{ 0.0f };
		
	// game loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float startTime = glfwGetTime();
		deltaTime = startTime - lastTime;
		lastTime = startTime;

		#ifdef NDEBUG
		if (deltaTime > 0.05f) {			
			deltaTime = 0.05f;
		}
		#endif	
		
		while ((deltaTime * 1000.0f) < fps) {
			// printf("%f", runTime.count());
			lastTime = glfwGetTime();
			deltaTime = lastTime - startTime;
		}

		angle += 10.0f * deltaTime;
		if (angle > 360.0f) {
			angle -= 360.0f;
		}

		vulkanRenderer.updateModel(
			glm::rotate(
				glm::mat4(1.0f),
				glm::radians(angle),
				glm::vec3(0.0f, 0.0f, 1.0f)
			)
		);

		vulkanRenderer.draw();

	}

	vulkanRenderer.destroy();

	// destroy window
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}