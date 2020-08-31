#define STB_IMAGE_IMPLEMENTATION

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <chrono>

#include "VulkanRenderer.h"
#include "Utilities.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;

GLFWwindow *window;
unique_ptr<VulkanRenderer> vulkanRenderer;
float constexpr FRAMES_PER_SECOND{ 144.0f };
float constexpr FPS{ 1000.0f / FRAMES_PER_SECOND };

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
	const int width{ 1920 };
	const int height{ 1080 };
	initWindow("Test Window", width, height);

	vulkanRenderer = make_unique<VulkanRenderer>();

	// create vulkan renderer instance
	if (vulkanRenderer->init(window) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	float angle{ 0.0f };
	float deltaTime{ 0.0f };
	float lastTime{ 0.0f };

	int man{ vulkanRenderer->createMeshModel("Models/FinalBaseMesh.obj") };
	//int ironMan{ vulkanRenderer->createMeshModel("Models/IronMan.obj") };

	//vulkanRenderer->createMeshModel("Models/chopper.obj");
	//int audi{ vulkanRenderer->createMeshModel("Models/Audi_R8_2017.obj") };
		
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
		
		/*
		while ((deltaTime * 1000.0f) < FPS) {
			// printf("%f", runTime.count());
			lastTime = glfwGetTime();
			deltaTime = lastTime - startTime;
		}*/

		angle += 10.0f * deltaTime;
		if (angle > 360.0f) {
			angle -= 360.0f;
		}
		
		glm::mat4 testMat{
			glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f))
		};
		testMat = glm::translate(testMat, glm::vec3{ 0.0f, -10.0f, -10.0f });
		//testMat = glm::rotate(testMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		vulkanRenderer->updateModel(man, testMat);
		
		/*
		// Update camera
		const float fovy{ glm::radians(angle) };
		const float aspect{ width / height };
		const float tanHalfFovy = tan(fovy / static_cast<float>(2));

		UboViewProjection *uboViewProj{ vulkanRenderer->getViewProj() };
		
		uboViewProj->proj[0][0] = static_cast<float>(1) / (aspect * tanHalfFovy);
		uboViewProj->proj[1][1] = static_cast<float>(1) / (tanHalfFovy);

		uboViewProj->proj[1][1] *= -1;

		vulkanRenderer->setViewProj(uboViewProj);
		*/

		/*
		glm::mat4 firstModel(1.0f);
		glm::mat4 secondModel(1.0f);

		firstModel = glm::translate(firstModel, glm::vec3(-1.0f, 0.0f, -1.5f));
		firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

		secondModel = glm::translate(secondModel, glm::vec3(1.0f, 0.0f, -3.0f));
		secondModel = glm::rotate(secondModel, glm::radians(-angle * 10), glm::vec3(0.0f, 0.0f, 1.0f));

		vulkanRenderer->updateModel(0, firstModel);
		vulkanRenderer->updateModel(1, secondModel);
		*/

		vulkanRenderer->draw();

	}

	vulkanRenderer->destroy();

	// destroy window
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}