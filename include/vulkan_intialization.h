#pragma once

#include <vulkan/vulkan_core.h>

typedef struct {
	VkInstance instance;
} VulkanRuntimeInfo;

/**
 * Given a VkResult returns its 'define' name 
 *
 * @param[in] res the vkResult to 'stringify'
 * 
 * @return the string containing the name of the VkResult enum / define name
 */
const char *VkResult_str(VkResult res);

/**
 * returns if any requested validation layer is available
 *
 * @return true if there are avilable validation layers, false otherwise
 */
bool check_validation_layer_support();

/**
 * Creates a VkInstance 
 *
 * @param[out] the pointer where to create the instance
 *
 * @return if the operation was successfull or not
 */
bool create_instance(VkInstance *instance);

/**
 * Destroy a VkInstance and its associated data
 *
 * @param[in] the instance to destroy
 *
 * @return if the operation was successfull or not
 */
bool destroy_instance(VkInstance *instance);
