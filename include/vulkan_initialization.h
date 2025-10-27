#pragma once
#include "ngv_objects.h"

/**
 * returns if any requested validation layer is available
 *
 * @return `true` if there are available validation layers, `false` otherwise
 */
bool check_validation_layer_support();

/**
 * Creates a vulkan callback to attach to the validation layer
 * It uses the logger function
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return true if successfull, false otherwise
 */
bool attach_logger_callback(VulkanRuntimeInfo *vri);

/**
 * destroy the vulkan callback attached to the logger function
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return true if successfull, false otherwise
 */
bool detach_logger_callback(VulkanRuntimeInfo *vri);

/**
 * List the available physical devices to use
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return `true` if successfull, `false` if no suitable device (or at all) was found
 */
bool pick_physical_device(VulkanRuntimeInfo *vri);

/**
 * Creates a logical device (`VkDevice`) based on the physical device
 *
 * @param[in] `vri` the vulkan context where to put the logical device
 *
 * @return `true` if successfull, `false` if no suitable device (or at all) was found
 */
bool create_logical_device(VulkanRuntimeInfo *vri);

/**
 * Destroy a `VkDevice` and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_logical_device(VulkanRuntimeInfo *vri);

/**
 * Creates a system specific `KHRsurface` (Thansks GLFW) to render stuff to
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_surface(VulkanRuntimeInfo *vri);

/**
 * Destroy a `VkSurface` and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_surface(VulkanRuntimeInfo *vri);

/**
 * Creates a swapchain based on the best formats and modes available
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_swapchain(VulkanRuntimeInfo *vri);

/**
 * destroys the old swapchain and creates it anew
 * useful for when the viewport has changed
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool re_create_swapchain(VulkanRuntimeInfo *vri);

/**
 * Destroy a swapchain and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_swapchain(VulkanRuntimeInfo *vri);

/**
 * Creates images views for the swapchain images
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_image_views(VulkanRuntimeInfo *vri);

/**
 * Destroy the images views of the swapchain images
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_image_views(VulkanRuntimeInfo *vri);

/**
 * Creates the shader pipeline
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_pipeline(VulkanRuntimeInfo *vri);

/**
 * Destroy the shader pipeline
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_pipeline(VulkanRuntimeInfo *vri);

/**
 * Creates the graphics pipleline with all of its stages
 * shaders, vertexes layout, uniforms...
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_renderpass(VulkanRuntimeInfo *vri);

/**
 * Destroys the pipeline
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_renderpass(VulkanRuntimeInfo *vri);

/**
 * Create the framebuffers
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_framebuffers(VulkanRuntimeInfo *vri);

/**
 * Destroys the framebuffer
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_framebuffers(VulkanRuntimeInfo *vri);

/**
 * Creates the command pool
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_command_pool(VulkanRuntimeInfo *vri);

/**
 * Destroys the command pool
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_command_pool(VulkanRuntimeInfo *vri);

bool create_depth_objects(VulkanRuntimeInfo *vri);
bool destroy_depth_objects(VulkanRuntimeInfo *vri);

/**
 * Creates the texture images
 * Should redo this function to create a texture on demand and return its index
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_texture_image(VulkanRuntimeInfo *vri);

/**
 * Destroys the texture images
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_texture_image(VulkanRuntimeInfo *vri);

bool create_texture_view(VulkanRuntimeInfo *vri, uint32_t index);
bool destroy_texture_view(VulkanRuntimeInfo *vri, uint32_t index);

bool create_texture_sampler(VulkanRuntimeInfo *vri, uint32_t index);
bool destroy_texture_sampler(VulkanRuntimeInfo *vri, uint32_t index);

/**
 * Creates the command buffer
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_command_buffer(VulkanRuntimeInfo *vri);

/**
 * Create the samaphores needed for GPU <-> CPU Synchronization
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_sync_objects(VulkanRuntimeInfo *vri);

/**
 * Destroys the samaphores needed for GPU <-> CPU Synchronization
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_sync_objects(VulkanRuntimeInfo *vri);

/**
 * Create the vertex buffer and sets it up with the default vertex layout
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_vertex_buffer(VulkanRuntimeInfo *vri);

/**
 * Destroys the vertex buffer
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_vertex_buffer(VulkanRuntimeInfo *vri);

/**
 * Create the index buffer and sets it up.
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_index_buffer(VulkanRuntimeInfo *vri);

/**
 * Destroys the index buffer
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_index_buffer(VulkanRuntimeInfo *vri);

/**
 * Create the uniform buffer and sets it up.
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_uniform_buffer(VulkanRuntimeInfo *vri);

/**
 * Destroys the uniform buffer
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_uniform_buffer(VulkanRuntimeInfo *vri);

/**
 * Creates a descriptor pool
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_descriptor_pool(VulkanRuntimeInfo *vri);

/**
 * Destroys the descriptor pool
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_descriptor_pool(VulkanRuntimeInfo *vri);

/**
 * Creates a descriptor set
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_descriptor_set(VulkanRuntimeInfo *vri);

/**
 * Destroys the descriptor set
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_descriptor_set(VulkanRuntimeInfo *vri);
