#pragma once

#include "ngv_objects.h"

uint32_t find_memory_type(const uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_dev);

/***
 * Issues a copy command on the `vri` transfer queue from `stc` to `dst` of `size` bytes
 * I may want to include some kind plug-in semaphore / fances as in/out parameters but only if I'll need it
 *
 * @param[in] `vri` the vulkan context to use
 * @param[in] `src` the source buffer
 * @param[in] `dst` the destination biffer
 * @param[in] `size` the amount of bytes to copy
 *
 * @return if the operation was successful
 */
bool copy_buffer(VulkanSetupInfo *setup_info, VkBuffer src, VkBuffer dst, VkDeviceSize size);

/**
 * @param[in] `setup_info` the vulkan context to use
 * @param[in] `raw_data`
 *
 * @return if the operation was successful
 */
bool update_uniform_buffer(VulkanSetupInfo *setup_info, void *raw_data);

bool push_to_buffer(const VulkanSetupInfo *setup_info, const VulkanFrameData *frame_data, const VkBuffer *buff, const void *pushed_data, const VkDeviceSize size);
