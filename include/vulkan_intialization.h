#pragma once

#include <vulkan/vulkan_core.h>

typedef struct {
	VkInstance instance;
} VulkanRuntimeInfo;

const char *VkError_str(VkResult res);

bool create_instance(VkInstance *instance);
