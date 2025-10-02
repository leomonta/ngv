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

// small hack since the value is defined at compile time
typedef struct {
	VkCommandBuffer cmd_buff[MAX_CONCURRENT_FRAMES];
	VkSemaphore     image_available[MAX_CONCURRENT_FRAMES];
	VkSemaphore     render_finished[MAX_CONCURRENT_FRAMES];
	VkFence         in_flight_fence[MAX_CONCURRENT_FRAMES];
	VkDescriptorSet descriptor_sets[MAX_CONCURRENT_FRAMES];
	VkBuffer        uniform_buff[MAX_CONCURRENT_FRAMES];
	VkDeviceMemory  uniform_buff_mem[MAX_CONCURRENT_FRAMES];
	void           *uniform_buff_mapped[MAX_CONCURRENT_FRAMES];
} FrameData;

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

// static objects that will rarely be changed
typedef struct {
	GLFWwindow              *sys_window;
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkSurfaceKHR             surface;
	VkPhysicalDevice         physical_dev;
	VkDevice                 logical_dev;
} VulkanApplicationInfos;

// infrastructure that may be changed
typedef struct {
	VkRenderPass       renderpass;
	VkCommandPool      graphics_cmd_pool;
	VkCommandPool      transfer_cmd_pool;
	VkDescriptorPool   descriptor_pool;
	CreatedQueues      device_queues;
	QueuesIndicies     device_queues_indices;
	SwapchainInfo      swapchain;
	ShaderPipeline     pipeline;
	DepthBufferObjects depth_objects;
} VulkanRederingObjects;

// data that needs to be modified per call or quite often
typedef struct {
	VkBuffer        index_buff;
	VkDeviceMemory  index_buff_mem;
	VkBuffer        vertex_buff;
	VkDeviceMemory  vertex_buff_mem;
	VkCommandBuffer transfer_cmd_buff;
	FrameData       frame_data_objects;
	TextureData     textures;
} VulkanRenderingResources;

typedef struct {
	GLFWwindow              *sys_window;
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkSurfaceKHR             surface;
	VkPhysicalDevice         physical_dev;
	VkDevice                 logical_dev;
	VkRenderPass             renderpass;
	VkCommandPool            graphics_cmd_pool;
	VkCommandPool            transfer_cmd_pool;
	VkDescriptorPool         descriptor_pool;
	VkCommandBuffer          transfer_cmd_buff;
	VkBuffer                 index_buff;
	VkDeviceMemory           index_buff_mem;
	VkBuffer                 vertex_buff;
	VkDeviceMemory           vertex_buff_mem;
	DepthBufferObjects       depth_objects;
	FrameData                frame_data_objects;
	TextureData              textures;
	CreatedQueues            device_queues;
	QueuesIndicies           device_queues_indices;
	SwapchainInfo            swapchain;
	ShaderPipeline           pipeline;
} VulkanRuntimeInfo;

// ------------------------------------------------------------------------------------------------
// Configuration structs
// ------------------------------------------------------------------------------------------------

typedef struct {
	bool     use_preferred_device;
	uint32_t preferred_physical_device_id;
} VulkanStaticSettings;

typedef struct {
} VulkanSetupSettings;

typedef enum {
	TWO_DIM,
	THREE_DIM,
} RendereDimensions;

typedef struct {
	VulkanStaticSettings static_settings;
	VulkanSetupSettings  setup_settings;
} RendererSettings;

// ------------------------------------------------------------------------------------------------
// Static data, set and startup and done, referenced rarely
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
	VkDevice         logical_dev;
	VkRenderPass     renderpass;
	VkCommandPool    graphics_cmd_pool;
	VkCommandPool    transfer_cmd_pool;
	VkDescriptorPool descriptor_pool;
	VkCommandBuffer  transfer_cmd_buff;
	ShaderPipeline   pipeline;
	CreatedQueues    device_queues;
	QueuesIndicies   device_queues_indices;
	SwapchainInfo    swapchain;
} VulkanSetupInfo;

// ------------------------------------------------------------------------------------------------
// dynamic Frame data
// ------------------------------------------------------------------------------------------------

typedef struct {
	
	VkBuffer           index_buff;
	VkDeviceMemory     index_buff_mem;
	VkBuffer           vertex_buff;
	VkDeviceMemory     vertex_buff_mem;
	DepthBufferObjects depth_objects;
	FrameData          frame_data_objects;
	TextureData        textures;
} VulaknFrameData;
