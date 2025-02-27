#pragma once

#include "vulkan_initialization.h"

#include <GLFW/glfw3.h>
#define DEFAULT_WINDOW_HEIGHT 1080

#define DEFAULT_WINDOW_WIDTH  1920

/**
 * Initializes `glfw` and returns a newly created window
 *
 * @return the created window
 */
GLFWwindow *init_window();

/**
 * Terminate the window and `glfw`
 *
 * @param[in] `wndw` the window to destroy
 */
void terminate_window(GLFWwindow *wndw);

/**
 * Initializes all the vulkan components and populates the given `VulkanRuntimeInfo`
 *
 * @param[out] `vri` the vulkan context that is gonna be initialized
 */
void init_vulkan(VulkanRuntimeInfo *vri);

/**
 * Destroy everything created by `init_vulkan` on reverse order
 *
 * @param[in] `vri` the vulkan context to terminate
 */
void terminate_vulkan(VulkanRuntimeInfo *vri);
