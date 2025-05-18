#include "vulkan_memory.h"

#include "logger.h"

#include "vulkan/vulkan_core.h"
uint32_t find_memory_type(const uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanRuntimeInfo *vri) {

	VkPhysicalDeviceMemoryProperties phy_props;
	vkGetPhysicalDeviceMemoryProperties(vri->physical_dev, &phy_props);

	for (uint32_t i = 0; i < phy_props.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (phy_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	llog(LOG_FATAL, "[VMEM] Failed to find a suitable memory type\n");
	return (uint32_t)(-1);

}
