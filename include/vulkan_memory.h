#pragma once

#include "vulkan_initialization.h"

uint32_t find_memory_type(const uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanRuntimeInfo *vri);

/***
 * Issues a copy command on the `vri` transfer queue from `stc` to `dst` of `size` bytes
 * I may want to include some kind plug-in semaphore / fances as in/out parameters but only if I'll need it
 *
 * @param[in] `vri` the vulkan context to use
 * @param[in] `src` the source buffer
 * @param[in] `dst` the destination biffer
 * @param[in] `size` the amount of bytes to copy
 */
bool copy_buffer(VulkanRuntimeInfo *vri, VkBuffer src, VkBuffer dst, VkDeviceSize size);
