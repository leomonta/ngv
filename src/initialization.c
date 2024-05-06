#include "initialization.h"
#include "vulkan_intialization.h"

GLFWwindow *init_window() {
	glfwInit();
	// Don't use OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// TODO: Handle Resizing
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// get the monitor size
	// if unavilable standard 1920x1080
	auto const mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int        wt = DEFAULT_WINDOW_WIDTH, ht = DEFAULT_WINDOW_HEIGHT;
	if (mode != NULL) {
		ht = mode->height;
		wt = mode->width;
	}
	auto window = glfwCreateWindow(wt, ht, "ngv", nullptr, nullptr);

	return window;
}

void terminate_window(GLFWwindow *wndw) {
	glfwDestroyWindow(wndw);
	glfwTerminate();
}

void init_vulkan() {

	VulkanRuntimeInfo vri = {0};

	// TODO: logging
	if(!create_instance(&vri.instance)) {
		return;
	}
}
