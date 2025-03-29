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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

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

bool init_vulkan(VulkanRuntimeInfo *vri) {

	llog(LOG_DEBUG, "[NGV] Started creating vulkan objects\n");

#ifdef USE_VALIDATION_LAYERS
	llog(LOG_DEBUG, "[NGV] Validation layers are enabled\n");

	if (!check_validation_layer_support()) {
		llog(LOG_WARNING, "[NGV] Validation layers unsupported\n");
	}
#endif

	if (!create_instance(vri)) {
		return false;
	}

#ifdef USE_VALIDATION_LAYERS
	attach_logger_callback(vri);
#endif

	vri->sys_window = init_window();

	if(!create_surface(vri)){
		return false;
	}

	if(!pick_physical_device(vri)){
		return false;
	}

	if(!create_logical_device(vri)){
		return false;
	}

	if(!create_swapchain(vri)){
		return false;
	}
	
	if(!create_image_views(vri)){
		return false;
	}

	if(!create_renderpass(vri)){
		return false;
	}

	if(!create_pipeline(vri)){
		return false;
	}

	if(!create_framebuffers(vri)){
		return false;
	}

	if(!create_command_pool(vri)){
		return false;
	}

	if(!create_command_buffer(vri)){
		return false;
	}

	if(!create_sync_objects(vri)){
		return false;
	}

	
	llog(LOG_DEBUG, "[NGV] Finished creating vulkan objects\n");
	return true;

}

void terminate_vulkan(VulkanRuntimeInfo *vri) {

	llog(LOG_DEBUG, "[NGV] Started destroying vulkan objects\n");

#ifdef USE_VALIDATION_LAYERS
	detach_logger_callback(vri);
#endif
	destroy_sync_objects(vri);

	destroy_command_pool(vri);

	destroy_framebuffers(vri);

	destroy_pipeline(vri);

	destroy_renderpass(vri);

	destroy_image_views(vri);

	destroy_swapchain(vri);

	destroy_logical_device(vri);

	destroy_surface(vri);

	destroy_instance(vri);

	llog(LOG_DEBUG, "[NGV] Finished destroying vulkan objects\n");

}
