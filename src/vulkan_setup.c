#include "vulkan_setup.h"

#include "config.h"
#include "logger.h"
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

bool create_setup_info(const VulkanSetupSettings *settings, VulkanSetupInfo *vsi, VulkanStaticInfo *static_info) {

	if (!create_logical_device(vsi, static_info)) {
		return false;
	}

	if (!create_swapchain(vsi, static_info)) {
		return false;
	}

	if (!create_swapchain_image_views(vsi)) {
		return false;
	}

	if (!create_framebuffers(vri)) {
		return false;
	}

	if (!create_depth_objects(vri)) {
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

bool create_logical_device(VulkanSetupInfo *vsi, VulkanStaticInfo *static_info) {

	if (get_queue_families(static_info->physical_dev, static_info->surface, &vsi->device_queues_indices)) {
		return false;
	}

	// if GRAPHIC_QUEUE_INDEX, TRANSFER_QUEUE_INDEX, or PRESENT_QUEUE_INDEX are not set
	if ((vsi->device_queues_indices.available_families & NEEDED_QUEUES) != NEEDED_QUEUES) {
		llog(LOG_ERROR, "[LOGICAL DEVICE] Could not satisfy the required queues necessary\n");
		return false;
	}

	uint32_t                needed_queues[NEEDED_QUEUES_COUNT] = {vsi->device_queues_indices.graphics, vsi->device_queues_indices.present, vsi->device_queues_indices.transfer};
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

	auto res = vkCreateDevice(static_info->physical_dev, &dev_create, nullptr, &vsi->logical_dev);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[LOGICAL DEVICE] Could not create the Vulkan Logical device: %s\n", VkResult_str(res));
		return false;
	}

	vkGetDeviceQueue(vsi->logical_dev, vsi->device_queues_indices.graphics, 0, &vsi->device_queues.graphics);
	vkGetDeviceQueue(vsi->logical_dev, vsi->device_queues_indices.present, 0, &vsi->device_queues.present);
	// vkGetDeviceQueue(vri->logical_dev, vri->device_queues_indices.compute, 0, &vri->device_queues.compure);
	vkGetDeviceQueue(vsi->logical_dev, vsi->device_queues_indices.transfer, 0, &vsi->device_queues.transfer);
	// vkGetDeviceQueue(vri->logical_dev, vri->device_queues_indices.sparse_binding, 0, &vri->device_queues.sparse_binding);

	llog(LOG_DEBUG, "[LOGICAL DEVICE] Logical device successfully created\n");

	return true;
}

bool destroy_logical_device(VulkanRuntimeInfo *vsi) {

	vkDestroyDevice(vsi->logical_dev, nullptr);

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

bool create_swapchain(VulkanSetupInfo *vsi, VulkanStaticInfo *static_info) {

	SwapchainDetails scd = get_swapchain_details(static_info->physical_dev, static_info->surface);

	auto format           = pick_swapchain_format(scd.formats, scd.formats_count);
	auto mode             = pick_swapchain_mode(scd.modes, scd.modes_count);
	vsi->swapchain.extent = pick_swapchain_extent(&scd.capabilities, static_info->system_window);

	uint32_t image_count = scd.capabilities.minImageCount + 1;

	// maxImageCount == 0 means that there isn't a hard maximum
	if (scd.capabilities.maxImageCount > 0) {
		image_count = clamp(image_count, scd.capabilities.minImageCount, scd.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR sc_create = {
	    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .surface               = static_info->surface,
	    .minImageCount         = image_count,
	    .imageFormat           = format.format,
	    .imageColorSpace       = format.colorSpace,
	    .imageExtent           = vsi->swapchain.extent,
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

	uint32_t queue_indices[] = {vsi->device_queues_indices.graphics, vsi->device_queues_indices.present};

	if (vsi->device_queues_indices.graphics != vsi->device_queues_indices.present) {
		sc_create.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		sc_create.queueFamilyIndexCount = 2;
		sc_create.pQueueFamilyIndices   = queue_indices;
	} else {
		sc_create.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		sc_create.queueFamilyIndexCount = 0;       // Optional
		sc_create.pQueueFamilyIndices   = nullptr; // Optional
	}

	VkSwapchainKHR sc;

	auto res = vkCreateSwapchainKHR(vsi->logical_dev, &sc_create, nullptr, &sc);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[SWAPCHAIN] Could not create the Swapchain: %s\n", VkResult_str(res));
		return false;
	}

	vsi->swapchain.swapchain = sc;
	vsi->swapchain.format    = format.format;

	free(scd.formats);
	free(scd.modes);

	// retrieving images
	vkGetSwapchainImagesKHR(vsi->logical_dev, sc, &vsi->swapchain.buffers_count, nullptr);
	// count > 0 cuz the creation of the swapchain was successfull
	// I can exploit the fact the when i recreate the swapchain `cleanup_swapchain` does not free the pointer, just need to ensure that the defualt value is nullptr
	vsi->swapchain.buffers = realloc(vsi->swapchain.buffers, sizeof(VkImage) * vsi->swapchain.buffers_count);
	TEST_MALLOC(vsi->swapchain.buffers);
	vkGetSwapchainImagesKHR(vsi->logical_dev, sc, &vsi->swapchain.buffers_count, vsi->swapchain.buffers);

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

	// destroy_depth_objects(vri);

	vkDestroySwapchainKHR(setup_info->logical_dev, setup_info->swapchain.swapchain, nullptr);

	return true;
}

uint32_t find_memory_type(const uint32_t type_filter, VkMemoryPropertyFlags properties, VulkanStaticInfo *static_info) {

	VkPhysicalDeviceMemoryProperties phy_props;
	vkGetPhysicalDeviceMemoryProperties(static_info->physical_dev, &phy_props);

	for (uint32_t i = 0; i < phy_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (phy_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	llog(LOG_FATAL, "[VMEM] Failed to find a suitable memory type\n");
	return (uint32_t)(-1);
}

bool create_image(VulkanRuntimeInfo *vri, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *image_mem) {

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

	auto res = vkCreateImage(vri->logical_dev, &img_info, nullptr, image);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[TEXTURE] Could not create texture: %s\n", VkResult_str(res));
	}

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(vri->logical_dev, *image, &mem_req);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize       = mem_req.size;
	alloc_info.memoryTypeIndex      = find_memory_type(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vri);

	res = vkAllocateMemory(vri->logical_dev, &alloc_info, nullptr, image_mem);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[TEXTURE] Could not allocate memory for the texture: %s\n", VkResult_str(res));
		return false;
	}

	vkBindImageMemory(vri->logical_dev, *image, *image_mem, 0);

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

	if (!create_framebuffers(vri)) {
		return false;
	}

	if (!create_depth_objects(vri)) {
		return false;
	}

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully re-created\n");

	return true;
}

bool destroy_swapchain(VulkanSetupInfo *setup_info) {

	free(setup_info->swapchain.buffers);
	setup_info->swapchain.buffers = nullptr;
	vri->swapchain.buffers_count  = 0;

	vkDestroySwapchainKHR(vri->logical_dev, vri->swapchain.swapchain, nullptr);

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully destroyed\n");

	return true;
}
