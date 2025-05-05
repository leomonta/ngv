#pragma once

#include <vulkan/vulkan.h>
/**
 * Vulkan callback definition
 * This will be hooked to the internal logger
 */
VKAPI_ATTR VkBool32 VKAPI_CALL logger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
