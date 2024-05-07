#pragma once

#include <vulkan/vulkan_core.h>

typedef struct {
	VkInstance instance;
} VulkanRuntimeInfo;

const char *VkError_str(VkResult res);

bool check_validation_layer_support();

bool create_instance(VkInstance *instance);
bool destroy_instance(VkInstance *instance);
