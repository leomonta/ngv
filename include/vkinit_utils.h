#pragma once

#include "vulkan_initialization.h"

/**
 * Queries the queue capabilities of the device and returns the indicies of the available ones
 *
 * @param[in] `device` the device to find the available families from
 * @param[in] `surface` needed to check for surface specific families
 *
 * @return `true` if the given device satisfies the defined extensions, `false`
 */
QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

/**
 * Checks if the give `VkPhysicalDevice` supports some required extension to render on screen (e.g. swapchain)
 *
 * @param[in] `device` the device to test the extensions of
 *
 * @return `true` if the given device satisfies the defined extensions, `false`
 */
bool has_required_extensions(VkPhysicalDevice device);

/**
 * Pick the best physical (as in hardware) device based on some requirements (swap chain support, graphic family availability)
 * and some metrics (is a discrete GPU, how much memory it has)
 *
 * @param[in] `devs` an array of physical devices to pick 'from
 * @param[in] `count` the length of the `devs` arrays
 * @param[in] `surface` the surface for which the device is queried againts
 *
 * @return the best device available or a `VK_NULL_HANDLE` if none is found
 */
VkPhysicalDevice pick_best_device(const VkPhysicalDevice *devs, const size_t count, VkSurfaceKHR surface);

/**
 * Given a `VkResult` returns its '#define' name
 *
 * @param[in] `res` the vkResult to 'stringify'
 *
 * @return the string containing the name of the `VkResult` enum / define name
 */
const char *VkResult_str(const VkResult res);
