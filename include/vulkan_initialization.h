#pragma once

#include "utils.h"

#include <vulkan/vulkan_core.h>

typedef struct {
	uint32_t graphics;
	uint32_t compute;
	uint32_t transfer;
	uint32_t sparse_binding;

	bitfield available_families;
} QueueFamilyIndicies;

enum : char {
	GRAPHIC_QUEUE_INDEX,
	COMPUTE_QUEUE_INDEX,
	TRANSFER_QUEUE_INDEX,
	SPARSE_BINDING_QUEUE_INDEX
};

typedef struct {
	VkQueue graphics;
	VkQueue compure;
	VkQueue transfer;
	VkQueue sparse_binding;
} Queues;

#define USED_QUEUE_FAMILIES sizeof(QueueFamilyIndicies) % sizeof(uint32_t) - 1;

typedef struct {
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkPhysicalDevice         physical_dev;
	VkDevice                 logical_dev;
	Queues                   device_queues;
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
 * destroy the vulkan callback attached to the logger function
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

/**
 * Creates a logical device based on the physical device
 *
 * @param[in] the vulkan context where to put the logical device
 *
 * @return true if successfull, false if no suitable device (or at all) was found
 */
bool create_logical_device(VulkanRuntimeInfo *vri);


/**
 * Destroy a VkDevice and its associated data
 *
 * @param[in] the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_logical_device(VulkanRuntimeInfo *vri);
