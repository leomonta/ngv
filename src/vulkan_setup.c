#include "vulkan_setup.h"

#include "config.h"
#include "logger.h"
#include "shader.h"
#include "vkinit_utils.h"

/*

typedef struct {
    VkDevice         logical_dev;
    VkRenderPass     renderpass;
    VkCommandPool    graphics_cmd_pool;
    VkCommandPool    transfer_cmd_pool;
    VkDescriptorPool descriptor_pool;
    VkCommandBuffer  transfer_cmd_buff;
    ShaderPipeline   pipeline;
    CreatedQueues    device_queues;
    QueuesIndicies   device_queues_indices;
    SwapchainInfo    swapchain;
} VulkanSetupInfo;
*/

#include <errno.h>
#include <string.h>

#define NEEDED_QUEUES       (GRAPHIC_QUEUE | PRESENT_QUEUE | TRANSFER_QUEUE)
#define NEEDED_QUEUES_COUNT 3

bool create_setup_info(const NGVRendererSettings *settings, VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info) {

	if (!create_logical_device(setup_info, static_info)) {
		return false;
	}

	if (!create_swapchain(setup_info, static_info)) {
		return false;
	}

	if (!create_swapchain_image_views(setup_info)) {
		return false;
	}

	if (!create_depth_objects(setup_info, static_info->physical_dev)) {
		return false;
	}

	if (!create_framebuffers(setup_info)) {
		return false;
	}

	if (!create_pipeline(settings, setup_info)) {
		return false;
	}

	if (!create_renderpass(setup_info)) {
		return false;
	}

	if (!create_descriptor_pool(setup_info)) {
		return false;
	}

	return true;
}

bool destroy_setup_info(VulkanSetupInfo *setup_info) {

	if (!destroy_descriptor_pool(setup_info)) {
		return false;
	}

	if (!destroy_renderpass(setup_info)) {
		return false;
	}

	if (!destroy_pipeline(setup_info)) {
		return false;
	}

	if (!destroy_swapchain(setup_info)) {
		return false;
	}

	if (!destroy_depth_objects(setup_info)) {
		return false;
	}

	if (!destroy_framebuffers(setup_info)) {
		return false;
	}

	if (!destroy_logical_device(setup_info->logical_dev)) {
		return false;
	}

	return true;
}

bool get_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, QueuesIndicies *indicies) {

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "[QUEUES] Could not find any queue family\n");
		return false;
	}

	VkQueueFamilyProperties *queues = malloc(sizeof(VkQueueFamilyProperties) * count);
	TEST_MALLOC(queues);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues);

	for (uint32_t i = 0; i < count; ++i) {
		auto qf = queues[i];

		if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT && at_bit(indicies->available_families, GRAPHIC_QUEUE) == false) {
			indicies->graphics = i;
			set_bit(&indicies->available_families, GRAPHIC_QUEUE);
		}

		if (qf.queueFlags & VK_QUEUE_COMPUTE_BIT && at_bit(indicies->available_families, COMPUTE_QUEUE) == false) {
			indicies->compute = i;
			set_bit(&indicies->available_families, COMPUTE_QUEUE);
		}

		// set this only if it's not also a graphic queue
		if (qf.queueFlags & VK_QUEUE_TRANSFER_BIT && !(qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) && at_bit(indicies->available_families, TRANSFER_QUEUE) == false) {
			indicies->transfer = i;
			set_bit(&indicies->available_families, TRANSFER_QUEUE);
		}

		VkBool32 support = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &support);
		if (support && at_bit(indicies->available_families, PRESENT_QUEUE) == false) {
			indicies->present = i;
			set_bit(&indicies->available_families, PRESENT_QUEUE);
		}
	}

	// any Queue family that supports graphics also supports transfers
	// so if a transfer queue was not found I use the graphic one
	if (at_bit(indicies->available_families, TRANSFER_QUEUE) == false) {
		indicies->transfer = indicies->graphics;
		set_bit(&indicies->available_families, TRANSFER_QUEUE);
	}

	free(queues);
	return true;
}

bool create_logical_device(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info) {

	if (get_queue_families(static_info->physical_dev, static_info->surface, &setup_info->device_queues_indices)) {
		return false;
	}

	// if GRAPHIC_QUEUE_INDEX, TRANSFER_QUEUE_INDEX, or PRESENT_QUEUE_INDEX are not set
	if ((setup_info->device_queues_indices.available_families & NEEDED_QUEUES) != NEEDED_QUEUES) {
		llog(LOG_ERROR, "[LOGICAL DEVICE] Could not satisfy the required queues necessary\n");
		return false;
	}

	uint32_t                needed_queues[NEEDED_QUEUES_COUNT] = {setup_info->device_queues_indices.graphics, setup_info->device_queues_indices.present, setup_info->device_queues_indices.transfer};
	VkDeviceQueueCreateInfo q_create[NEEDED_QUEUES_COUNT]      = {};

	// I need to ensure that if a queue family supports multiple functionalities
	// I only add it once to the logical device creation struct
	uint32_t num_unique_queues = NEEDED_QUEUES_COUNT;

	// check if there is a duplice index
	// if so replace it with the one at the end of the list and consider the array 1 element smaller
	// FIXME: this will bite me in the ass in the future, I've definitly made a mistake
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

		q_create[i] = (VkDeviceQueueCreateInfo){
		    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    .flags            = 0,
		    .queueFamilyIndex = needed_queues[i],
		    .queueCount       = 1,
		    .pQueuePriorities = &q_priority,
		    .pNext            = nullptr,
		};
	}

	VkPhysicalDeviceFeatures dev_features = {};
	dev_features.samplerAnisotropy        = VK_TRUE;

	VkDeviceCreateInfo dev_create = {
	    .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .flags                = 0,
	    .queueCreateInfoCount = num_unique_queues,
	    .pQueueCreateInfos    = q_create,

#ifdef USE_VALIDATION_LAYERS
	    dev_create.enabledLayerCount   = VALIDATION_LAYERS_COUNT,
	    dev_create.ppEnabledLayerNames = VALIDATION_LAYERS,
#else
	    .enabledLayerCount   = 0,
	    .ppEnabledLayerNames = nullptr,
#endif
	    .enabledExtensionCount   = PHYSICAL_EXTENSIONS_COUNT,
	    .ppEnabledExtensionNames = PHYSICAL_EXTENSIONS,
	    .pEnabledFeatures        = &dev_features,
	    .pNext                   = nullptr,
	};

	auto res = vkCreateDevice(static_info->physical_dev, &dev_create, nullptr, &setup_info->logical_dev);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[LOGICAL DEVICE] Could not create the Vulkan Logical device: %s\n", VkResult_str(res));
		return false;
	}

	vkGetDeviceQueue(setup_info->logical_dev, setup_info->device_queues_indices.graphics, 0, &setup_info->device_queues.graphics);
	vkGetDeviceQueue(setup_info->logical_dev, setup_info->device_queues_indices.present, 0, &setup_info->device_queues.present);
	// vkGetDeviceQueue(setup_info->logical_dev, setup_info->device_queues_indices.compute, 0, &setup_info->device_queues.compure);
	vkGetDeviceQueue(setup_info->logical_dev, setup_info->device_queues_indices.transfer, 0, &setup_info->device_queues.transfer);
	// vkGetDeviceQueue(setup_info->logical_dev, setup_info->device_queues_indices.sparse_binding, 0, &setup_info->device_queues.sparse_binding);

	llog(LOG_DEBUG, "[LOGICAL DEVICE] Logical device successfully created\n");

	return true;
}

bool destroy_logical_device(VkDevice logical_dev) {

	vkDestroyDevice(logical_dev, nullptr);

	llog(LOG_DEBUG, "[LOGICAL DEVICE] Logical device successfully destroyed\n");

	return true;
}

VkSurfaceFormatKHR pick_swapchain_format(const VkSurfaceFormatKHR *formats, const size_t count) {

	// attempt to choose a stadard 888 srgb
	for (size_t i = 0; i < count; ++i) {
		auto fmt = formats[i];
		if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return fmt;
		}
	}

	llog(LOG_WARNING, "[SWAPCHAIN] Could not choose the preferred format\n");

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

	llog(LOG_WARNING, "[SWAPCHAIN] Could not choose the preferred present mode (MAILBOX). Using the default (FIFO)\n");

	// else FIFO is guaranteed to exists
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D pick_swapchain_extent(const VkSurfaceCapabilitiesKHR *caps, GLFWwindow *win) {

	// if width or height is `0xffffffff` we have free reign on the swachain extent
	// else we just use the maximum
	if (caps->currentExtent.width != 0xffffffff) {
		llog(LOG_WARNING, "[SWAPCHAIN] Could not choose the preferred extent\n");
		return caps->maxImageExtent;
	} else {
		uint32_t width, height;
		glfwGetFramebufferSize(win, (int *)(&width), (int *)(&height));

		// the specification specifies that the `min` and `max` `ImageExtent`are not valid if the special value is present in `currentExtent`
		// https://vulkan.lunarg.com/doc/view/latest/windows/apispec.html#VkSurfaceCapabilitiesKHR

		return (VkExtent2D){width, height};
	}
}

SwapchainDetails get_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface) {

	SwapchainDetails res = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &res.capabilities);

	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

	if (count <= 0) {
		llog(LOG_ERROR, "[SWAPCHAIN] Could not find any available surface format\n");
		return res;
	}

	if (count != 0) {
		res.formats = malloc(sizeof(VkSurfaceFormatKHR) * count);
		TEST_MALLOC_RET(res.formats, res)
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, res.formats);
		res.formats_count = count;
	}

	count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

	if (count != 0) {
		res.modes = malloc(sizeof(VkPresentModeKHR) * count);
		TEST_MALLOC_RET(res.formats, res)
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, res.modes);
		res.modes_count = count;
	}

	return res;
}

bool create_swapchain(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info) {

	SwapchainDetails scd = get_swapchain_details(static_info->physical_dev, static_info->surface);

	auto format                  = pick_swapchain_format(scd.formats, scd.formats_count);
	auto mode                    = pick_swapchain_mode(scd.modes, scd.modes_count);
	setup_info->swapchain.extent = pick_swapchain_extent(&scd.capabilities, static_info->system_window);

	uint32_t image_count = 3;

	// maxImageCount == 0 means that there isn't a hard maximum
	if (scd.capabilities.maxImageCount > 0) {
		image_count = clamp(image_count, scd.capabilities.minImageCount, scd.capabilities.maxImageCount);
	} else {
		image_count = clamp(image_count, scd.capabilities.minImageCount, image_count);
	}

	VkSwapchainCreateInfoKHR sc_create = {
	    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .surface               = static_info->surface,
	    .minImageCount         = image_count,
	    .imageFormat           = format.format,
	    .imageColorSpace       = format.colorSpace,
	    .imageExtent           = setup_info->swapchain.extent,
	    .imageArrayLayers      = 1,
	    .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    .imageSharingMode      = 0,       // later
	    .queueFamilyIndexCount = 0,       // later
	    .pQueueFamilyIndices   = nullptr, // later
	    .preTransform          = scd.capabilities.currentTransform,
	    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // may be in settings
	    .presentMode           = mode,
	    .clipped               = VK_TRUE,
	    .oldSwapchain          = VK_NULL_HANDLE,
	    .flags                 = 0,
	    .pNext                 = nullptr,
	};

	uint32_t queue_indices[] = {setup_info->device_queues_indices.graphics, setup_info->device_queues_indices.present};

	if (setup_info->device_queues_indices.graphics != setup_info->device_queues_indices.present) {
		sc_create.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		sc_create.queueFamilyIndexCount = 2;
		sc_create.pQueueFamilyIndices   = queue_indices;
	} else {
		sc_create.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		sc_create.queueFamilyIndexCount = 0;       // Optional
		sc_create.pQueueFamilyIndices   = nullptr; // Optional
	}

	auto res = vkCreateSwapchainKHR(setup_info->logical_dev, &sc_create, nullptr, &setup_info->swapchain.swapchain);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[SWAPCHAIN] Could not create the Swapchain: %s\n", VkResult_str(res));
		return false;
	}

	setup_info->swapchain.format    = format.format;

	free(scd.formats);
	free(scd.modes);

	// retrieving images
	vkGetSwapchainImagesKHR(setup_info->logical_dev, setup_info->swapchain.swapchain, &setup_info->swapchain.buffers_count, nullptr);
	// count > 0 cuz the creation of the swapchain was successfull
	// I can exploit the fact the when i recreate the swapchain `cleanup_swapchain` does not free the pointer, just need to ensure that the defualt value is nullptr
	setup_info->swapchain.buffers = realloc(setup_info->swapchain.buffers, sizeof(VkImage) * setup_info->swapchain.buffers_count);
	TEST_MALLOC(setup_info->swapchain.buffers);
	vkGetSwapchainImagesKHR(setup_info->logical_dev, setup_info->swapchain.swapchain, &setup_info->swapchain.buffers_count, setup_info->swapchain.buffers);

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully created\n");

	return true;
}

bool create_swapchain_image_views(VulkanSetupInfo *setup_info) {

	setup_info->swapchain.views = realloc(setup_info->swapchain.views, sizeof(VkImageView) * setup_info->swapchain.buffers_count);
	TEST_MALLOC(setup_info->swapchain.views);

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {

		VkImageViewCreateInfo view_create = {
		    .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		    .image                           = setup_info->swapchain.buffers[i],
		    .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
		    .format                          = setup_info->swapchain.format,
		    .components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
		    .components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
		    .components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
		    .components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
		    .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
		    .subresourceRange.baseMipLevel   = 0,
		    .subresourceRange.levelCount     = 1,
		    .subresourceRange.baseArrayLayer = 0,
		    .subresourceRange.layerCount     = 1,
		    .pNext                           = nullptr,
		    .flags                           = 0,
		};

		auto res = vkCreateImageView(setup_info->logical_dev, &view_create, nullptr, &setup_info->swapchain.views[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SWAPCHAIN] Could not create image views: %s\n", VkResult_str(res));
			free(setup_info->swapchain.views);
			return false;
		}
	}

	llog(LOG_DEBUG, "[SWAPCHAIN] Image views successfully created\n");

	return true;
}

bool cleanup_swapchain(VulkanSetupInfo *setup_info) {

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		vkDestroyFramebuffer(setup_info->logical_dev, setup_info->swapchain.framebuffers[i], nullptr);
	}

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		vkDestroyImageView(setup_info->logical_dev, setup_info->swapchain.views[i], nullptr);
	}

	// destroy_depth_objects(setup_info);

	vkDestroySwapchainKHR(setup_info->logical_dev, setup_info->swapchain.swapchain, nullptr);

	return true;
}

bool re_create_swapchain(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info) {

	int width = 0, height = 0;

	glfwGetFramebufferSize(static_info->system_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(static_info->system_window, &width, &height);
		llog(LOG_DEBUG, "[DRAWING] minized...\n");
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(setup_info->logical_dev);

	cleanup_swapchain(setup_info);

	if (!create_swapchain(setup_info, static_info)) {
		return false;
	}

	if (!create_swapchain_image_views(setup_info)) {
		return false;
	}

	if (!create_framebuffers(setup_info)) {
		return false;
	}

	if (!create_depth_objects(setup_info, static_info->physical_dev)) {
		return false;
	}

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully re-created\n");

	return true;
}

bool destroy_swapchain(VulkanSetupInfo *setup_info) {

	free(setup_info->swapchain.buffers);
	setup_info->swapchain.buffers       = nullptr;
	setup_info->swapchain.buffers_count = 0;

	vkDestroySwapchainKHR(setup_info->logical_dev, setup_info->swapchain.swapchain, nullptr);

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully destroyed\n");

	return true;
}

bool create_depth_objects(VulkanSetupInfo *setup_info, VkPhysicalDevice physical_dev) {

	create_image(
	    setup_info->logical_dev,
	    physical_dev,
	    setup_info->swapchain.extent.width,
	    setup_info->swapchain.extent.height,
	    VK_FORMAT_D32_SFLOAT_S8_UINT,
	    VK_IMAGE_TILING_OPTIMAL,
	    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	    &setup_info->depth_objects.image,
	    &setup_info->depth_objects.memory);

	VkImageViewCreateInfo vw_create = {
	    .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    .image                           = setup_info->depth_objects.image,
	    .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
	    .format                          = VK_FORMAT_D32_SFLOAT_S8_UINT,
	    .components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
	    .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
	    .subresourceRange.baseMipLevel   = 0,
	    .subresourceRange.levelCount     = 1,
	    .subresourceRange.baseArrayLayer = 0,
	    .subresourceRange.layerCount     = 1,
	    .flags                           = 0,
	    .pNext                           = nullptr,
	};

	auto res = vkCreateImageView(setup_info->logical_dev, &vw_create, nullptr, &setup_info->depth_objects.view);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DEPTH BUFFER] Could not create image views: %s\n", VkResult_str(res));
		return false;
	}

	// transition_image_layout(setup_info, setup_info->depth_objects.image, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	llog(LOG_DEBUG, "[DEPTH BUFFER] Depth buffer objects successfully created.\n");

	return true;
}

bool destroy_depth_objects(VulkanSetupInfo *setup_info) {

	vkDestroyImage(setup_info->logical_dev, setup_info->depth_objects.image, nullptr);
	vkFreeMemory(setup_info->logical_dev, setup_info->depth_objects.memory, nullptr);
	vkDestroyImageView(setup_info->logical_dev, setup_info->depth_objects.view, nullptr);

	llog(LOG_DEBUG, "[DEPTH BUFFER] Depth buffer objects successfully destroyed.\n");

	return true;
}

bool create_framebuffers(VulkanSetupInfo *setup_info) {

	setup_info->swapchain.framebuffers = _realloc(setup_info->swapchain.framebuffers, sizeof(VkFramebuffer) * setup_info->swapchain.buffers_count);
	TEST_MALLOC(setup_info->swapchain.framebuffers)

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		VkImageView attachments[2] = {
		    setup_info->swapchain.views[i],
		    setup_info->depth_objects.view};

		VkFramebufferCreateInfo fb_create = {};
		fb_create.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create.renderPass              = setup_info->renderpass;
		fb_create.attachmentCount         = 2;
		fb_create.pAttachments            = attachments;
		fb_create.width                   = setup_info->swapchain.extent.width;
		fb_create.height                  = setup_info->swapchain.extent.height;
		fb_create.layers                  = 1;

		auto res = vkCreateFramebuffer(setup_info->logical_dev, &fb_create, nullptr, &setup_info->swapchain.framebuffers[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[FRAMBUFFER] Could not create a frambuffer: %s\n", VkResult_str(res));
			free(setup_info->swapchain.framebuffers);
			return false;
		}
	}

	llog(LOG_DEBUG, "[FRAMBUFFER] frambuffers successfully created\n");

	return true;
}

bool destroy_framebuffers(VulkanSetupInfo *setup_info) {

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		vkDestroyFramebuffer(setup_info->logical_dev, setup_info->swapchain.framebuffers[i], nullptr);
	}
	free(setup_info->swapchain.framebuffers);
	setup_info->swapchain.framebuffers = nullptr;

	llog(LOG_DEBUG, "[FRAMBUFFER] framebuffers successfully destroyed\n");
	return true;
}

VkPolygonMode ngv_to_vk_geometry_drawn(GeometryDraw gd) {
	switch (gd) {
		case GEOMETRY_FILL:
			return VK_POLYGON_MODE_FILL;
		case GEOMETRY_WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case GEOMETRY_POINT:
			return VK_POLYGON_MODE_POINT;
	}

}

VkPrimitiveTopology ngv_to_vk_topology(Topology tp) {
	switch (tp) {
		case TOPOLOGY_TRIANGLE:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;

		case TOPOLOGY_POINT:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
	
	}

}

bool create_pipeline(const NGVRendererSettings *settings, VulkanSetupInfo *setup_info) {
	shaderc_compilation_result_t vert_res;
	VkShaderModule               vert_module = {};

	create_shader_module("../shaders/main.vert", VERTEX_SHADER, setup_info->logical_dev, &vert_module, &vert_res);

	shaderc_compilation_result_t frag_res;
	VkShaderModule               frag_module = {};

	create_shader_module("../shaders/main.frag", FRAGMENT_SHADER, setup_info->logical_dev, &frag_module, &frag_res);

	VkPipelineShaderStageCreateInfo vert_stage_create = {
	    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .stage               = VK_SHADER_STAGE_VERTEX_BIT,
	    .module              = vert_module,
	    .pName               = "main",
	    .pSpecializationInfo = nullptr,
	    .flags               = 0,
	    .pNext               = nullptr,
	};

	VkPipelineShaderStageCreateInfo frag_stage_create = {
	    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
	    .module              = frag_module,
	    .pName               = "main",
	    .pSpecializationInfo = nullptr,
	    .flags               = 0,
	    .pNext               = nullptr,
	};

	VkPipelineShaderStageCreateInfo sh_stages[] = {vert_stage_create, frag_stage_create};

	VkPipelineDynamicStateCreateInfo dn_create = {
	    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .dynamicStateCount = PIPELINE_DYNAMIC_STATE_COUNT,
	    .pDynamicStates    = PIPELINE_DYNAMIC_STATE,
	    .flags             = 0,
	    .pNext             = nullptr,
	};

	// TODO: set the correct layout
	// maybe ask for some kind of struct to base the layout to
	VkPipelineVertexInputStateCreateInfo vl_create = {
	    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	    .vertexBindingDescriptionCount   = 1,
	    .pVertexBindingDescriptions      = &Vertex_layout,
	    .vertexAttributeDescriptionCount = Vertex_attributes_num,
	    .pVertexAttributeDescriptions    = Vertex_attribs,
	    .flags                           = 0,
	    .pNext                           = nullptr,
	};

	VkPipelineInputAssemblyStateCreateInfo ia_create = {
	    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology               = ngv_to_vk_topology(settings->topology),
	    .primitiveRestartEnable = VK_FALSE,
	    .flags                  = 0,
	    .pNext                  = nullptr,
	};

	VkPipelineViewportStateCreateInfo vp_create = {
	    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .scissorCount  = 1,
	    .pScissors     = nullptr,
	    .pViewports    = nullptr,
	    .flags         = 0,
	    .pNext         = nullptr,
	};

	VkPipelineRasterizationStateCreateInfo rt_create = {
	    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    .depthClampEnable        = VK_FALSE,
	    .rasterizerDiscardEnable = VK_FALSE,
	    .polygonMode             = ngv_to_vk_geometry_drawn(settings->geometry_drawn),
	    .lineWidth               = 1.0f,
	    .cullMode                = VK_CULL_MODE_NONE,
	    // rt_create.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
	    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
	    .depthBiasEnable         = VK_FALSE,
	    .depthBiasClamp          = 0,
	    .depthBiasConstantFactor = 0,
	    .depthBiasSlopeFactor    = 0,
	    .flags                   = 0,
	    .pNext                   = nullptr,
	};

	// TODO: This should probably be enabled
	VkPipelineMultisampleStateCreateInfo multisampling_create = {
	    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .sampleShadingEnable   = VK_FALSE,
	    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
	    .minSampleShading      = 1.0f,
	    .pSampleMask           = nullptr,
	    .alphaToCoverageEnable = VK_FALSE,
	    .alphaToOneEnable      = VK_FALSE,
	    .flags                 = 0,
	    .pNext                 = nullptr,
	};

	VkPipelineColorBlendAttachmentState colorblend_attach = {
	    .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	    .blendEnable         = VK_TRUE,
	    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
	    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	    .colorBlendOp        = VK_BLEND_OP_ADD,
	    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
	    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	    .alphaBlendOp        = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo colorblend_create = {
	    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	    .logicOpEnable   = VK_FALSE,
	    .attachmentCount = 1,
	    .pAttachments    = &colorblend_attach,
	    .logicOp         = 0,
	    .blendConstants  = {0, 0, 0, 0},
	    .flags           = 0,
	    .pNext           = nullptr,
	};

	VkDescriptorSetLayoutBinding mvp_binding = {
	    .binding            = 0,
	    .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	    .descriptorCount    = 1,
	    .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
	    .pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding sampl_binding = {
	    .binding            = 1,
	    .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	    .descriptorCount    = 1,
	    .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
	    .pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding bindings[2] = {mvp_binding, sampl_binding};

	VkDescriptorSetLayoutCreateInfo layout_info = {
	    .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
	    .bindingCount = 2,
	    .pBindings    = bindings,
	    .flags        = 0,
	    .pNext        = nullptr,
	};

	auto res = vkCreateDescriptorSetLayout(setup_info->logical_dev, &layout_info, nullptr, &setup_info->pipeline.descriptor_set_layout);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[PIPLINE] Failed to create descriptor set layout!: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[PIPELINE] Uniform descroptor set layout successfully created\n");

	VkPipelineLayoutCreateInfo pipeline_layout = {
	    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .setLayoutCount         = 1,
	    .pSetLayouts            = &setup_info->pipeline.descriptor_set_layout,
	    .pushConstantRangeCount = 0,
	    .pPushConstantRanges    = nullptr,
	    .flags                  = 0,
	    .pNext                  = nullptr,
	};

	res = vkCreatePipelineLayout(setup_info->logical_dev, &pipeline_layout, nullptr, &setup_info->pipeline.layout);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[PIPELINE] Could not create the pipeline layout: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[PIPELINE] Pipeline layout successfully crated\n");

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create = {
	    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable       = VK_TRUE,
	    .depthWriteEnable      = VK_TRUE,
	    .depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL,
	    .depthBoundsTestEnable = VK_FALSE,
	    .minDepthBounds        = 0.0f,
	    .maxDepthBounds        = 1.0f,
	    .stencilTestEnable     = VK_FALSE,
	    .front                 = {0},
	    .back                  = {0},
	    .flags                 = 0,
	    .pNext                 = nullptr,
	};

	VkGraphicsPipelineCreateInfo pipeline_create = {
	    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    .stageCount          = 2,
	    .pStages             = sh_stages,
	    .pVertexInputState   = &vl_create,
	    .pInputAssemblyState = &ia_create,
	    .pViewportState      = &vp_create,
	    .pRasterizationState = &rt_create,
	    .pMultisampleState   = &multisampling_create,
	    .pDepthStencilState  = &depth_stencil_create,
	    .pColorBlendState    = &colorblend_create,
	    .pDynamicState       = &dn_create,
	    .layout              = setup_info->pipeline.layout,
	    .renderPass          = setup_info->renderpass,
	    .subpass             = 0,
	    .flags               = 0,
	    .pNext               = nullptr,
	};

	res = vkCreateGraphicsPipelines(setup_info->logical_dev, VK_NULL_HANDLE, 1, &pipeline_create, nullptr, &setup_info->pipeline.object);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[PIPELINE] Could not create the pipeline layout: %s\n", VkResult_str(res));
		return false;
	}

	vkDestroyShaderModule(setup_info->logical_dev, vert_module, nullptr);
	release_shader(vert_res);
	vkDestroyShaderModule(setup_info->logical_dev, frag_module, nullptr);
	release_shader(frag_res);

	llog(LOG_DEBUG, "[PIPELINE] Graphics pipeline successfully crated\n");

	return true;
}

bool destroy_pipeline(VulkanSetupInfo *setup_info) {
	vkDestroyPipeline(setup_info->logical_dev, setup_info->pipeline.object, nullptr);
	vkDestroyPipelineLayout(setup_info->logical_dev, setup_info->pipeline.layout, nullptr);
	vkDestroyDescriptorSetLayout(setup_info->logical_dev, setup_info->pipeline.descriptor_set_layout, nullptr);

	llog(LOG_DEBUG, "[PIPELINE] Graphics pipeline successfully destroyed\n");

	return true;
}

bool create_command_pool(VulkanSetupInfo *setup_info) {

	VkCommandPoolCreateInfo g_pool_crate = {};
	g_pool_crate.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	g_pool_crate.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	g_pool_crate.queueFamilyIndex        = setup_info->device_queues_indices.graphics;

	auto res = vkCreateCommandPool(setup_info->logical_dev, &g_pool_crate, nullptr, &setup_info->graphics_cmd_pool);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND POOL] Could not create create the graphics command pool: %s\n", VkResult_str(res));
		return false;
	}
	llog(LOG_DEBUG, "[COMMAND POOL] Graphics command pool successfully created\n");

	VkCommandPoolCreateInfo t_pool_crate = {};
	t_pool_crate.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	t_pool_crate.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	t_pool_crate.queueFamilyIndex        = setup_info->device_queues_indices.transfer;

	res = vkCreateCommandPool(setup_info->logical_dev, &t_pool_crate, nullptr, &setup_info->transfer_cmd_pool);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND POOL] Could not create create the transfer command pool: %s\n", VkResult_str(res));
		return false;
	}
	llog(LOG_DEBUG, "[COMMAND POOL] Transfer command pool successfully created\n");

	return true;
}

bool destroy_command_pool(VulkanSetupInfo *setup_info) {
	vkDestroyCommandPool(setup_info->logical_dev, setup_info->graphics_cmd_pool, nullptr);
	vkDestroyCommandPool(setup_info->logical_dev, setup_info->transfer_cmd_pool, nullptr);

	llog(LOG_DEBUG, "[COMMAND POOL] Command pools successfully destroyed\n");

	return true;
}

bool create_renderpass(VulkanSetupInfo *setup_info) {
	VkAttachmentDescription attachments[2] = {};
	// color
	attachments[0].format                  = setup_info->swapchain.format;
	attachments[0].samples                 = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// depth
	attachments[1].format                  = VK_FORMAT_D32_SFLOAT_S8_UINT;
	attachments[1].samples                 = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_ref = {};
	color_ref.attachment            = 0;
	color_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_ref = {};
	depth_ref.attachment            = 1;
	depth_ref.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass    = {};
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount    = 1;
	subpass.pColorAttachments       = &color_ref;
	subpass.pDepthStencilAttachment = &depth_ref;

	VkSubpassDependency subpass_deps = {};
	subpass_deps.srcSubpass          = VK_SUBPASS_EXTERNAL;
	subpass_deps.dstSubpass          = 0;
	subpass_deps.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpass_deps.srcAccessMask       = VK_ACCESS_NONE;
	subpass_deps.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpass_deps.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderpass_create = {};
	renderpass_create.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_create.attachmentCount        = 2;
	renderpass_create.pAttachments           = attachments;
	renderpass_create.subpassCount           = 1;
	renderpass_create.pSubpasses             = &subpass;
	renderpass_create.dependencyCount        = 1;
	renderpass_create.pDependencies          = &subpass_deps;

	auto res = vkCreateRenderPass(setup_info->logical_dev, &renderpass_create, nullptr, &setup_info->renderpass);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[RENDERPASS] Could not create the render pass: %s\n", VkResult_str(res));
	}

	llog(LOG_DEBUG, "[RENDERPASS] Renderpass successfully created\n");
	return true;
}

bool destroy_renderpass(VulkanSetupInfo *setup_info) {

	vkDestroyRenderPass(setup_info->logical_dev, setup_info->renderpass, nullptr);

	llog(LOG_DEBUG, "[RENDERPASS] Renderpass successfully destroyed\n");

	return true;
}


bool create_descriptor_pool(VulkanSetupInfo *setup_info) {
	VkDescriptorPoolSize pool_sz[2] = {};
	pool_sz[0]                      = (VkDescriptorPoolSize){};
	pool_sz[0].type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sz[0].descriptorCount      = MAX_CONCURRENT_FRAMES;
	pool_sz[1]                      = (VkDescriptorPoolSize){};
	pool_sz[1].type                 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sz[1].descriptorCount      = MAX_CONCURRENT_FRAMES;

	VkDescriptorPoolCreateInfo dp_info = {};
	dp_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dp_info.poolSizeCount              = 2;
	dp_info.pPoolSizes                 = pool_sz;
	dp_info.maxSets                    = MAX_CONCURRENT_FRAMES;

	auto res = vkCreateDescriptorPool(setup_info->logical_dev, &dp_info, nullptr, &setup_info->descriptor_pool);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DESCRIPTOR POOL] Could not create the descriptor pool: %s\n", VkResult_str(res));
	}
	llog(LOG_DEBUG, "[DESCRIPTOR POOL] Descriptor pool successfully created\n");

	return true;
}

bool destroy_descriptor_pool(VulkanSetupInfo *setup_info) {
	vkDestroyDescriptorPool(setup_info->logical_dev, setup_info->descriptor_pool, nullptr);

	llog(LOG_DEBUG, "[DESCRIPTOR POOL] Descriptor pool successfully destroyed\n");
	return true;
}

bool create_command_buffer(VulkanSetupInfo *setup_info) {

	VkCommandBufferAllocateInfo t_cmd_crate = {};
	t_cmd_crate.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	t_cmd_crate.commandPool                 = setup_info->transfer_cmd_pool;
	t_cmd_crate.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	t_cmd_crate.commandBufferCount          = 1;

	auto res = vkAllocateCommandBuffers(setup_info->logical_dev, &t_cmd_crate, &setup_info->transfer_cmd_buff);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND BUFFER] Could not create the transfer command buffer: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[COMMAND BUFFER] Transfer command buffer successfully created\n");

	return true;
}
