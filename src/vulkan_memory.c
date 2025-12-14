#include "vulkan_memory.h"

#include "cglm_proxy.h"
#include "logger.h"
#include "vulkan/vulkan_core.h"

#include <string.h>

uint32_t find_memory_type(const uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical_dev) {

	VkPhysicalDeviceMemoryProperties phy_props;
	vkGetPhysicalDeviceMemoryProperties(physical_dev, &phy_props);

	for (uint32_t i = 0; i < phy_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (phy_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	llog(LOG_FATAL, "[VMEM] Failed to find a suitable memory type\n");
	return (uint32_t)(-1);
}

bool copy_buffer(VulkanSetupInfo *setup_info, VkBuffer src, VkBuffer dst, VkDeviceSize size) {

	VkCommandBufferBeginInfo beg_info = {};
	beg_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beg_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(setup_info->transfer_cmd_buff, &beg_info);
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset    = 0;
		copyRegion.dstOffset    = 0;
		copyRegion.size         = size;
		vkCmdCopyBuffer(setup_info->transfer_cmd_buff, src, dst, 1, &copyRegion);
	}
	vkEndCommandBuffer(setup_info->transfer_cmd_buff);

	VkSubmitInfo submitInfo       = {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &setup_info->transfer_cmd_buff;

	vkQueueSubmit(setup_info->device_queues.transfer, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(setup_info->device_queues.transfer);

	vkResetCommandBuffer(setup_info->transfer_cmd_buff, 0);

	return true;
}

bool update_uniform_buffer(VulkanSetupInfo *setup_info, void *raw_data) {

	MVP mvp = {
	    GLM_MAT4_IDENTITY_INIT,
	    GLM_MAT4_IDENTITY_INIT,
	    GLM_MAT4_IDENTITY_INIT,
	};

	glm_mat4_copy(mvp.model, GLM_MAT4_IDENTITY);
	glm_rotate(mvp.model, glm_rad(1.f), ((vec3){0.0f, 0.0f, 1.0f}));

	glm_lookat((float[]){2.0f, 2.0f, 2.0f}, (float[]){0.0f, 0.0f, 0.0f}, (float[]){0.0f, 0.0f, 1.0f}, mvp.view);

	glm_perspective(glm_rad(100.f), (float)setup_info->swapchain.extent.width / (float)setup_info->swapchain.extent.height, 0.1f, 4.0f, mvp.proj);

	mvp.proj[1][1] *= -1;

	memcpy(raw_data, &mvp, sizeof(mvp));

	return true;
}

bool push_to_buffer(VulkanSetupInfo *setup_info, const VulkanFrameData *frame_data, VkBuffer buff, const void *pushed_data, const VkDeviceSize size) {

	void *data;
	vkMapMemory(frame_data->logical_dev, frame_data->staging_buff_mem, 0, size, 0, &data);
	memcpy(data, pushed_data, size);
	vkUnmapMemory(frame_data->logical_dev, frame_data->staging_buff_mem);

	copy_buffer(setup_info, frame_data->staging_buff, buff, size);

	return true;
}
