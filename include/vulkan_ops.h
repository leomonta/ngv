#pragma once

#include "ngv_objects.h"

#include <vulkan/vulkan.h>

/**
 * Marks the command buffer as ready to receive commands
 *
 * @param[in] `buff`
 * @param[in] `img_index`
 *
 * @return if the operation was successfull
 */
bool record_cmd_buff(VulkanSetupInfo *setup_info, VulkanFrameData *frame_data, uint32_t img_index);

/**
 * Sends command to the command buffer and to the other queues to issue a draw call
 *
 * @param[in] `vri` the vulkan context to user
 */
void draw_frame(VulkanStaticInfo *static_info, VulkanSetupInfo *setup_info, VulkanFrameData *frame_data);

/**
 * Sets the given `command_buffer` to a begun one from the pool specified by `kind`
 *
 * @param[in] `vri` the vulkan context to use
 * @param[in] `kind` the kind of queue and command pool to use
 * @param[out] `command_buffer` where to put the begun
 *
 * @return if the operation was successful
 */
bool begin_temporary_command_buffer(VulkanRuntimeInfo *vri, QueueKind kind, VkCommandBuffer *command_buffer);

/**
 * Ends and submits the `command_buffer`, then waits for its completion
 * the command_buffer is invalidated after use.
 *
 * @param[in] `vri` the vulkan context to use
 * @param[in] `kind` the kind of queue and command pool to use
 * @param[in] `command_buffer` the command buffer to end
 *
 * @return if the operation was successful
 */
bool end_temporary_command_buffer(VulkanRuntimeInfo *vri, QueueKind kind, VkCommandBuffer command_buffer);

void transition_image_layout(VulkanRuntimeInfo *vri, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

void copy_buffer_to_image(VulkanRuntimeInfo *vri, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
