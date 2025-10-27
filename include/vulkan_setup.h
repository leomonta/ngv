#pragma once

#include "ngv_objects.h"

// typedef struct {
// VkDevice         logical_dev;
// VkRenderPass     renderpass;
// VkCommandPool    graphics_cmd_pool;
// VkCommandPool    transfer_cmd_pool;
// VkDescriptorPool descriptor_pool;
// VkCommandBuffer  transfer_cmd_buff;
// ShaderPipeline   pipeline;
// CreatedQueues    device_queues;
// QueuesIndicies   device_queues_indices;
// SwapchainInfo    swapchain;
//} VulkanSetupInfo;

bool create_setup_info(const VulkanSetupSettings *settings, VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info);

bool destroy_setup_info(VulkanSetupInfo *setup_info);

/**
 * Creates a logical device (`VkDevice`) based on the physical device
 *
 * @param[in] `setup_info` the vulkan context where to put the logical device
 *
 * @return `true` if successfull, `false` if no suitable device (or at all) was found
 */
bool create_logical_device(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info);

/**
 * Destroy a `VkDevice` and its associated data
 *
 * @param[in] `logical_dev` the logical device to destroy
 *
 * @return if the operation was successfull or not
 */
bool destroy_logical_device(VkDevice logical_dev);

/**
 * Creates a swapchain based on the best formats and modes available
 *
 * @param[in] `setup_info` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_swapchain(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info);

/**
 * destroys the old swapchain and creates it anew
 * useful for when the viewport has changed
 *
 * @param[in] `setup_info` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool re_create_swapchain(VulkanSetupInfo *setup_info, VulkanStaticInfo *static_info);

/**
 * Destroy a swapchain and its associated data
 *
 * @param[in] `setup_info` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_swapchain(VulkanSetupInfo *setup_info);

/**
 * Creates images views for the swapchain images
 *
 * @param[in] `setup_info` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_swapchain_image_views(VulkanSetupInfo *setup_info);

/**
 * Creates the depth buffer and linked objects
 *
 * @param[in] `setup_info` the setup information necessary
 * @param[in] `physical_dev` physical device to create the images
 *
 * @return if the operation was successfull or not
 */
bool create_depth_objects(VulkanSetupInfo *setup_info, VkPhysicalDevice physical_dev);

/**
 * Destroys the depth buffer and connected objects
 *
 * @param[in] `setup_info` the setup information necessary
 *
 * @return if the operation was successfull or not
 */
bool destroy_depth_objects(VulkanSetupInfo *setup_info);

/**
 * Creates the framebuffer and its linked resources
 *
 * @param[in] `setup_info` the setup information necessary
 *
 * @return if the operation was successfull or not
 */
bool create_framebuffers(VulkanSetupInfo *setup_info);

/**
 * Destroy the framebuffer and its linked resources
 *
 * @param[in] `setup_info` the setup information necessary
 *
 * @return if the operation was successfull or not
 */
bool destroy_framebuffers(VulkanSetupInfo *setup_info);


bool create_pipeline(const VulkanSetupSettings settings, VulkanSetupInfo *setup_info);

bool destroy_pipeline(VulkanSetupInfo *setup_info);
