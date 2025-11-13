#pragma once

#include "cglm_proxy.h"
#include "config.h"
#include "utils.h"

#include <stddef.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>

// ------------------------------------------------------------------------------------------------
// internal Enums
// ------------------------------------------------------------------------------------------------

typedef enum : char {
	GRAPHIC_QUEUE,
	COMPUTE_QUEUE,
	TRANSFER_QUEUE,
	SPARSE_BINDING_QUEUE,
	PRESENT_QUEUE,
	QUEUE_ENUM_COUNT,
} QueueKind;

typedef enum : char {
	VERTEX_SHADER,
	TESSELATION_SHADER,
	GEOMETRY_SHADER,
	FRAGMENT_SHADER,
	COMPUTE_SHADER,
} ShaderKind;

// ------------------------------------------------------------------------------------------------
// Simple structs
// ------------------------------------------------------------------------------------------------

typedef struct {
	VkQueue graphics;
	VkQueue compute; // unused
	VkQueue transfer;
	VkQueue sparse_binding; // unused
	VkQueue present;
} CreatedQueues;

typedef struct {
	uint32_t graphics;
	uint32_t compute; // unused
	uint32_t transfer;
	uint32_t sparse_binding; // unused
	uint32_t present;

	bitfield available_families;
} QueuesIndicies;

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
	VkPipelineLayout      layout;
	VkPipeline            object;
	VkDescriptorSetLayout descriptor_set_layout;
} ShaderPipeline;

typedef struct {
	size_t         count;
	VkImage        objects[TEMP_ARRAY_SIZE];
	VkDeviceMemory memory[TEMP_ARRAY_SIZE];
	VkImageView    views[TEMP_ARRAY_SIZE];    // need something more flexible than one view per texture
	VkSampler      samplers[TEMP_ARRAY_SIZE]; // need something more flexible than one view per texture
} TextureData;

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR      *formats;
	size_t                   formats_count;
	VkPresentModeKHR        *modes;
	size_t                   modes_count;
} SwapchainDetails;

typedef struct {
	mat4 model;
	mat4 view;
	mat4 proj;
} MVP;

typedef struct {
	VkImage        image;
	VkDeviceMemory memory;
	VkImageView    view;
} DepthBufferObjects;

// ------------------------------------------------------------------------------------------------
// Configuration structs
// ------------------------------------------------------------------------------------------------

typedef enum : char {
	TWO_DIM,
	THREE_DIM,
} RendererDimensions;

typedef struct {
	bool               use_preferred_device;
	RendererDimensions dimensions;
	bool               accumulation_buffer;
	uint32_t           preferred_physical_device_id;
	int                window_width;
	int                window_height;
	char              *window_name;
} NGVRendererSettings;

// ------------------------------------------------------------------------------------------------
// Static data, set at startup and done, referenced rarely
// ------------------------------------------------------------------------------------------------

typedef struct {
	GLFWwindow      *system_window;
	VkInstance       vulkan_instance;
	VkSurfaceKHR     surface;
	VkPhysicalDevice physical_dev;
} VulkanStaticInfo;

// ------------------------------------------------------------------------------------------------
// Instance specification, pipeline and pieline related data that will rarely change but will be often read
// ------------------------------------------------------------------------------------------------

typedef struct {
	VkDevice           logical_dev;
	VkRenderPass       renderpass;
	VkCommandPool      graphics_cmd_pool;
	VkCommandPool      transfer_cmd_pool;
	VkDescriptorPool   descriptor_pool;
	VkCommandBuffer    transfer_cmd_buff;
	DepthBufferObjects depth_objects;
	ShaderPipeline     pipeline;
	CreatedQueues      device_queues;
	QueuesIndicies     device_queues_indices;
	SwapchainInfo      swapchain;
} VulkanSetupInfo;

// ------------------------------------------------------------------------------------------------
// dynamic Frame data
// ------------------------------------------------------------------------------------------------

typedef struct {
	VkPhysicalDevice physical_dev; // the same present in static info, just a shortcut
	VkDevice         logical_dev;  // the same present in setup info, just a shortcut
	uint32_t         index_count;
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
