#pragma once

#include <vulkan/vulkan_core.h>

typedef struct {
	VkInstance instance;
} VulkanRuntimeInfo;

/**
 * given a VkResult returns its 'define' name 
 *
 * @param[in] res the vkResult to 'stringify'
 * 
 * @return the string containing the name of the VkResult compile time name
 */
const char *VkResult_str(VkResult res);

bool check_validation_layer_support();

bool create_instance(VkInstance *instance);
bool destroy_instance(VkInstance *instance);
