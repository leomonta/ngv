#pragma once

#include "utils.h"

#include <vulkan/vulkan_core.h>

typedef struct {
	uint32_t graphics;

	bitfield available_families;
} QueueFamilyIndicies;

#define USED_QUEUE_FAMILIES sizeof(QueueFamilyIndicies) % sizeof(uint32_t) - 1;

typedef struct {
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkPhysicalDevice         physical_dev;
	QueueFamilyIndicies      families;
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
 * @param[out] the vulkan context where to put the created instance
 *
 * @return if the operation was successfull or not
 */
bool create_instance(VulkanRuntimeInfo *vri);

/**
 * Destroy a VkInstance and its associated data
 *
 * @param[in] the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_instance(VulkanRuntimeInfo *vri);

/**
 * Creates a vulkan callback to attach to the validation layer
 * It uses the logger function
 *
 * @param[in] the vulkan context to use
 *
 * @return true if successfull, false otherwise
 */
bool attach_logger_callback(VulkanRuntimeInfo *vri);

/**
 * destroy the vulkna callback attached to the logger function
 *
 * @param[in] the vulkan context to use
 *
 * @return true if successfull, false otherwise
 */
bool detach_logger_callback(VulkanRuntimeInfo *vri);

/**
 * List the available physical devices to use
 *
 * @param[in] the vulkan context to use
 *
 * @return true if successfull, false if no suitable device (or at all) was found
 */
bool pick_physical_device(VulkanRuntimeInfo *vri);
