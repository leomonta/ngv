#pragma once

#include "config.h"
#include "vulkan_objects.h"

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
 * Queries glfw about the required extensions for Vulkan and returns a **mallocated array that needs to be manually freed**
 *
 * @param[out] `count` the number of required extensions returned
 *
 * @return an array of required extensions names
 */
const char **get_required_extensions(uint32_t *count);

/**
 * Queries the queue capabilities of the device and returns the indicies of the available ones
 *
 * @param[in] `device` the device to get the available families from
 * @param[in] `surface` needed to check for surface specific families
 *
 * @return `true` if the given device satisfies the defined extensions, `false`
 */
QueuesIndicies get_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

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
VkPhysicalDevice filter_suitable_devices(const VkPhysicalDevice *devs, const size_t count, VkSurfaceKHR surface);

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
 * @param[out] `result` a `shaderc_compilation_result_t` that holds the compiled code and other metadata
 *
 * @return the compiled shader if successfull, nullptr otherwise
 */
bool compile_shader_file(const char *filename, const ShaderKind kind, shaderc_compilation_result_t *result);

/**
 * Compiles the shader given as input with shaderc
 * the returned string is heap mallocated, so it should be freed when no longer needed
 *
 * @param[in] `code` the actual code of the shader
 * @param[in] `kind` the kind of shader to be compiled
 * @param[out] `result` a `shaderc_compilation_result_t` that holds the compiled code and other metadata
 *
 * @return the compiled shader if successfull, nullptr otherwise
 */
bool compile_shader(const char *code, const size_t size, const ShaderKind kind, shaderc_compilation_result_t *result);

/**
 * Releases the data stored in the `shaderc_compilation_result_t`
 *
 * @param[in] `res` the result to release
 *
 * @return if the operation was successfull
 */
bool release_shader(shaderc_compilation_result_t res);

/**
 * Destroys all of the swachain hadles in the given context, but does not deallocate the memory
 * this should be used to clean the swapchain before recreating it
 *
 * @param[in] `res` the result to release
 *
 * @return if the operation was successfull
 */
bool cleanup_swapchain(VulkanRuntimeInfo *vri);

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
 * @param[in] `vri` the vulkan context to use
 * @param[in] `size` the size in bytes of the buffer
 * @param[in] `usage` an OR list of flags for what will it be used for
 * @param[in] `properties`
 * @param[out] `buffer` where to put the created buffer handle
 * @param[out] `buffer_memory` where to put the created buffer address
 *
 * @return a memory if successfull, else 0
 */
bool create_buffer(VulkanRuntimeInfo *vri, const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory);

/**
 * Creates and allocates a GPU side buffer
 *
 * @param[in] `vri` the vulkan context to use
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
bool create_image(VulkanRuntimeInfo *vri, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage *texture, VkDeviceMemory *texture_mem);
