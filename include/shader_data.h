#pragma once

#include <cglm/cglm.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>

typedef struct {
	vec2 position;
	vec3 color;
	vec2 texture_coordinates;
} Vertex;

// how many fields there are in the vertex struct
constexpr static size_t Vertex_attributes_num = 3;

static const VkVertexInputBindingDescription Vertex_layout = {
    .binding   = 0,
    .stride    = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

static const VkVertexInputAttributeDescription Vertex_attribs[Vertex_attributes_num] = {
    {
     .binding  = 0,
     .location = 0,
     .format   = VK_FORMAT_R32G32_SFLOAT,
     .offset   = offsetof(Vertex, position),
     },
    {
     .binding  = 0,
     .location = 1,
     .format   = VK_FORMAT_R32G32B32_SFLOAT,
     .offset   = offsetof(Vertex,               color),
     },
    {
     .binding  = 0,
     .location = 2,
     .format   = VK_FORMAT_R32G32_SFLOAT,
     .offset   = offsetof(Vertex,                                 texture_coordinates),
     }
};

static const Vertex __temp__data[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
};

static const uint32_t __temp__indicies[] = {
    0, 1, 2, 0, 3, 1};
