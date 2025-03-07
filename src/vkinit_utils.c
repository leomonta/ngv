#include "vkinit_utils.h"

#include "logger.h"
#include "vulkan_initialization.h"

#include <errno.h>
#include <shaderc/shaderc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

QueueFamilyIndicies get_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {

	QueueFamilyIndicies res = {0};

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "Could not find any queue family\n");
		return res;
	}

	VkQueueFamilyProperties *queues = malloc(sizeof(VkQueueFamilyProperties) * count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues);

	for (uint32_t i = 0; i < count; ++i) {
		auto qf = queues[i];

		if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			res.graphics = i;
			set_bit(&res.available_families, GRAPHIC_QUEUE_INDEX);
		}
		if (qf.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			res.compute = i;
			set_bit(&res.available_families, COMPUTE_QUEUE_INDEX);
		}
		if (qf.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			res.transfer = i;
			set_bit(&res.available_families, TRANSFER_QUEUE_INDEX);
		}

		VkBool32 support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
		if (support) {
			res.present = i;
			set_bit(&res.available_families, PRESENT_QUEUE_INDEX);
		}
	}

	free(queues);
	return res;
}

bool has_required_extensions(VkPhysicalDevice device) {
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

	VkExtensionProperties *aval_exts = malloc(sizeof(VkExtensionProperties) * count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, aval_exts);

	bool res = false;

	for (size_t i = 0; i < count; ++i) {
		// uh oh, unsecured unbounded strcmp of constants string
		if (strcmp(aval_exts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0) {
			continue;
		}

		res = true;
	}

	free(aval_exts);

	return res;
}

/*
 * Honestly 99% of the there are going to be:
 * 1 dedicated GPU (desktop)
 * 2 integrated GPU (laptop)
 * 3 integrated + dedicated GPU (high-end laptop)
 *
 * So the selection should be much simpler
 * but this way I can stop the program if the device does not support features that are necessary to me
 */
VkPhysicalDevice pick_best_device(const VkPhysicalDevice *devs, const size_t count, VkSurfaceKHR surface) {

	auto     choice    = VK_NULL_HANDLE;
	unsigned score     = 0;
	unsigned old_score = 0;

	for (size_t i = 0; i < count; ++i) {
		score     = 0;
		auto qfam = get_queue_families(devs[i], surface);

		// ----
		// necessary stuff

		if (!at_bit(qfam.available_families, GRAPHIC_QUEUE_INDEX)) {
			continue;
		}
		if (!at_bit(qfam.available_families, PRESENT_QUEUE_INDEX)) {
			continue;
		}

		if (!has_required_extensions(devs[i])) {
			continue;
		}

		auto swd = get_swapchain_details(devs[i], surface);
		if (swd.formats_count == 0 || swd.modes_count == 0) {
			continue;
		}

		// TODO:
		// free the swapchain details (swd) since I don't need these anymore
		if (swd.modes != nullptr) {
			free(swd.modes);
		}
		if (swd.formats != nullptr) {
			free(swd.formats);
		}

		// ----
		// stuff that is nice to have

		VkPhysicalDeviceProperties dev_props;
		VkPhysicalDeviceFeatures   dev_feats;
		vkGetPhysicalDeviceProperties(devs[i], &dev_props);
		vkGetPhysicalDeviceFeatures(devs[i], &dev_feats);

		if (dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 100;
		}

		if (old_score < score) {
			old_score = score;
			choice    = devs[i];
		}
	}

	return choice;
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

SwapchainDetails get_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapchainDetails res = {0};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &res.capabilities);

	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

	if (count != 0) {
		res.formats = malloc(sizeof(VkSurfaceFormatKHR) * count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, res.formats);
		res.formats_count = count;
	}

	count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

	if (count != 0) {
		res.modes = malloc(sizeof(VkPresentModeKHR) * count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, res.modes);
		res.modes_count = count;
	}

	return res;
}

VkSurfaceFormatKHR pick_swapchain_format(const VkSurfaceFormatKHR *formats, const size_t count) {

	// attempt to choose a stadard 888 srgb
	for (size_t i = 0; i < count; ++i) {
		auto fmt = formats[i];
		if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return fmt;
		}
	}

	llog(LOG_WARNING, "[Swapchain] Could not choose the preferred format\n");

	// else whatever
	return formats[0];
}

VkPresentModeKHR pick_swapchain_mode(const VkPresentModeKHR *modes, const size_t count) {
	// attempt to choose MAILBOX
	for (size_t i = 0; i < count; ++i) {
		auto md = modes[i];
		if (md == VK_PRESENT_MODE_MAILBOX_KHR) {
			return md;
		}
	}

	llog(LOG_WARNING, "[Swapchain] Could not choose the preferred present mode (MAILBOX). Using the default (FIFO)\n");

	// else FIFO is guaranteed to exists
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D pick_swapchain_extent(const VkSurfaceCapabilitiesKHR *caps, GLFWwindow *win) {
	// if width or height is `0xffffffff` we have free reign on the swachain extent
	// else we just use the maximum
	if (caps->currentExtent.width != 0xffffffff) {
		return caps->maxImageExtent;
		llog(LOG_WARNING, "[Swapchain] Could not choose the preferred extent\n");
	} else {
		uint32_t width, height;
		glfwGetFramebufferSize(win, (int *)(&width), (int *)(&height));

		// the specification specifies that the `min` and `max` `ImageExtent`are not valid if the special value is present in `currentExtent`
		// https://vulkan.lunarg.com/doc/view/latest/windows/apispec.html#VkSurfaceCapabilitiesKHR

		return (VkExtent2D){width, height};
	}
}

bool compile_shader_file(const char *filename, const ShaderKind kind, ShaderInfo *result) {

	errno         = 0;
	FILE *sd_file = fopen(filename, "r");
	if (sd_file == NULL || errno != 0) {
		llog(LOG_ERROR, "[SHADER] Could not read the shader file '%s': %s\n", filename, strerror(errno));
		if (sd_file != NULL) {
			fclose(sd_file);
		}
		return false;
	}

	fseek(sd_file, 0, SEEK_END);
	auto sz = (unsigned long)(ftell(sd_file));

	char *code = malloc(sz);

	fread(code, 1, sz, sd_file);

	if (ferror(sd_file)) {
		llog(LOG_ERROR, "[SHADER] Could read from file '%s': %s\n", filename);
		fclose(sd_file);
		free(code);
		return false;
	}

	fclose(sd_file);

	auto res = compile_shader(code, sz, kind, result);

	free(code);

	return res;
}

bool compile_shader(const char *code, const size_t size, const ShaderKind kind, ShaderInfo *result) {

	shaderc_shader_kind _kind = shaderc_vertex_shader;

	switch (kind) {
	case VERTEX_SHADER:
		_kind = shaderc_vertex_shader;
		break;

	case TESSELATION_SHADER:
		_kind = shaderc_tess_evaluation_shader;
		break;

	case GEOMETRY_SHADER:
		_kind = shaderc_geometry_shader;
		break;

	case FRAGMENT_SHADER:
		_kind = shaderc_fragment_shader;
		break;

	case COMPUTE_SHADER:
		_kind = shaderc_compute_shader;
		break;
	}

	auto compiler = shaderc_compiler_initialize();
	auto c_res    = shaderc_compile_into_spv(compiler, code, size, _kind, "internal_compilation", "main", nullptr);

	auto c_status = shaderc_result_get_compilation_status(c_res);

	if (c_status != shaderc_compilation_status_success) {
		llog(LOG_ERROR, "[SHADER] Could not compile shader: %s\n", shaderc_result_get_error_message(c_res));
		return false;
	}

	result->kind   = kind;
	result->result = c_res;

	shaderc_compiler_release(compiler);

	return true;
}

bool release_shader(shaderc_compilation_result_t res) {
	shaderc_result_release(res);

	return true;
}
