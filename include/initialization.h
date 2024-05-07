#pragma once

#include "vulkan_intialization.h"

#include <GLFW/glfw3.h>

#define DEFAULT_WINDOW_HEIGHT 1080
#define DEFAULT_WINDOW_WIDTH  1920

/**
 * Initializes glfw and returns a newly created window
 *
 * @returns the create window
 */
GLFWwindow *init_window();

/**
 * Terminate the window and glfw
 *
 * @param[in] the window to destroy
 */
void terminate_window(GLFWwindow *wndw);

void init_vulkan(VulkanRuntimeInfo *vri);
void terminate_vulkan(VulkanRuntimeInfo *vri);
