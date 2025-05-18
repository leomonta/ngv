#pragma once

#include <cglm/cglm.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>

typedef struct {
	vec2 position;
	vec3 color;
} Vertex;

// how many fields there are in the vertex struct
constexpr static size_t Vertex_attributes_num = 2;

const static VkVertexInputBindingDescription Vertex_layout = {
    .binding   = 0,
    .stride    = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

const static VkVertexInputAttributeDescription Vertex_attribs[Vertex_attributes_num] = {
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
     }
};

const static Vertex __temp__data[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
