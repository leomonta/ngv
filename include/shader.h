#pragma once

#include "cglm_proxy.h"
#include "ngv_objects.h"

#include <shaderc/shaderc.h>
#include <stddef.h>
#include <vulkan/vulkan_core.h>

typedef struct {
	vec3 position;
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
     .format   = VK_FORMAT_R32G32B32_SFLOAT,
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
    {{-0.5f, -0.5f, 0.0f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f},    {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f},   {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.0f},   {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
};

static const uint32_t __temp__indicies[] = {
    0,
    1,
    2,
    0,
    3,
    1,
    4,
    5,
    6,
    4,
    7,
    5,
};

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
 * create a shader module 
 *
 * @param[in] `filename` the filename of the shader code
 * @param[in] `shader_kind` the kind of shader we are dealing with
 * @param[in] `logical_dev` the logical device to refer this 
 * @param[out] `module` a valid handle to a `VkShaderModule`
 * @param[out] `shaderc_result` the `shaderc` result handle that needs to be closed later
 *
 * @return if the operation was successfull
 */
bool create_shader_module(const char *filename, const ShaderKind shader_kind, const VkDevice logical_dev, VkShaderModule *module, shaderc_compilation_result_t *shaderc_result);

bool destroy_shader_module(const VkDevice logical_dev, shaderc_compilation_result_t shaderc_result, VkShaderModule shader_module);
