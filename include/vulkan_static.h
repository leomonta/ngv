#pragma once

#include "ngv_objects.h"

/**
 * Fill the given `VulkanStaticInfo` with valid objects as configured
 *
 * @param[in] `settings` the settings for the static info creation
 * @param[out] `vsi` the vulkan static info to fill
 *
 * @return if the operation was successfull or not
 */
bool create_static_info(VulkanStaticInfo *vsi, VulkanStaticSettings settings);

/**
 * Creates a `VkInstance`
 *
 * @param[in][out] `instance` the vulkan instance to fill
 *
 * @return if the operation was successfull or not
 */
bool create_instance(VkInstance *instance);

/**
 * Destroy a `VkInstance` and its associated data
 *
 * @return if the operation was successfull or not
 */
bool destroy_instance(VulkanStaticInfo *vsi);

/**
 * Queries glfw about the required extensions for Vulkan and returns a **mallocated array that needs to be manually freed**
 *
 * @param[out] `count` the number of required extensions returned
 *
 * @return an array of required extensions names
 */
const char **get_required_extensions(uint32_t *count);

/**
 * Initializes `glfw` and returns a newly created window
 *
 * @param[out] `window` the window object to init
 *
 * @return if the operation was successfull
 */
bool init_window(GLFWwindow **window);

/**
 * Terminate the window and `glfw`
 *
 * @param[in] `window` the window to destroy
 */
bool terminate_window(GLFWwindow *window);

/**
 * Creates a system specific `KHRsurface` (Thansks GLFW) to render stuff to
 *
 * @param[in] `surface` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_surface(VulkanStaticInfo *surface);

/**
 * Destroy a `VkSurface` and its associated data
 *
 * @param[in] `surface` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_surface(VulkanStaticInfo *surface);

/**
 * List the available physical devices to use
 *
 * @param[in] `preferred_dev_id` the id of the preferred physical device to use, if UINT32_MAX any will be taken
 * @param[in] `vsi` the vulkan static
 *
 * @return `true` if successfull, `false` if no suitable device (or at all) was found
 */
bool pick_physical_device(const VulkanStaticSettings settings, VulkanStaticInfo *vsi);
