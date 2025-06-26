#pragma once

#include "config.h"
#include "utils.h"

#include <cglm/cglm.h>
#include <stddef.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.h>
#include <vulkan/vulkan_core.h>

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
	VkQueue compute; // unused
	VkQueue transfer;
	VkQueue sparse_binding; // unused
	VkQueue present;
} QueuesInfo;

typedef struct {
	uint32_t graphics;
	uint32_t compute; // unused
	uint32_t transfer;
	uint32_t sparse_binding; // unused
	uint32_t present;

	bitfield available_families;
} QueueFamilyIndicies;

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
} GraphicsCmdSynchro;

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
	GLFWwindow              *sys_window;
	VkInstance               instance;
	VkDebugUtilsMessengerEXT debug_logger;
	VkSurfaceKHR             surface;
	VkPhysicalDevice         physical_dev;
	VkDevice                 logical_dev;
	VkRenderPass             renderpass;
	VkCommandPool            graphics_cmd_pool;
	VkCommandPool            transfer_cmd_pool;
	VkCommandBuffer          transfer_cmd_buff;
	VkBuffer                 index_buffer;
	VkDeviceMemory           index_buffer_memory;
	VkBuffer                 vertex_buffer;
	VkDeviceMemory           vertex_buffer_memory;
	GraphicsCmdSynchro       graphic_cmd_synchro;
	QueuesInfo               device_queues;
	QueueFamilyIndicies      device_queues_indices;
	SwapchainInfo            swapchain;
	ShaderPipeline           pipeline;
	MVP                      mvp_matrix;
} VulkanRuntimeInfo;
