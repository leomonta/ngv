#pragma once

#include <vulkan/vulkan_core.h>

typedef struct {
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkPhysicalDevice         physical_dev;
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
bool create_instance(VulkanRuntimeInfo *vri);

/**
 * Destroy a VkInstance and its associated data
 *
 * @param[in] the instance to destroy
 *
 * @return if the operation was successfull or not
 */
bool destroy_instance(VulkanRuntimeInfo *vri);

/**
 * Creates a vulkan callback to attach to the validation layer
 * It uses the logger function
 *
 * @param[in] instance to which instance this debugger is boud to
 * @param[in] debug_logger the logger to destroy
 *
 * @return true if successfull, false otherwise
 */
bool attach_logger_callback(VulkanRuntimeInfo *vri);

/**
 * destroy the vulkna callback attached to the logger function
 *
 * @param[in] instance to which instance this debugger is boud to
 * @param[in] debug_logger the logger to destroy
 *
 * @return true if successfull, false otherwise
 */
bool detach_logger_callback(VulkanRuntimeInfo *vri);

/**
 *
 */
void pick_physical_device(VulkanRuntimeInfo *vri);
