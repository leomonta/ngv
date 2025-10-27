#pragma once

#include "config.h"
#include "ngv_objects.h"

#include <shaderc/shaderc.h>

// ugly I KNOW
// #ifdef USE_VALIDATION_LAYERS
// static const char        *VALIDATION_LAYERS[]     = {"VK_LAYER_KHRONOS_validation"};
// static constexpr unsigned VALIDATION_LAYERS_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);
// #endif

/**
 * Checks if the validation layers are supported on this system
 *
 * @return true if validation layers are supported, false otherwise
 */
bool check_validation_layer_support();

/**
 * Given a `ShaderKind` returns its enum symbol name
 *
 * @param[in] `kind` the shader kind to `stringify`
 *
 * @return the string containing the name of the `ShaderKind` enum namf
 */
const char *ShaderKind_str(const ShaderKind kind);

/**
 * Given a `VkResult` returns its '#define' name
 *
 * @param[in] `res` the vkResult to 'stringify'
 *
 * @return the string containing the name of the `VkResult` enum / define name
 */
const char *VkResult_str(const VkResult res);

/**
 * Returns the details of the device swapchain
 *
 * @param[in] `device` the physical device that supports the swapchain
 * @param[in] `surface` the surface of the swapchain
 *
 * @return a `swapchain` details struct
 */
SwapchainDetails get_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface);

/**
 * Return a memory type that satisfies the given filter and properties
 *
 * @param[in] `vri` the vulkan context to use
 * @param[in] `typeFilter`
 * @param[in] `properties`
 *
 * @return a memory if successfull, else 0
 */
uint32_t get_memory_type_index(VulkanRuntimeInfo *vri, const uint32_t typeFilter, const VkMemoryPropertyFlags properties);

/**
 * Creates and allocates a GPU side buffer
 *
 * @param[in] `setup_infp` the engine setup information
 * @param[in] `physical_dev` the physical device to find the correct memory type for
 * @param[in] `size` the size in bytes of the buffer
 * @param[in] `usage` an OR list of flags for what will it be used for
 * @param[in] `properties`
 * @param[out] `buffer` where to put the created buffer handle
 * @param[out] `buffer_memory` where to put the created buffer address
 *
 * @return a memory if successfull, else 0
 */
bool create_buffer(VulkanSetupInfo *setup_info, VkPhysicalDevice physical_dev, const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory);

/**
 * Creates and allocates a GPU side buffer
 *
 * @param[in] `logical_dev` the logical device to create the image for
 * @param[in] `physical_dev` the physical device to find the correct memory type for
 * @param[in] `width` the width in pixels of the buffer
 * @param[in] `height` the height in pixels of the buffer
 * @param[in] `format` the rgb format of the image
 * @param[in] `tiling` optimal of linear tiling
 * @param[in] `usage` an OR list of flags for what will it be used for
 * @param[out] `texture` where to put the created buffer handle
 * @param[out] `texture_mem` where to put the created buffer address
 *
 * @return a memory if successfull, else 0
 */
bool create_image(VkDevice logical_dev, VkPhysicalDevice physical_dev, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *image_mem);
