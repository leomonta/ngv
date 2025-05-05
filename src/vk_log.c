#include "vk_log.h"
#include "logger.h"

#include <stdio.h>

VKAPI_ATTR VkBool32 VKAPI_CALL logger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, 	__attribute__((unused)) void *pUserData) {

	logLevel ll = LOG_DEBUG;

	switch (messageSeverity) {

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		ll = LOG_DEBUG;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		ll = LOG_INFO;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		ll = LOG_WARNING;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ll = LOG_ERROR;
		break;
	default:
		ll = LOG_DEBUG;
		break;
	}

	const char *context;

	switch (messageType) {
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		context = "General";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		context = "Validation";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		context = "Performance";
		break;
	default:
		context = "Unknown";
	}

	logger(ll, "Vulkan", 0, context, pCallbackData->pMessage);
	printf("\n"); //FIXME: This is not the best thing ever, oH well

	return VK_FALSE;
}
