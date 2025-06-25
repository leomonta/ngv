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

bool copy_buffer(VulkanRuntimeInfo *vri, VkBuffer src, VkBuffer dst, VkDeviceSize size) {

	VkCommandBufferBeginInfo beg_info = {};
	beg_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beg_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(vri->transfer_cmd_buff, &beg_info);
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset    = 0;
		copyRegion.dstOffset    = 0;
		copyRegion.size         = size;
		vkCmdCopyBuffer(vri->transfer_cmd_buff, src, dst, 1, &copyRegion);
	}
	vkEndCommandBuffer(vri->transfer_cmd_buff);

	VkSubmitInfo submitInfo       = {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &vri->transfer_cmd_buff;

	vkQueueSubmit(vri->device_queues.transfer, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vri->device_queues.transfer);

	vkResetCommandBuffer(vri->transfer_cmd_buff, 0);

	return true;
}
