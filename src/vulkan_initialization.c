#include "vulkan_initialization.h"

#include "config.h"
#include "logger.h"
#include "vkinit_utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef RAW_PRINTS
#	include <stdio.h>
#endif

const char    *PHYSICAL_EXTENSIONS[]    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const unsigned PHYSICAL_EXTENSION_COUNT = sizeof(PHYSICAL_EXTENSIONS) / sizeof(char *);
const char    *VALIDATION_LAYERS[]      = {"VK_LAYER_KHRONOS_validation"};
const unsigned VALIDATION_LAYERS_COUNT  = sizeof(VALIDATION_LAYERS) / sizeof(char *);

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
 * Queries glfw about the required extensions for Vulkan and returns a **mallocated array that needs to be manually freed**
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

	VkApplicationInfo app_create = {0};

	app_create.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_create.pApplicationName   = "Neon Genesis Vulkan";
	app_create.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_create.pEngineName        = "None";
	app_create.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
	app_create.apiVersion         = VK_API_VERSION_1_0;

	// what we need to create with vkCreateInstance
	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &app_create;

	// how many instance we can use
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	VkExtensionProperties *ext_props = (VkExtensionProperties *)(malloc(sizeof(VkExtensionProperties) * extension_count));
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, ext_props);

#ifdef RAW_PRINTS
	llog(LOG_DEBUG, "Available instance extensions:\n");

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
		llog(LOG_FATAL, "Could not create vulkan instance: %s\n", VkResult_str(result));
		return false;
	}
	return true;
}

bool destroy_instance(VulkanRuntimeInfo *vri) {

	vkDestroyInstance(vri->instance, nullptr);

	return true;
}

bool attach_logger_callback(VulkanRuntimeInfo *vri) {

	VkDebugUtilsMessengerCreateInfoEXT db_create = {0};
	db_create.sType                              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	db_create.messageSeverity                    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	db_create.messageType                        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	db_create.pfnUserCallback                    = logger_callback;
	db_create.pUserData                          = nullptr; // Optional

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vri->instance, "vkCreateDebugUtilsMessengerEXT");

	if (func == nullptr) {
		llog(LOG_ERROR, "Could not get the debug callback creation function\n");
		return false;
	}
	auto res = func(vri->instance, &db_create, nullptr, &vri->debug_logger);

	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "The debug callback creation function returned %s, could not create the debug callback\n", VkResult_str(res));
		return false;
	}

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

bool pick_physical_device(VulkanRuntimeInfo *vri) {

	uint32_t count = 0;
	vkEnumeratePhysicalDevices(vri->instance, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "Could not enumerate physical devices\n");
		return false;
	}

	VkPhysicalDevice *devs = malloc(count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(vri->instance, &count, devs);

	auto best_dev = pick_best_device(devs, count, vri->surface);
	if (best_dev == VK_NULL_HANDLE) {
		llog(LOG_FATAL, "Could not find a suitable physical device\n");
		return false;
	}
	vri->physical_dev = best_dev;

	free(devs);

	return true;
}

bool create_logical_device(VulkanRuntimeInfo *vri) {

	auto indices = get_queue_families(vri->physical_dev, vri->surface);

	// if bot GRAPHIC_QUEUE_INDEX and PRESENT_QUEUE_INDEX are set
	if ((indices.available_families & (GRAPHIC_QUEUE_INDEX | PRESENT_QUEUE_INDEX)) != (GRAPHIC_QUEUE_INDEX | PRESENT_QUEUE_INDEX)) {
		return false;
	}

	constexpr uint32_t num_needed_queues = 2;

	uint32_t                needed_queues[num_needed_queues] = {indices.graphics, indices.present};
	VkDeviceQueueCreateInfo q_create[num_needed_queues]      = {0};

	// I need to ensure that if a family supports multiple queues
	// I only add it once to the logical device creation struct
	uint32_t num_unique_queues = num_needed_queues;

	// check if there is a duplice index
	// if so replace it with the one at the end and consider the array 1 element smaller
	// FIXME: this will bite me in the ass in the future
	for (uint32_t i = 0; i < num_unique_queues; ++i) {
		for (uint32_t j = 0; j < i; ++j) {
			// duplicate
			if (needed_queues[i] == needed_queues[j]) {
				// replace with last element
				needed_queues[i] = needed_queues[num_unique_queues - 1];
				// the newly replaced could also be duplicate
				// check it
				--i;
				--num_unique_queues;
			}
		}
	}

	float q_priority = 1.0f;
	for (uint32_t i = 0; i < num_unique_queues; ++i) {

		q_create[i].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		q_create[i].queueFamilyIndex = needed_queues[i];
		q_create[i].queueCount       = 1;
		q_create[i].pQueuePriorities = &q_priority;
	}

	VkPhysicalDeviceFeatures dev_features = {0};
	VkDeviceCreateInfo       dev_create;
	dev_create.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	dev_create.pQueueCreateInfos       = q_create;
	dev_create.queueCreateInfoCount    = num_unique_queues;
	dev_create.pEnabledFeatures        = &dev_features;
	dev_create.ppEnabledExtensionNames = PHYSICAL_EXTENSIONS;
	dev_create.enabledExtensionCount   = 1;

#ifdef USE_VALIDATION_LAYERS
	dev_create.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	dev_create.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
	dev_create.enabledLayerCount = 0;
#endif

	auto res = vkCreateDevice(vri->physical_dev, &dev_create, nullptr, &vri->logical_dev);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "Could not create the Vulkan Logical device: %s\n", VkResult_str(res));
		return false;
	}

	vkGetDeviceQueue(vri->logical_dev, indices.graphics, 0, &vri->device_queues.graphics);
	vkGetDeviceQueue(vri->logical_dev, indices.present, 0, &vri->device_queues.present);
	// vkGetDeviceQueue(vri->logical_dev, indices.compure, 0, &vri->device_queues.compure);
	// vkGetDeviceQueue(vri->logical_dev, indices.transfer, 0, &vri->device_queues.transfer);
	// vkGetDeviceQueue(vri->logical_dev, indices.sparse_binding, 0, &vri->device_queues.sparse_binding);

	return true;
}

bool destroy_logical_device(VulkanRuntimeInfo *vri) {

	vkDestroyDevice(vri->logical_dev, nullptr);

	return true;
}

bool create_surface(VulkanRuntimeInfo *vri) {

	auto res = glfwCreateWindowSurface(vri->instance, vri->sys_window, nullptr, &vri->surface);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "Could not create the Vulkan Surface: %s\n", VkResult_str(res));
		return false;
	}

	return true;
}

bool destroy_surface(VulkanRuntimeInfo *vri) {

	vkDestroySurfaceKHR(vri->instance, vri->surface, nullptr);

	return true;
}

bool create_swapchain(VulkanRuntimeInfo *vri) {
	auto scd = get_swapchain_details(vri->physical_dev, vri->surface);

	auto format           = pick_swapchain_format(scd.formats, scd.formats_count);
	auto mode             = pick_swapchain_mode(scd.modes, scd.modes_count);
	vri->swapchain.extent = pick_swapchain_extent(&scd.capabilities, vri->sys_window);

	uint32_t image_count = scd.capabilities.minImageCount + 1;

	// maxImageCount == 0 means that there isn't a hard maximum
	if (scd.capabilities.maxImageCount > 0) {
		image_count = clamp(image_count, scd.capabilities.minImageCount, scd.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR sc_create = {0};
	sc_create.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_create.surface                  = vri->surface;
	sc_create.minImageCount            = image_count;
	sc_create.imageFormat              = format.format;
	sc_create.imageColorSpace          = format.colorSpace;
	sc_create.imageExtent              = vri->swapchain.extent;
	sc_create.imageArrayLayers         = 1;
	sc_create.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_create.preTransform             = scd.capabilities.currentTransform;
	sc_create.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_create.presentMode              = mode;
	sc_create.clipped                  = VK_TRUE;
	sc_create.oldSwapchain             = VK_NULL_HANDLE;

	QueueFamilyIndicies indices              = get_queue_families(vri->physical_dev, vri->surface);
	uint32_t            queueFamilyIndices[] = {indices.graphics, indices.present};

	if (indices.graphics != indices.present) {
		sc_create.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		sc_create.queueFamilyIndexCount = 2;
		sc_create.pQueueFamilyIndices   = queueFamilyIndices;
	} else {
		sc_create.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		sc_create.queueFamilyIndexCount = 0;       // Optional
		sc_create.pQueueFamilyIndices   = nullptr; // Optional
	}

	VkSwapchainKHR sc;

	auto res = vkCreateSwapchainKHR(vri->logical_dev, &sc_create, nullptr, &sc);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[Swapchain] Could no create the Swapchain: %s\n", VkResult_str(res));
		return false;
		;
	}

	vri->swapchain.swapchain = sc;
	vri->swapchain.format    = format.format;

	free(scd.formats);
	free(scd.modes);

	// retrieving images
	uint32_t count;
	vkGetSwapchainImagesKHR(vri->logical_dev, sc, &count, nullptr);
	// count > 0 cuz the creation of the swapchain was successfull
	vri->swapchain.buffers = malloc(sizeof(VkImage) * count);
	vkGetSwapchainImagesKHR(vri->logical_dev, sc, &count, vri->swapchain.buffers);

	return true;
}

bool destroy_swapchain(VulkanRuntimeInfo *vri) {

	free(vri->swapchain.buffers);

	vkDestroySwapchainKHR(vri->logical_dev, vri->swapchain.swapchain, nullptr);

	return true;
}

bool create_image_views(VulkanRuntimeInfo *vri) {

	vri->swapchain.views = malloc(sizeof(VkImageView) * vri->swapchain.buffers_count);

	for (size_t i = 0; i < vri->swapchain.buffers_count; ++i) {

		VkImageViewCreateInfo vw_create           = {0};
		vw_create.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vw_create.image                           = vri->swapchain.buffers[i];
		vw_create.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		vw_create.format                          = vri->swapchain.format;
		vw_create.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		vw_create.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		vw_create.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		vw_create.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		vw_create.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		vw_create.subresourceRange.baseMipLevel   = 0;
		vw_create.subresourceRange.levelCount     = 1;
		vw_create.subresourceRange.baseArrayLayer = 0;
		vw_create.subresourceRange.layerCount     = 1;

		auto res = vkCreateImageView(vri->logical_dev, &vw_create, nullptr, &vri->swapchain.views[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[Swapchain] Could not create image views: %s\n", VkResult_str(res));
			return false;
		}
	}

	return true;
}

bool destroy_image_views(VulkanRuntimeInfo *vri) {
	for (size_t i = 0; i < vri->swapchain.buffers_count; ++i) {
		vkDestroyImageView(vri->logical_dev, vri->swapchain.views[i], nullptr);
	}

	free(vri->swapchain.views);

	return true;
}

bool create_pipeline(VulkanRuntimeInfo *vri) {



}

bool destroy_pipeline(VulkanRuntimeInfo *vri) {

}
