#pragma once

#include "vulkan_initialization.h"

#include <shaderc/shaderc.h>

/**
 * Queries the queue capabilities of the device and returns the indicies of the available ones
 *
 * @param[in] `device` the device to get the available families from
 * @param[in] `surface` needed to check for surface specific families
 *
 * @return `true` if the given device satisfies the defined extensions, `false`
 */
QueueFamilyIndicies
get_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

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
 * Picks the 'best' surface format for the swapchain
 *
 * @param[in] `formats` array of available formats to choose from
 * @param[in] `count` the number of formats in the array
 *
 * @return the chosen `VkSurfaceFormatKHR`
 */
VkSurfaceFormatKHR pick_swapchain_format(const VkSurfaceFormatKHR *formats, const size_t count);

/**
 * Picks the 'best' present mode for the swapchain (aka `MAILBOX)`
 * if `MAILBOX` is not present pick `FIFO`
 *
 * @param[in] `modes` array of available modes to choose from
 * @param[in] `count` the number of modes in the array
 *
 * @return the chosen `VkPresentModeKHR`
 */
VkPresentModeKHR pick_swapchain_mode(const VkPresentModeKHR *modes, const size_t count);

/**
 * Picks the 'best' extend (surface area) for the swapchain
 *
 * @param[in] `caps` the capabilities the surface
 * @param[in] `win` the glfw window of the application to retrive the desired dimensions for the extent
 *
 * @return the window 'pixel correct' extent if possible, else the maximum extent of the surface
 */
VkExtent2D pick_swapchain_extent(const VkSurfaceCapabilitiesKHR *caps, GLFWwindow *win);

/**
 * Reads the shader code from a file and compiles it with shaderc
 * the returned string is heap mallocated, so it should be freed when no longer needed
 *
 * @param[in] `filename` the shader file location
 * @param[in] `kind` the kind of shader to be compiled
 *
 * @return the compiled shader if successfull, nullptr otherwise
 */
bool compile_shader_file(const char *filename, const ShaderKind kind, ShaderInfo *result);

/**
 * Compiles the shader given as input with shaderc
 * the returned string is heap mallocated, so it should be freed when no longer needed
 *
 * @param[in] `code` the actual code of the shader
 * @param[in] `kind` the kind of shader to be compiled
 *
 * @return the compiled shader if successfull, nullptr otherwise
 */
bool compile_shader(const char *code, const size_t size, const ShaderKind kind, ShaderInfo *result);

/**
 * Releases the data stored in the `shaderc_compilation_result_t`
 *
 * @param[in] `res` the result to release
 *
 * #return if the operation was successfull
 */
bool release_shader(shaderc_compilation_result_t res);

