#include "initialization.h"

#include "logger.h"
#include "vulkan_intialization.h"

#include <stdio.h>

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

void init_vulkan(VulkanRuntimeInfo *vri) {

	if (check_validation_layer_support() == false) {
		llog(LOG_WARNING, "Validation layers unsupported\n");
	}

	llog(LOG_INFO, "TEST\n");

	// TODO: logging
	if (!create_instance(&vri->instance)) {
		return;
	}
}

void terminate_vulkan(VulkanRuntimeInfo *vri) {
	destroy_instance(&vri->instance);
}
