#pragma once

#include "ngv_objects.h"

/*
typedef struct {
    VkPhysicalDevice physical_dev; // the same present in static info, just a shortcut
    VkDevice         logical_dev;  // the same present in setup info, just a shortcut
    VkBuffer         index_buff;
    VkDeviceMemory   index_buff_mem;
    VkBuffer         vertex_buff;
    VkDeviceMemory   vertex_buff_mem;
    VkCommandBuffer  cmd_buff[MAX_CONCURRENT_FRAMES];
    VkSemaphore      image_available[MAX_CONCURRENT_FRAMES];
    VkSemaphore      render_finished[MAX_CONCURRENT_FRAMES];
    VkFence          in_flight_fence[MAX_CONCURRENT_FRAMES];
    VkDescriptorSet  descriptor_sets[MAX_CONCURRENT_FRAMES];
    VkBuffer         uniform_buff[MAX_CONCURRENT_FRAMES];
    VkDeviceMemory   uniform_buff_mem[MAX_CONCURRENT_FRAMES];
    void            *uniform_buff_mapped[MAX_CONCURRENT_FRAMES];
    TextureData      textures;
} VulkanFrameData;
*/

bool create_frame_data(const NGVRendererSettings *settings, VulkanFrameData *frame_data, VulkanSetupInfo *setup_info);

bool destroy_frame_data(VulkanFrameData *frame_data);

/**
 * Creates the texture images
 * Should redo this function to create a texture on demand and return its index
 *
 * @param[in] `setup_info` the vulkan context to use
 * @param[in|out] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_texture_image(VulkanSetupInfo *setup_info, VulkanFrameData *frame_data);

/**
 * Destroys the texture images
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_texture_image(VulkanFrameData *frame_data);

/**
 * Creates a view for the specified texture index
 *
 * @param[in] `index` the index of the texture to create the view for
 * @param[in|out] `frame_data` where to put the view and where to find the texture image buffer
 *
 */
bool create_texture_view(const uint32_t index, VulkanFrameData *frame_data);

/**
 * Destroys the view for the specified texture index
 *
 * @param[in] `index` the index of the texture to destroy the view for
 * @param[in|out] `frame_data` where to find the texture image view
 *
 */
bool destroy_texture_view(const uint32_t index, VulkanFrameData *frame_data);

bool create_texture_sampler(VulkanFrameData *frame_data, uint32_t index);
bool destroy_texture_sampler(VulkanFrameData *frame_data, uint32_t index);

/**
 * Create the samaphores needed for GPU <-> CPU Synchronization
 *
 * @param[in] `frame_data` the vulkan context to use
 * @param[in] `setup_info` info needed for command pool
 *
 * @return if the operation was successfull or not
 */
bool create_sync_objects(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info);

/**
 * Destroys the samaphores needed for GPU <-> CPU Synchronization
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_sync_objects(VulkanFrameData *frame_data);

/**
 * Create the vertex buffer and sets it up with the default vertex layout
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_vertex_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info, void* vertex_data, VkDeviceSize size);

/**
 * Destroys the vertex buffer
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_vertex_buffer(VulkanFrameData *frame_data);

/**
 * Create the index buffer and sets it up.
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_index_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info, void* index_data, VkDeviceSize size);

/**
 * Destroys the index buffer
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_index_buffer(VulkanFrameData *frame_data);

/**
 * Create the uniform buffer and sets it up.
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_uniform_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info);

/**
 * Destroys the uniform buffer
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_uniform_buffer(VulkanFrameData *frame_data);

/**
 * Creates a descriptor set
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool create_descriptor_set(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info);

/**
 * Destroys the descriptor set
 *
 * @param[in] `frame_data` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_descriptor_set(VulkanFrameData *frame_data);
