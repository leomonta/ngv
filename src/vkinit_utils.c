#include "vkinit_utils.h"

#include "config.h"
#include "logger.h"
#include "utils.h"
#include "vulkan_memory.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool check_validation_layer_support() {
#ifdef USE_VALIDATION_LAYERS

	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	VkLayerProperties *props = (VkLayerProperties *)(malloc(sizeof(VkLayerProperties) * count));
	TEST_MALLOC(props)

	vkEnumerateInstanceLayerProperties(&count, props);

	for (unsigned i = 0; i < VALIDATION_LAYERS_COUNT; ++i) {

		for (unsigned j = 0; j < count; ++j) {
			if (strcmp(VALIDATION_LAYERS[i], props[j].layerName) == 0) {
				llog(LOG_DEBUG, "[DEBUG] Validation layer found\n");
				free(props);
				return true;
			}
		}
	}

	free(props);

	return false;
#else
	return true;
#endif
}

const char *ShaderKind_str(const ShaderKind kind) {

	switch (kind) {

	case VERTEX_SHADER:
		return "VERTEX_SHADER";
	case TESSELATION_SHADER:
		return "TESSELATION_SHADER";
	case GEOMETRY_SHADER:
		return "GEOMETRY_SHADER";
	case FRAGMENT_SHADER:
		return "FRAGMENT_SHADER";
	case COMPUTE_SHADER:
		return "COMPUTE_SHADER";
	}

	return "UNKNOWN SHADER KIND";
}

const char *VkResult_str(const VkResult res) {

	// I would have liked to use a lookup array of sort but
	// 	1 I shouldn't depend on implementation details (e.g. the constants might be rally big numbers)
	// 	2 Some Macros have nagative values, so it is a nono
	// a switch is a fine alternative
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

bool create_buffer(VulkanSetupInfo *setup_info, VkPhysicalDevice physical_dev, const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory) {

	uint32_t           sharing_families[] = {setup_info->device_queues_indices.transfer, setup_info->device_queues_indices.graphics};
	VkBufferCreateInfo buff_create        = {};
	buff_create.sType                     = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_create.size                      = size;
	buff_create.usage                     = usage;
	buff_create.sharingMode               = VK_SHARING_MODE_CONCURRENT;
	buff_create.queueFamilyIndexCount     = 2;
	buff_create.pQueueFamilyIndices       = sharing_families;

	auto res = vkCreateBuffer(setup_info->logical_dev, &buff_create, nullptr, buffer);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[VMEM] Could not create vertex buffer: %s\n", VkResult_str(res));
		return false;
	}

	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(setup_info->logical_dev, *buffer, &reqs);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize       = reqs.size;
	alloc_info.memoryTypeIndex      = find_memory_type(reqs.memoryTypeBits, properties, physical_dev);

	if (alloc_info.memoryTypeIndex == (uint32_t)(-1)) {
		return false;
	}

	res = vkAllocateMemory(setup_info->logical_dev, &alloc_info, nullptr, buffer_memory);
	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[VMEM] Failed to allocate vertex buffer memory: %s\n", VkResult_str(res));
		return false;
	}

	res = vkBindBufferMemory(setup_info->logical_dev, *buffer, *buffer_memory, 0);
	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[VMEM] Failed to bind the created buffer (%p) to its memory: %s\n", buffer, VkResult_str(res));
		return false;
	}

	return true;
}

bool create_image(VkDevice logical_dev, VkPhysicalDevice physical_dev, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *image_mem) {

	VkImageCreateInfo img_info = {};
	img_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.imageType         = VK_IMAGE_TYPE_2D;
	img_info.extent.width      = (uint32_t)(width);
	img_info.extent.height     = (uint32_t)(height);
	img_info.extent.depth      = 1;
	img_info.mipLevels         = 1;
	img_info.arrayLayers       = 1;
	img_info.format            = format;
	img_info.tiling            = tiling;
	img_info.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	img_info.usage             = usage;
	img_info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
	img_info.samples           = VK_SAMPLE_COUNT_1_BIT;
	img_info.flags             = 0; // Optional

	auto res = vkCreateImage(logical_dev, &img_info, nullptr, image);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[TEXTURE] Could not create texture: %s\n", VkResult_str(res));
	}

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(logical_dev, *image, &mem_req);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize       = mem_req.size;
	alloc_info.memoryTypeIndex      = find_memory_type(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physical_dev);

	res = vkAllocateMemory(logical_dev, &alloc_info, nullptr, image_mem);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[TEXTURE] Could not allocate memory for the texture: %s\n", VkResult_str(res));
		return false;
	}

	vkBindImageMemory(logical_dev, *image, *image_mem, 0);

	return true;
}
