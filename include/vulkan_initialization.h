#pragma once

#include "config.h"
#include "utils.h"

#include <stddef.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>

typedef struct {
	uint32_t graphics;
	uint32_t compute;        // unused
	uint32_t transfer;       // unused
	uint32_t sparse_binding; // unused
	uint32_t present;

	bitfield available_families;
} QueueFamilyIndicies;

enum : char {
	GRAPHIC_QUEUE_INDEX,
	COMPUTE_QUEUE_INDEX,
	TRANSFER_QUEUE_INDEX,
	SPARSE_BINDING_QUEUE_INDEX,
	PRESENT_QUEUE_INDEX,
	QUEUE_ENUM_COUNT,
};

typedef enum : char {
	VERTEX_SHADER,
	TESSELATION_SHADER,
	GEOMETRY_SHADER,
	FRAGMENT_SHADER,
	COMPUTE_SHADER,
} ShaderKind;

typedef struct {
	VkQueue graphics;
	VkQueue compute;        // unused
	VkQueue transfer;       // unused
	VkQueue sparse_binding; // unused
	VkQueue present;
} QueuesInfo;

#define USED_QUEUE_FAMILIES sizeof(QueueFamilyIndicies) % sizeof(uint32_t) - 1;

typedef struct {
	VkSwapchainKHR swapchain;
	uint32_t       buffers_count;
	VkImage       *buffers;
	VkImageView   *views;
	VkFramebuffer *framebuffers;
	VkExtent2D     extent;
	VkFormat       format;
} SwapchainInfo;

typedef struct {
	VkPipelineLayout layout;
	VkPipeline       pipeline;
} ShaderPipeline;

// small hack since the value is defined at compile time
typedef struct {
	VkCommandBuffer cmd_buffer[MAX_CONCURRENT_FRAMES];
	VkSemaphore     image_available[MAX_CONCURRENT_FRAMES];
	VkSemaphore     render_finished[MAX_CONCURRENT_FRAMES];
	VkFence         in_flight_fence[MAX_CONCURRENT_FRAMES];
} CmdBufferAndCo;

typedef struct {
	GLFWwindow              *sys_window;
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkSurfaceKHR             surface;
	VkPhysicalDevice         physical_dev;
	VkDevice                 logical_dev;
	VkRenderPass             renderpass;
	VkCommandPool            cmd_pool;
	VkBuffer                 vertex_buffer;
	VkDeviceMemory           vertex_buffer_memory;
	CmdBufferAndCo           cmd_buff_mngn;
	QueuesInfo               device_queues;
	SwapchainInfo            swapchain;
	ShaderPipeline           pipeline;
} VulkanRuntimeInfo;

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR      *formats;
	size_t                   formats_count;
	VkPresentModeKHR        *modes;
	size_t                   modes_count;
} SwapchainDetails;

/**
 * returns if any requested validation layer is available
 *
 * @return `true` if there are available validation layers, `false` otherwise
 */
bool check_validation_layer_support();

/**
 * Creates a `VkInstance`
 *
 * @param[out] `vri` the vulkan context where to put the created instance
 *
 * @return if the operation was successfull or not
 */
bool create_instance(VulkanRuntimeInfo *vri);

/**
 * Destroy a `VkInstance` and its associated data
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_instance(VulkanRuntimeInfo *vri);

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
 * Destroys the vertex buffer and sets it up with the default vertex layout
 *
 * @param[in] `vri` the vulkan context to use
 *
 * @return if the operation was successfull or not
 */
bool destroy_vertex_buffer(VulkanRuntimeInfo *vri);
