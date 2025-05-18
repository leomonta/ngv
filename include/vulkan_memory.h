#pragma once

#include "vulkan_initialization.h"

uint32_t find_memory_type(const uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanRuntimeInfo *vri);
