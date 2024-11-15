#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApplication {
public:

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:

	void initWindow() {
		//Make a GLFW Window, and make it agnostic to api (OpenGL is default context.
		// We don't want this.)
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//Lets make windows not-resizable for now
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//Create actual window
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {

	}

	void mainLoop() {
		//Main while loop as long as window is open.
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		glfwDestroyWindow(window);

		glfwTerminate();
	}

	GLFWwindow* window;
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}