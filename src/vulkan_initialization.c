#include "logger.h"
#include "vulkan_intialization.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

const char    *VALIDATION_LAYERS[]     = {"VK_LAYER_KHRONOS_validation"};
const unsigned VALIDATION_LAYERS_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(char *);

const char *VkResult_str(VkResult res) {

	switch (res) {

	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_UNKNOWN:
		return "VK_ERROR_UNKNOWN";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION:
		return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	case VK_PIPELINE_COMPILE_REQUIRED:
		return "VK_PIPELINE_COMPILE_REQUIRED";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
		return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_NOT_PERMITTED_KHR:
		return "VK_ERROR_NOT_PERMITTED_KHR";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_THREAD_IDLE_KHR:
		return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR:
		return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR:
		return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR:
		return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR:
		return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
		return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
	case VK_INCOMPATIBLE_SHADER_BINARY_EXT:
		return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
	default:
		return "Unkown Error";
	};
}

bool check_validation_layer_support() {
	uint32_t extension_count = 0;
	vkEnumerateInstanceLayerProperties(&extension_count, nullptr);

	VkLayerProperties *props = (VkLayerProperties *)(malloc(sizeof(VkLayerProperties) * extension_count));

	vkEnumerateInstanceLayerProperties(&extension_count, props);

	for (unsigned i = 0; i < VALIDATION_LAYERS_COUNT; ++i) {

		for (unsigned j = 0; j < extension_count; ++j) {
			if (strcmp(VALIDATION_LAYERS[i], props[j].layerName) == 0) {
				llog(LOG_INFO, "Validation layer found\n");
				return true;
			}
		}
	}

	return false;
}

bool create_instance(VkInstance *instance) {
	VkApplicationInfo appInfo = {0};

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Neon Genesis Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "None";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &appInfo;

	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	VkExtensionProperties *ext_props = (VkExtensionProperties *)(malloc(sizeof(VkExtensionProperties) * extension_count));

	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, ext_props);

	llog(LOG_DEBUG, "Available extensions:\n");

	for (unsigned i = 0; i < extension_count; ++i) {
		printf("\t%s\n", ext_props[i].extensionName);
	}

	uint32_t     glfwExtensionCount = 0;
	const char **glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount   = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	createInfo.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;

	auto result = vkCreateInstance(&createInfo, nullptr, instance);

	if (result != VK_SUCCESS) {
		llog(LOG_ERROR, "Could not create vulkan instance: %s\n", VkResult_str(result));
		return false;
	}
	return true;
}

bool destroy_instance(VkInstance *instance) {
	vkDestroyInstance(*instance, nullptr);

	return true;
}
