#include "vulkan_ops.h"

#include "logger.h"
#include "shader_data.h"
#include "vkinit_utils.h"
#include "vulkan_memory.h"

static uint32_t frame_index = 0;

bool record_cmd_buff(VulkanRuntimeInfo *vri, uint32_t img_index) {
	VkCommandBufferBeginInfo beg_info = {};
	beg_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beg_info.flags                    = 0;       // Optional
	beg_info.pInheritanceInfo         = nullptr; // Optional

	auto res = vkBeginCommandBuffer(vri->frame_data_objects.cmd_buff[frame_index], &beg_info);
	{
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[COMMAND BUFFER] Could not begin recording the command buffer: %s\n", VkResult_str(res));
		}

		VkClearValue          clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		VkRenderPassBeginInfo rp_info    = {};
		rp_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_info.renderPass               = vri->renderpass;
		rp_info.framebuffer              = vri->swapchain.framebuffers[img_index];
		rp_info.renderArea.offset        = (VkOffset2D){0, 0};
		rp_info.renderArea.extent        = vri->swapchain.extent;
		rp_info.clearValueCount          = 1;
		rp_info.pClearValues             = &clearColor;

		vkCmdBeginRenderPass(vri->frame_data_objects.cmd_buff[frame_index], &rp_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(vri->frame_data_objects.cmd_buff[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vri->pipeline.object);
			VkViewport viewport = {};
			viewport.x          = 0.0f;
			viewport.y          = 0.0f;
			viewport.width      = (float)(vri->swapchain.extent.width);
			viewport.height     = (float)(vri->swapchain.extent.height);
			viewport.minDepth   = 0.0f;
			viewport.maxDepth   = 1.0f;
			vkCmdSetViewport(vri->frame_data_objects.cmd_buff[frame_index], 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset   = (VkOffset2D){0, 0};
			scissor.extent   = vri->swapchain.extent;
			vkCmdSetScissor(vri->frame_data_objects.cmd_buff[frame_index], 0, 1, &scissor);

			VkBuffer     v_bufs[]  = {vri->vertex_buff};
			VkDeviceSize offsets[] = {};
			vkCmdBindVertexBuffers(vri->frame_data_objects.cmd_buff[frame_index], 0, 1, v_bufs, offsets);

			vkCmdBindIndexBuffer(vri->frame_data_objects.cmd_buff[frame_index], vri->index_buff, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(vri->frame_data_objects.cmd_buff[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, vri->pipeline.layout, 0, 1, &vri->frame_data_objects.descriptor_sets[frame_index], 0, nullptr);

			vkCmdDrawIndexed(vri->frame_data_objects.cmd_buff[frame_index], sizeof(__temp__indicies) / sizeof(__temp__indicies[0]), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(vri->frame_data_objects.cmd_buff[frame_index]);
	}
	res = vkEndCommandBuffer(vri->frame_data_objects.cmd_buff[frame_index]);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAW CALL] Could not end the command buffer recording: %s\n", VkResult_str(res));
	}
	return true;
}

void draw_frame(VulkanRuntimeInfo *vri) {
	vkWaitForFences(vri->logical_dev, 1, &vri->frame_data_objects.in_flight_fence[frame_index], VK_TRUE, UINT64_MAX);

	uint32_t img_index;
	auto     res = vkAcquireNextImageKHR(vri->logical_dev, vri->swapchain.swapchain, UINT64_MAX, vri->frame_data_objects.image_available[frame_index], VK_NULL_HANDLE, &img_index);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		llog(LOG_INFO, "[DRAWING] The swapchain is out of date, recreating it\n");
		re_create_swapchain(vri);
		return;
	} else if (res == VK_SUBOPTIMAL_KHR) {
		llog(LOG_INFO, "[DRAWING] The swapchain is suboptimal, doing nothing about it\n");
	} else if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[DRAWING] Image Acquisition from swapchain failed: %s\n", VkResult_str(res));
	}

	update_uniform_buffer(vri, frame_index);

	vkResetFences(vri->logical_dev, 1, &vri->frame_data_objects.in_flight_fence[frame_index]);

	vkResetCommandBuffer(vri->frame_data_objects.cmd_buff[frame_index], 0);

	record_cmd_buff(vri, img_index);

	VkSemaphore          signal_sems[] = {vri->frame_data_objects.render_finished[frame_index]};
	VkSemaphore          wait_sems[]   = {vri->frame_data_objects.image_available[frame_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo         submit_info   = {};
	submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount     = 1;
	submit_info.pWaitSemaphores        = wait_sems;
	submit_info.signalSemaphoreCount   = 1;
	submit_info.pSignalSemaphores      = signal_sems;
	submit_info.pWaitDstStageMask      = wait_stages;
	submit_info.commandBufferCount     = 1;
	submit_info.pCommandBuffers        = &vri->frame_data_objects.cmd_buff[frame_index];

	res = vkQueueSubmit(vri->device_queues.graphics, 1, &submit_info, vri->frame_data_objects.in_flight_fence[frame_index]);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAWING] Could not submit command to queue: %s\n", VkResult_str(res));
	}

	VkSwapchainKHR   swapchains[]   = {vri->swapchain.swapchain};
	VkPresentInfoKHR present_info   = {};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = signal_sems;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = swapchains;
	present_info.pImageIndices      = &img_index;
	present_info.pResults           = nullptr; // Optional

	vkQueuePresentKHR(vri->device_queues.present, &present_info);

	frame_index = (frame_index + 1) % MAX_CONCURRENT_FRAMES;
}

bool begin_temporary_command_buffer(VulkanRuntimeInfo *vri, QueueKind kind, VkCommandBuffer *command_buffer) {

	VkCommandPool cp;

	switch (kind) {

	case GRAPHIC_QUEUE:
		cp = vri->graphics_cmd_pool;
		break;

	case TRANSFER_QUEUE:
		cp = vri->transfer_cmd_pool;
		break;

	case COMPUTE_QUEUE:
	case SPARSE_BINDING_QUEUE:
	case PRESENT_QUEUE:
	case QUEUE_ENUM_COUNT:
		return false;
		break;
	}

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool                 = cp;
	alloc_info.commandBufferCount          = 1;

	vkAllocateCommandBuffers(vri->logical_dev, &alloc_info, command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(*command_buffer, &begin_info);

	return true;
}

bool end_temporary_command_buffer(VulkanRuntimeInfo *vri, QueueKind kind, VkCommandBuffer command_buffer) {

	VkCommandPool cp;
	VkQueue       q;

	switch (kind) {

	case GRAPHIC_QUEUE:
		cp = vri->graphics_cmd_pool;
		q  = vri->device_queues.graphics;
		break;

	case TRANSFER_QUEUE:
		cp = vri->transfer_cmd_pool;
		q  = vri->device_queues.transfer;
		break;

	case COMPUTE_QUEUE:
	case SPARSE_BINDING_QUEUE:
	case PRESENT_QUEUE:
	case QUEUE_ENUM_COUNT:
		return false;
		break;
	}

	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info       = {};
	submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &command_buffer;

	vkQueueSubmit(q, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(q);

	vkFreeCommandBuffers(vri->logical_dev, cp, 1, &command_buffer);

	return true;
}

void transition_image_layout(VulkanRuntimeInfo *vri, VkImage image, VkFormat format, VkImageLayout from_layout, VkImageLayout to_layout) {
	VkCommandBuffer cmd_buff;
	begin_temporary_command_buffer(vri, GRAPHIC_QUEUE, &cmd_buff);

	VkImageMemoryBarrier barrier            = {};
	barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout                       = from_layout;
	barrier.newLayout                       = to_layout;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;

	VkPipelineStageFlags src_stage = {};
	VkPipelineStageFlags dst_stage = {};

	if (from_layout == VK_IMAGE_LAYOUT_UNDEFINED && to_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage             = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage             = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (from_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && to_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src_stage             = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage             = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		llog(LOG_ERROR, "[IMAGE] Unsuppoerted layout transition.\n");
		return;
	}

	vkCmdPipelineBarrier(cmd_buff, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	end_temporary_command_buffer(vri, GRAPHIC_QUEUE, cmd_buff);

}

void copy_buffer_to_image(VulkanRuntimeInfo *vri, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer cmd_buff;
	begin_temporary_command_buffer(vri, GRAPHIC_QUEUE, &cmd_buff);

	VkBufferImageCopy region               = {};
	region.bufferOffset                    = 0;
	region.bufferRowLength                 = 0;
	region.bufferImageHeight               = 0;
	region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel       = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount     = 1;
	region.imageOffset                     = (VkOffset3D){0, 0, 0};
	region.imageExtent                     = (VkExtent3D){width, height, 1};

	vkCmdCopyBufferToImage(cmd_buff, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	end_temporary_command_buffer(vri, GRAPHIC_QUEUE, cmd_buff);
}
