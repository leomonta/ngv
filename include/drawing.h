#pragma once

#include "vulkan_initialization.h"
#include <vulkan/vulkan.h>

/**
 * Marks the command buffer as ready to receive commands
 *
 * @param[in] `buff`
 * @param[in] `img_index`
 *
 * @return if the operation was successfull
 */
bool record_cmd_buff(VulkanRuntimeInfo *vri, uint32_t img_index);
