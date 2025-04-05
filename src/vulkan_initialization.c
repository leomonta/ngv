#include "vulkan_initialization.h"

#include "config.h"
#include "logger.h"
#include "utils.h"
#include "vkinit_utils.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

#ifdef RAW_PRINTS
#	include <stdio.h>
#endif

const char              *PHYSICAL_EXTENSIONS[]        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr unsigned       PHYSICAL_EXTENSIONS_COUNT    = sizeof(PHYSICAL_EXTENSIONS) / sizeof(PHYSICAL_EXTENSIONS[0]);
constexpr VkDynamicState PIPELINE_DYNAMIC_STATE[]     = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}; // Do i really need these as dynamic state?
constexpr unsigned       PIPELINE_DYNAMIC_STATE_COUNT = sizeof(PIPELINE_DYNAMIC_STATE) / sizeof(PIPELINE_DYNAMIC_STATE[0]);

bool create_instance(VulkanRuntimeInfo *vri) {

	// Application information, fairly trivial / uninmportant

	VkApplicationInfo app_create = {0};

	app_create.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_create.pApplicationName   = "Neon Genesis Vulkan";
	app_create.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_create.pEngineName        = "None";
	app_create.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
	app_create.apiVersion         = VK_API_VERSION_1_4;

	// what we need to create with vkCreateInstance
	VkInstanceCreateInfo createInfo = {0};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &app_create;

	// how many instance we can use
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

#ifdef USE_VALIDATION_LAYERS
	createInfo.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
	createInfo.enabledLayerCount = 0;
#endif

	auto exts = get_required_extensions(&createInfo.enabledExtensionCount);
	if (exts == nullptr) {
		return false;
	}
	createInfo.ppEnabledExtensionNames = exts;

	auto result = vkCreateInstance(&createInfo, nullptr, &vri->instance);

	free(exts);

	if (result != VK_SUCCESS) {
		llog(LOG_FATAL, "[INSTANCE] Could not create vulkan instance: %s\n", VkResult_str(result));
		return false;
	}

	llog(LOG_DEBUG, "[INSTANCE] Vulkan instance successfully created\n");
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
		llog(LOG_ERROR, "[DEBUG] Could not get the debug callback creation function\n");
		return false;
	}
	auto res = func(vri->instance, &db_create, nullptr, &vri->debug_logger);

	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[DEBUG] Could not create the debugger callback: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[DEBUG] Debug callback successfully attached\n");

	return true;
}

bool detach_logger_callback(VulkanRuntimeInfo *vri) {

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vri->instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func == nullptr) {
		llog(LOG_ERROR, "[DEBUG] Could not get the debug destruction function\n");
		return false;
	}

	func(vri->instance, vri->debug_logger, nullptr);

	llog(LOG_DEBUG, "[DEBUG] Debug callback successfully detached\n");

	return true;
}

bool pick_physical_device(VulkanRuntimeInfo *vri) {

	uint32_t count = 0;
	vkEnumeratePhysicalDevices(vri->instance, &count, nullptr);

	llog(LOG_DEBUG, "[PHYSICAL DEVICE] count = %d\n", count);

	VkPhysicalDevice *devs = malloc(count * sizeof(VkPhysicalDevice));
	TEST_MALLOC(devs)
	vkEnumeratePhysicalDevices(vri->instance, &count, devs);

	// auto chosen_dev = filter_suitable_devices(devs, count, vri->surface);
	auto chosen_dev = get_chosen_device(devs, count); // devs[VULKAN_CHOSEN_PHYSICAL_DEVICE_INDEX];

	if (chosen_dev == VK_NULL_HANDLE) {
		llog(LOG_FATAL, "[PHYSICAL DEVICE] Could not find a suitable physical device\n");
		free(devs);
		return false;
	}
	vri->physical_dev = chosen_dev;

	free(devs);

	llog(LOG_DEBUG, "[PHYSICAL DEVICE] Physical device successfully created\n");

	return true;
}

bool create_logical_device(VulkanRuntimeInfo *vri) {

	auto indices = get_queue_families(vri->physical_dev, vri->surface);

	// if both GRAPHIC_QUEUE_INDEX and PRESENT_QUEUE_INDEX are set
	if ((indices.available_families & (GRAPHIC_QUEUE_INDEX | PRESENT_QUEUE_INDEX)) != (GRAPHIC_QUEUE_INDEX | PRESENT_QUEUE_INDEX)) {
		llog(LOG_ERROR, "[LOGICAL DEVICE] Could not satisfy the required queues necessary\n");
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
	// FIXME: this will bite me in the ass in the future, I've definitely made a mistake
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
	VkDeviceCreateInfo       dev_create   = {0};
	dev_create.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_create.pQueueCreateInfos          = q_create;
	dev_create.queueCreateInfoCount       = num_unique_queues;
	dev_create.pEnabledFeatures           = &dev_features;
	dev_create.ppEnabledExtensionNames    = PHYSICAL_EXTENSIONS;
	dev_create.enabledExtensionCount      = PHYSICAL_EXTENSIONS_COUNT;

#ifdef USE_VALIDATION_LAYERS
	dev_create.enabledLayerCount   = VALIDATION_LAYERS_COUNT;
	dev_create.ppEnabledLayerNames = VALIDATION_LAYERS;
#else
	dev_create.enabledLayerCount = 0;
#endif

	auto res = vkCreateDevice(vri->physical_dev, &dev_create, nullptr, &vri->logical_dev);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[LOGICAL DEVICE] Could not create the Vulkan Logical device: %s\n", VkResult_str(res));
		return false;
	}

	vkGetDeviceQueue(vri->logical_dev, indices.graphics, 0, &vri->device_queues.graphics);
	vkGetDeviceQueue(vri->logical_dev, indices.present, 0, &vri->device_queues.present);
	// vkGetDeviceQueue(vri->logical_dev, indices.compute, 0, &vri->device_queues.compure);
	// vkGetDeviceQueue(vri->logical_dev, indices.transfer, 0, &vri->device_queues.transfer);
	// vkGetDeviceQueue(vri->logical_dev, indices.sparse_binding, 0, &vri->device_queues.sparse_binding);

	llog(LOG_DEBUG, "[LOGICAL DEVICE] Logical device successfully created\n");

	return true;
}

bool destroy_logical_device(VulkanRuntimeInfo *vri) {

	vkDestroyDevice(vri->logical_dev, nullptr);

	return true;
}

bool create_surface(VulkanRuntimeInfo *vri) {

	auto res = glfwCreateWindowSurface(vri->instance, vri->sys_window, nullptr, &vri->surface);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[SURFACE] Could not create the Vulkan Surface: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[DEBUG] Surface successfully created\n");

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
		llog(LOG_FATAL, "[SWAPCHAIN] Could not create the Swapchain: %s\n", VkResult_str(res));
		return false;
	}

	vri->swapchain.swapchain = sc;
	vri->swapchain.format    = format.format;

	free(scd.formats);
	free(scd.modes);

	// retrieving images
	vkGetSwapchainImagesKHR(vri->logical_dev, sc, &vri->swapchain.buffers_count, nullptr);
	// count > 0 cuz the creation of the swapchain was successfull
	// I can exploi the fact the when i recreate the swapchain `cleanup_swapchain` does not free the pointer, just need to ensure that the defualt value is nullptr
	vri->swapchain.buffers = realloc(vri->swapchain.buffers, sizeof(VkImage) * vri->swapchain.buffers_count);
	TEST_MALLOC(vri->swapchain.buffers);
	vkGetSwapchainImagesKHR(vri->logical_dev, sc, &vri->swapchain.buffers_count, vri->swapchain.buffers);

	llog(LOG_DEBUG, "[SWAPCHAIN] Swapchain successfully created\n");

	return true;
}

bool re_create_swapchain(VulkanRuntimeInfo *vri) {

	int width = 0, height = 0;

	glfwGetFramebufferSize(vri->sys_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(vri->sys_window, &width, &height);
		llog(LOG_DEBUG, "[DRAWING] minized...\n");
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(vri->logical_dev);

	cleanup_swapchain(vri);

	if (create_swapchain(vri)) {
		return false;
	}

	if (create_image_views(vri)) {
		return false;
	}

	if (create_framebuffers(vri) == false) {
		return false;
	}

	return true;
}

bool destroy_swapchain(VulkanRuntimeInfo *vri) {

	free(vri->swapchain.buffers);
	vri->swapchain.buffers       = nullptr;
	vri->swapchain.buffers_count = 0;

	vkDestroySwapchainKHR(vri->logical_dev, vri->swapchain.swapchain, nullptr);

	return true;
}

bool create_image_views(VulkanRuntimeInfo *vri) {

	vri->swapchain.views = realloc(vri->swapchain.views, sizeof(VkImageView) * vri->swapchain.buffers_count);
	TEST_MALLOC(vri->swapchain.views);

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
			llog(LOG_FATAL, "[SWAPCHAIN] Could not create image views: %s\n", VkResult_str(res));
			free(vri->swapchain.views);
			return false;
		}
	}

	llog(LOG_DEBUG, "[SWAPCHAIN] Image views successfully created\n");

	return true;
}

bool destroy_image_views(VulkanRuntimeInfo *vri) {
	for (size_t i = 0; i < vri->swapchain.buffers_count; ++i) {
		vkDestroyImageView(vri->logical_dev, vri->swapchain.views[i], nullptr);
	}

	free(vri->swapchain.views);
	vri->swapchain.views = nullptr;

	return true;
}

bool create_pipeline(VulkanRuntimeInfo *vri) {
	shaderc_compilation_result_t vert_res;
	VkShaderModule               vert_module = {0};

	if (compile_shader_file("../shaders/main.vert", VERTEX_SHADER, &vert_res)) {
		VkShaderModuleCreateInfo sh_create = {0};
		sh_create.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		sh_create.codeSize                 = shaderc_result_get_length(vert_res);
		sh_create.pCode                    = (const uint32_t *)(shaderc_result_get_bytes(vert_res));

		auto res = vkCreateShaderModule(vri->logical_dev, &sh_create, nullptr, &vert_module);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SHADER] Could not create vulkan shader moldule: %s\n", VkResult_str(res));
		}
	} else {
		return false;
	}

	shaderc_compilation_result_t frag_res;
	VkShaderModule               frag_module = {0};

	if (compile_shader_file("../shaders/main.frag", FRAGMENT_SHADER, &frag_res)) {
		VkShaderModuleCreateInfo sh_create = {0};
		sh_create.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		sh_create.codeSize                 = shaderc_result_get_length(frag_res);
		sh_create.pCode                    = (const uint32_t *)(shaderc_result_get_bytes(frag_res));

		auto res = vkCreateShaderModule(vri->logical_dev, &sh_create, nullptr, &frag_module);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SHADER] Could not create vulkan shader moldule: %s\n", VkResult_str(res));
		}
	} else {
		return false;
	}

	VkPipelineShaderStageCreateInfo vert_stage_create = {0};
	vert_stage_create.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_create.stage                           = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_create.module                          = vert_module;
	vert_stage_create.pName                           = "main";

	VkPipelineShaderStageCreateInfo frag_stage_create = {0};
	frag_stage_create.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_create.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_create.module                          = frag_module;
	frag_stage_create.pName                           = "main";

	VkPipelineShaderStageCreateInfo sh_stages[] = {vert_stage_create, frag_stage_create};

	VkPipelineDynamicStateCreateInfo dn_create = {0};
	dn_create.sType                            = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dn_create.dynamicStateCount                = PIPELINE_DYNAMIC_STATE_COUNT;
	dn_create.pDynamicStates                   = PIPELINE_DYNAMIC_STATE;

	// TODO: set the correct layout
	// maybe ask for some kind of struct to base the layout to
	VkPipelineVertexInputStateCreateInfo vl_create = {0};
	vl_create.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vl_create.vertexBindingDescriptionCount        = 0;
	vl_create.pVertexBindingDescriptions           = nullptr; // Optional
	vl_create.vertexAttributeDescriptionCount      = 0;
	vl_create.pVertexAttributeDescriptions         = nullptr; // Optional

	// TODO: make this changeable for the user
	VkPipelineInputAssemblyStateCreateInfo ia_create = {0};
	ia_create.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia_create.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	ia_create.primitiveRestartEnable                 = VK_FALSE;

	VkPipelineViewportStateCreateInfo vp_create = {0};
	vp_create.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp_create.viewportCount                     = 1;
	vp_create.scissorCount                      = 1;

	VkPipelineRasterizationStateCreateInfo rt_create = {0};
	rt_create.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rt_create.depthClampEnable                       = VK_FALSE;
	rt_create.rasterizerDiscardEnable                = VK_FALSE;
	rt_create.polygonMode                            = VK_POLYGON_MODE_FILL; // TODO: settable wireframe here
	rt_create.lineWidth                              = 1.0f;
	rt_create.cullMode                               = VK_CULL_MODE_BACK_BIT;
	rt_create.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
	rt_create.depthBiasEnable                        = VK_FALSE;

	// TODO: This should probably be enabled
	VkPipelineMultisampleStateCreateInfo ms_create = {0};
	ms_create.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_create.sampleShadingEnable                  = VK_FALSE;
	ms_create.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
	ms_create.minSampleShading                     = 1.0f;     // Optional
	ms_create.pSampleMask                          = nullptr;  // Optional
	ms_create.alphaToCoverageEnable                = VK_FALSE; // Optional
	ms_create.alphaToOneEnable                     = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState cb_attachment = {0};
	cb_attachment.colorWriteMask                      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	cb_attachment.blendEnable                         = VK_FALSE;
	cb_attachment.blendEnable                         = VK_TRUE;
	cb_attachment.srcColorBlendFactor                 = VK_BLEND_FACTOR_SRC_ALPHA;
	cb_attachment.dstColorBlendFactor                 = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	cb_attachment.colorBlendOp                        = VK_BLEND_OP_ADD;
	cb_attachment.srcAlphaBlendFactor                 = VK_BLEND_FACTOR_ONE;
	cb_attachment.dstAlphaBlendFactor                 = VK_BLEND_FACTOR_ZERO;
	cb_attachment.alphaBlendOp                        = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo cb_create = {0};
	cb_create.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb_create.logicOpEnable                       = VK_FALSE;
	cb_create.attachmentCount                     = 1;
	cb_create.pAttachments                        = &cb_attachment;

	// TODO: setup uniforms
	VkPipelineLayoutCreateInfo pl_layout = {0};
	pl_layout.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl_layout.setLayoutCount             = 0;       // Optional
	pl_layout.pSetLayouts                = nullptr; // Optional
	pl_layout.pushConstantRangeCount     = 0;       // Optional
	pl_layout.pPushConstantRanges        = nullptr; // Optional

	auto res = vkCreatePipelineLayout(vri->logical_dev, &pl_layout, nullptr, &vri->pipeline.layout);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[PIPELINE] Could not create the pipeline layout: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[PIPELINE] Pipeline layout successfully crated\n");

	VkGraphicsPipelineCreateInfo pl_create = {0};
	pl_create.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pl_create.stageCount                   = 2;
	pl_create.pStages                      = sh_stages;
	pl_create.pVertexInputState            = &vl_create;
	pl_create.pInputAssemblyState          = &ia_create;
	pl_create.pViewportState               = &vp_create;
	pl_create.pRasterizationState          = &rt_create;
	pl_create.pMultisampleState            = &ms_create;
	pl_create.pDepthStencilState           = nullptr; // Optional
	pl_create.pColorBlendState             = &cb_create;
	pl_create.pDynamicState                = &dn_create;
	pl_create.layout                       = vri->pipeline.layout;
	pl_create.renderPass                   = vri->renderpass;
	pl_create.subpass                      = 0;

	res = vkCreateGraphicsPipelines(vri->logical_dev, VK_NULL_HANDLE, 1, &pl_create, nullptr, &vri->pipeline.pipeline);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[PIPELINE] Could not create the pipeline: %s\n", VkResult_str(res));
		return false;
	}

	vkDestroyShaderModule(vri->logical_dev, vert_module, nullptr);
	release_shader(vert_res);
	vkDestroyShaderModule(vri->logical_dev, frag_module, nullptr);
	release_shader(frag_res);

	llog(LOG_DEBUG, "[PIPELINE] Graphics pipeline successfully crated\n");

	return true;
}

bool destroy_pipeline(VulkanRuntimeInfo *vri) {
	vkDestroyPipeline(vri->logical_dev, vri->pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(vri->logical_dev, vri->pipeline.layout, nullptr);

	return true;
}

bool create_renderpass(VulkanRuntimeInfo *vri) {
	VkAttachmentDescription cl_attachment = {0};
	cl_attachment.format                  = vri->swapchain.format;
	cl_attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
	cl_attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	cl_attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	cl_attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	cl_attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	cl_attachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	cl_attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference cl_ref = {0};
	cl_ref.attachment            = 0;
	cl_ref.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &cl_ref;

	VkSubpassDependency sp_deps = {0};
	sp_deps.srcSubpass          = VK_SUBPASS_EXTERNAL;
	sp_deps.dstSubpass          = 0;
	sp_deps.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_deps.srcAccessMask       = VK_ACCESS_NONE;
	sp_deps.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sp_deps.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo rp_create = {0};
	rp_create.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_create.attachmentCount        = 1;
	rp_create.pAttachments           = &cl_attachment;
	rp_create.subpassCount           = 1;
	rp_create.pSubpasses             = &subpass;
	rp_create.dependencyCount        = 1;
	rp_create.pDependencies          = &sp_deps;

	auto res = vkCreateRenderPass(vri->logical_dev, &rp_create, nullptr, &vri->renderpass);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[RENDERPASS] Could not create the render pass: %s\n", VkResult_str(res));
	}

	llog(LOG_DEBUG, "[RENDERPASS] Renderpass successfully created\n");
	return true;
}

bool destroy_renderpass(VulkanRuntimeInfo *vri) {

	vkDestroyRenderPass(vri->logical_dev, vri->renderpass, nullptr);

	return true;
}

bool create_framebuffers(VulkanRuntimeInfo *vri) {

	vri->swapchain.framebuffers = realloc(vri->swapchain.framebuffers, sizeof(VkFramebuffer) * vri->swapchain.buffers_count);
	TEST_MALLOC(vri->swapchain.framebuffers)

	for (size_t i = 0; i < vri->swapchain.buffers_count; ++i) {
		VkImageView attachments[] = {
		    vri->swapchain.views[i]};

		VkFramebufferCreateInfo fb_create = {0};
		fb_create.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create.renderPass              = vri->renderpass;
		fb_create.attachmentCount         = 1;
		fb_create.pAttachments            = attachments;
		fb_create.width                   = vri->swapchain.extent.width;
		fb_create.height                  = vri->swapchain.extent.height;
		fb_create.layers                  = 1;

		auto res = vkCreateFramebuffer(vri->logical_dev, &fb_create, nullptr, &vri->swapchain.framebuffers[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[FRAMBUFFER] Could not create a frambuffer: %s\n", VkResult_str(res));
			free(vri->swapchain.framebuffers);
			return false;
		}
	}

	llog(LOG_DEBUG, "[FRAMBUFFER] frambuffers successfully created\n");

	return true;
}

bool destroy_framebuffers(VulkanRuntimeInfo *vri) {

	for (size_t i = 0; i < vri->swapchain.buffers_count; ++i) {
		vkDestroyFramebuffer(vri->logical_dev, vri->swapchain.framebuffers[i], nullptr);
	}
	free(vri->swapchain.framebuffers);
	vri->swapchain.framebuffers = nullptr;
	return true;
}

bool create_command_pool(VulkanRuntimeInfo *vri) {

	QueueFamilyIndicies qs = get_queue_families(vri->physical_dev, vri->surface);

	VkCommandPoolCreateInfo pool_crate = {0};
	pool_crate.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_crate.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_crate.queueFamilyIndex        = qs.graphics;

	auto res = vkCreateCommandPool(vri->logical_dev, &pool_crate, nullptr, &vri->cmd_pool);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND POOL] Could not create create the command pool: %s\n", VkResult_str(res));
		return false;
	}
	llog(LOG_DEBUG, "[COMMAND POOL] Command pool successfully created\n");

	return true;
}

bool destroy_command_pool(VulkanRuntimeInfo *vri) {
	vkDestroyCommandPool(vri->logical_dev, vri->cmd_pool, nullptr);
	return true;
}

bool create_command_buffer(VulkanRuntimeInfo *vri) {

	VkCommandBufferAllocateInfo cmd_crate = {0};
	cmd_crate.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_crate.commandPool                 = vri->cmd_pool;
	cmd_crate.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_crate.commandBufferCount          = MAX_CONCURRENT_FRAMES;

	auto res = vkAllocateCommandBuffers(vri->logical_dev, &cmd_crate, vri->cmd_buff_mngn.cmd_buffer);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND BUFFER] Could not create the command buffer: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[COMMAND BUFFER] Command buffer successfully created\n");
	return true;
}

bool create_sync_objects(VulkanRuntimeInfo *vri) {
	VkSemaphoreCreateInfo sem_create = {0};
	sem_create.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fnc_create = {0};
	fnc_create.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fnc_create.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {

		auto res = vkCreateSemaphore(vri->logical_dev, &sem_create, nullptr, &vri->cmd_buff_mngn.image_available[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the Image available semaphore: %s\n", VkResult_str(res));
			return false;
		}

		res = vkCreateSemaphore(vri->logical_dev, &sem_create, nullptr, &vri->cmd_buff_mngn.render_finished[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the render finished semaphore: %s\n", VkResult_str(res));
			return false;
		}

		res = vkCreateFence(vri->logical_dev, &fnc_create, nullptr, &vri->cmd_buff_mngn.in_flight_fence[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the render in in flight fence fence: %s\n", VkResult_str(res));
			return false;
		}
	}

	llog(LOG_DEBUG, "[SYNCHRO] Synchronizaion semphores successfully created\n");

	return true;
}

bool destroy_sync_objects(VulkanRuntimeInfo *vri) {

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
		vkDestroyFence(vri->logical_dev, vri->cmd_buff_mngn.in_flight_fence[i], nullptr);
		vkDestroySemaphore(vri->logical_dev, vri->cmd_buff_mngn.render_finished[i], nullptr);
		vkDestroySemaphore(vri->logical_dev, vri->cmd_buff_mngn.image_available[i], nullptr);
	}

	llog(LOG_DEBUG, "[SYNCHRO] Synchronization objects successfully destroyed\n");

	return true;
}
