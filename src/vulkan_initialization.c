#include "config.h"
#include "logger.h"
#include "vulkan_intialization.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#ifdef RAW_PRINTS
#	include <stdio.h>
#endif

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
				free(props);
				return true;
			}
		}
	}

	free(props);

	return false;
}

/**
 * Queries glfw about the required extensions for Vulkan and returns a <strong>mallocated array that needs to be manually freed</strong>
 */
const char **get_required_extensions(uint32_t *count) {
	// get vulkan extensions from glfw
	const char **glfw_exts;

	glfw_exts = glfwGetRequiredInstanceExtensions(count);

#ifdef USE_VALIDATION_LAYERS
	++(*count);
#endif

	auto exts = (const char **)malloc(*count * sizeof(char *));
	memcpy(exts, glfw_exts, *count * sizeof(const char *));

#ifdef USE_VALIDATION_LAYERS
	exts[*count - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
#endif

	return exts;
}

bool create_instance(VulkanRuntimeInfo *vri) {

	// Application information, fairly trivial / uninmportant

	VkApplicationInfo appInfo = {0};

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Neon Genesis Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName        = "None";
	appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_3;

	// what we need to create with vkCreateInstance
	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &appInfo;

	// how many instance we can use
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	VkExtensionProperties *ext_props = (VkExtensionProperties *)(malloc(sizeof(VkExtensionProperties) * extension_count));
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, ext_props);

#ifdef RAW_PRINTS
	llog(LOG_DEBUG, "Available extensions:\n");

	for (unsigned i = 0; i < extension_count; ++i) {
		printf("\t%s\n", ext_props[i].extensionName);
	}
#endif

	free(ext_props);

#ifdef USE_VALIDATION_LAYERS
	createInfo.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
	createInfo.enabledLayerCount = 0;
#endif
	auto exts                          = get_required_extensions(&createInfo.enabledExtensionCount);
	createInfo.ppEnabledExtensionNames = exts;

	auto result = vkCreateInstance(&createInfo, nullptr, &vri->instance);

	free(exts);

	if (result != VK_SUCCESS) {
		llog(LOG_ERROR, "Could not create vulkan instance: %s\n", VkResult_str(result));
		return false;
	}
	return true;
}

bool destroy_instance(VulkanRuntimeInfo *vri) {
	vkDestroyInstance(vri->instance, nullptr);

	return true;
}

bool detach_logger_callback(VulkanRuntimeInfo *vri) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vri->instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func == nullptr) {
		llog(LOG_ERROR, "Could not get the debug destruction function\n");
		return false;
	}

	func(vri->instance, vri->debug_logger, nullptr);

	return true;
}

bool attach_logger_callback(VulkanRuntimeInfo *vri) {
	VkDebugUtilsMessengerCreateInfoEXT info = {0};
	info.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback                    = logger_callback;
	info.pUserData                          = nullptr; // Optional

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vri->instance, "vkCreateDebugUtilsMessengerEXT");

	if (func == nullptr) {
		llog(LOG_ERROR, "Could not get the debug callback creation function\n");
		return false;
	}
	auto res = func(vri->instance, &info, nullptr, &vri->debug_logger);

	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "The debug callback creation function returned %s, could not create the debug callback\n", VkResult_str(res));
		return false;
	}

	return true;
}

QueueFamilyIndicies find_queue_families(VkPhysicalDevice physical_dev) {

	QueueFamilyIndicies res = {0};

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_dev, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "Could not find any queue family\n");
		return res;
	}

	VkQueueFamilyProperties *queues = malloc(sizeof(VkQueueFamilyProperties) * count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_dev, &count, queues);

	for (uint32_t i = 0; i < count; ++i) {
		auto qf = queues[i];
		if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			res.graphics = i;
			set_bit(res.graphics, GRAPHIC_QUEUE_INDEX);
		} else if (qf.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			res.graphics = i;
			set_bit(res.graphics, COMPUTE_QUEUE_INDEX);
		} else if (qf.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			res.graphics = i;
			set_bit(res.graphics, TRANSFER_QUEUE_INDEX);
		}
	}

	free(queues);
	return res;
}

VkPhysicalDevice pick_best_device(const VkPhysicalDevice *devs, const size_t count) {

	auto     choice    = VK_NULL_HANDLE;
	unsigned score     = 0;
	unsigned old_score = 0;

	for (size_t i = 0; i < count; ++i) {
		score = 0;
		auto qfam = find_queue_families(devs[i]);

		if (at_bit(qfam.available_families, GRAPHIC_QUEUE_INDEX)) {
			continue;
		}
		VkPhysicalDeviceProperties dev_props;
		VkPhysicalDeviceFeatures   dev_feats;
		vkGetPhysicalDeviceProperties(devs[i], &dev_props);
		vkGetPhysicalDeviceFeatures(devs[i], &dev_feats);


		if (dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			choice = devs[i];
			break;
		}

		if (old_score < score) {
			old_score = score;
			choice    = devs[i];
		}
	}

	return choice;
}

bool pick_physical_device(VulkanRuntimeInfo *vri) {
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(vri->instance, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "Could not enumerate physical devices\n");
		return false;
	}

	VkPhysicalDevice *devs = malloc(count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(vri->instance, &count, devs);

	pick_best_device(devs, count);
	// TODO: save the selcted device somewhere else

	free(devs);

	return true;
}
