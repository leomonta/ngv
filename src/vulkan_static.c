#include "vulkan_static.h"

#include "config.h"
#include "logger.h"
#include "utils.h"
#include "vkinit_utils.h"

#include <errno.h>
#include <string.h>

bool create_static_info(VulkanStaticInfo *static_info, VulkanStaticSettings settings) {
	if (!create_instance(&static_info->vulkan_instance)) {
		return false;
	}
	if (!create_surface(static_info)) {
		return false;
	}
	if (!init_window(&static_info->system_window)) {
		return false;
	}
	if (!pick_physical_device(settings, static_info)) {
		return false;
	}
	return true;
}

bool destroy_static_info(VulkanStaticInfo *static_info) {
	if (!terminate_window(static_info->system_window)) {
		return false;
	}
	if (!destroy_surface(static_info)) {
		return false;
	}
	if (!destroy_instance(static_info)) {
		return false;
	}
	return true;
}

bool create_instance(VkInstance *instance) {

	// Application information, fairly trivial / unimportant

	VkApplicationInfo app_create_info = {
	    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pApplicationName   = NGV_DEFUALT_APPLICATION_NAME,
	    .applicationVersion = NGV_APPLICATION_VERSION,
	    .pEngineName        = NGV_ENGINE_NAME,
	    .engineVersion      = NGV_ENGINE_VERSION,
	    .apiVersion         = VK_API_VERSION_1_4,
	    .pNext              = nullptr,
	};

	// what we need to create with vkCreateInstance
	VkInstanceCreateInfo instance_create_info = {
	    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .pApplicationInfo        = &app_create_info,
	    .enabledLayerCount       = 0,
	    .ppEnabledLayerNames     = nullptr,
	    .enabledExtensionCount   = 0,
	    .ppEnabledExtensionNames = nullptr,
	    .flags                   = 0, // None available
	    .pNext                   = nullptr,
	};

#ifdef USE_VALIDATION_LAYERS
	instance_create_info.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
	createInfo.enabledLayerCount = 0;
#endif

	auto exts = get_required_extensions(&instance_create_info.enabledExtensionCount);
	if (exts == nullptr) {
		return false;
	}
	instance_create_info.ppEnabledExtensionNames = exts;

	auto result = vkCreateInstance(&instance_create_info, nullptr, instance);

	free(exts);

	if (result != VK_SUCCESS) {
		llog(LOG_FATAL, "[INSTANCE] Could not create vulkan instance: %s\n", VkResult_str(result));
		return false;
	}

	llog(LOG_DEBUG, "[INSTANCE] Vulkan instance successfully created\n");
	return true;
}

bool destroy_instance(VulkanStaticInfo *static_info) {

	vkDestroyInstance(static_info->vulkan_instance, nullptr);

	llog(LOG_DEBUG, "[INSTANCE] Vulkan instance successfully destroyed\n");

	return true;
}

const char **get_required_extensions(uint32_t *count) {

	// get vulkan extensions from glfw
	const char **glfw_exts;
	uint32_t     glfw_count;

	glfw_exts = glfwGetRequiredInstanceExtensions(&glfw_count);

#ifdef USE_VALIDATION_LAYERS
	*count = glfw_count + VALIDATION_EXTENSIONS_COUNT;
#else
	*count = glfw_count;
#endif

	auto exts = (const char **)malloc(*count * sizeof(char *));
	TEST_MALLOC_RET(exts, nullptr)
	memcpy(exts, glfw_exts, glfw_count * sizeof(const char *));

#ifdef USE_VALIDATION_LAYERS
	exts[*count - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

	return exts;
}

bool init_window(GLFWwindow **window) {

	// renderdoc does not supper wayland
#ifdef DO_RENDERDOC
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	glfwWindowHint(GLFW_X11_XCB_VULKAN_SURFACE, GLFW_FALSE);
#endif

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

	*window = glfwCreateWindow(wt, ht, "ngv", nullptr, nullptr);

	return true;
}

bool terminate_window(GLFWwindow *window) {
	glfwDestroyWindow(window);
	glfwTerminate();

	return true;
}

bool create_surface(VulkanStaticInfo *static_info) {

	auto res = glfwCreateWindowSurface(static_info->vulkan_instance, static_info->system_window, nullptr, &static_info->surface);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[SURFACE] Could not create the Vulkan Surface: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[DEBUG] Surface successfully created\n");

	return true;
}

bool destroy_surface(VulkanStaticInfo *static_info) {

	vkDestroySurfaceKHR(static_info->vulkan_instance, static_info->surface, nullptr);

	llog(LOG_DEBUG, "[DEBUG] Surface successfully destroyed\n");

	return true;
}

VkPhysicalDevice get_chosen_device(const VkPhysicalDevice *devs, const uint32_t count, const uint32_t preferred_dev_id) {
	VkPhysicalDeviceProperties props;

	VkPhysicalDevice res = VK_NULL_HANDLE;

	for (size_t i = 0; i < count; ++i) {
		vkGetPhysicalDeviceProperties(devs[i], &props);
		llog(LOG_DEBUG, "[PHYSICAL DEVICE] ID = %ld\n", props.deviceID);
		if (props.deviceID == preferred_dev_id) {
			res = devs[i];
		}
	}

	return res;
}

bool pick_physical_device(const VulkanStaticSettings settings, VulkanStaticInfo *static_info) {

	uint32_t count = 0;
	vkEnumeratePhysicalDevices(static_info->vulkan_instance, &count, nullptr);

	llog(LOG_DEBUG, "[PHYSICAL DEVICE] count = %d\n", count);

	VkPhysicalDevice *devs = malloc(count * sizeof(VkPhysicalDevice));
	TEST_MALLOC(devs)
	vkEnumeratePhysicalDevices(static_info->vulkan_instance, &count, devs);

	VkPhysicalDevice chosen_dev;

	if (settings.use_preferred_device) {
		chosen_dev = devs[0];
	} else {
		chosen_dev = get_chosen_device(devs, count, settings.preferred_physical_device_id); // devs[VULKAN_CHOSEN_PHYSICAL_DEVICE_INDEX];
	}

	if (chosen_dev == VK_NULL_HANDLE) {
		llog(LOG_FATAL, "[PHYSICAL DEVICE] Could not find a suitable physical device\n");
		free(devs);
		return false;
	}
	static_info->physical_dev = chosen_dev;

	free(devs);

	llog(LOG_DEBUG, "[PHYSICAL DEVICE] Physical device successfully created\n");

	return true;
}
