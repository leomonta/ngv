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

bool create_setup_info(const VulkanSetupSettings *settings, VulkanSetupInfo *vsi, VulkanStaticInfo *static_info);

/**
 * Creates a logical device (`VkDevice`) based on the physical device
 *
 * @param[in] `vri` the vulkan context where to put the logical device
 *
 * @return `true` if successfull, `false` if no suitable device (or at all) was found
 */
bool create_logical_device(VulkanSetupInfo *vsi, VulkanStaticInfo *static_info);

/**
 * Destroy a `VkDevice` and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_logical_device(VulkanRuntimeInfo *vsi);

/**
 * Creates a swapchain based on the best formats and modes available
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_swapchain(VulkanSetupInfo *vsi, VulkanStaticInfo *static_info);

/**
 * destroys the old swapchain and creates it anew
 * useful for when the viewport has changed
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool re_create_swapchain(VulkanSetupInfo *vsi, VulkanStaticInfo *static_info);

/**
 * Destroy a swapchain and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_swapchain(VulkanRuntimeInfo *vri);

/**
 * Creates images views for the swapchain images
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_swapchain_image_views(VulkanSetupInfo *vsi);
