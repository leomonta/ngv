#include "initialization.h"

#include "config.h"
#include "logger.h"
#include "vulkan_initialization.h"

#include <stdio.h>

GLFWwindow *init_window() {
	glfwInit();
	// Don't use OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// TODO: Handle Resizing
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// get the monitor size
	// if unavailable standard 1920x1080
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

#ifdef USE_VALIDATION_LAYERS
	if (check_validation_layer_support() == false) {
		llog(LOG_WARNING, "Validation layers unsupported\n");
	}
#endif

	if (!create_instance(vri)) {
		llog(LOG_INFO, "Vulkan instance created\n");
		return;
	}

#ifdef USE_VALIDATION_LAYERS
	attach_logger_callback(vri);
#endif

	create_surface(vri, init_window());

	pick_physical_device(vri);

	create_logical_device(vri);
}

void terminate_vulkan(VulkanRuntimeInfo *vri) {
#ifdef USE_VALIDATION_LAYERS
	detach_logger_callback(vri);
#endif

	destroy_logical_device(vri);
	destroy_surface(vri);
	destroy_instance(vri);
}

