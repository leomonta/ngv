#include "vulkan_ops.h"

#include "logger.h"
#include "vkinit_utils.h"
#include "vulkan_memory.h"
#include "vulkan_setup.h"

static bool first_img_acquired = false;

bool record_cmd_buff(const NGVRendererSettings *settings, VulkanSetupInfo *setup_info, VulkanFrameData *frame_data) {
	VkCommandBufferBeginInfo beg_info = {};
	beg_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beg_info.flags                    = 0;       // Optional
	beg_info.pInheritanceInfo         = nullptr; // Optional

	auto res = vkBeginCommandBuffer(frame_data->cmd_buff[frame_data->current_image_index], &beg_info);
	{
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[COMMAND BUFFER] Could not begin recording the command buffer: %s\n", VkResult_str(res));
		}

		VkClearValue clear_colors[2] = {};
		clear_colors[0].color        = (VkClearColorValue){
		           .float32 = {0.0f, 0.0f, 0.0f, 1.0f},
                };
		clear_colors[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderPass            = setup_info->renderpass;
		renderpass_info.framebuffer           = setup_info->swapchain.framebuffers[frame_data->current_image_index];
		renderpass_info.renderArea.offset     = (VkOffset2D){0, 0};
		renderpass_info.renderArea.extent     = setup_info->swapchain.extent;
		renderpass_info.clearValueCount       = settings->accumulation_buffer ? 0 : 2;
		renderpass_info.pClearValues          = clear_colors;

		vkCmdBeginRenderPass(frame_data->cmd_buff[frame_data->current_image_index], &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(frame_data->cmd_buff[frame_data->current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, setup_info->pipeline.object);
			VkViewport viewport = {};
			viewport.x          = 0.0f;
			viewport.y          = 0.0f;
			viewport.width      = (float)(setup_info->swapchain.extent.width);
			viewport.height     = (float)(setup_info->swapchain.extent.height);
			viewport.minDepth   = 0.0f;
			viewport.maxDepth   = 1.0f;
			vkCmdSetViewport(frame_data->cmd_buff[frame_data->current_image_index], 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset   = (VkOffset2D){0, 0};
			scissor.extent   = setup_info->swapchain.extent;
			vkCmdSetScissor(frame_data->cmd_buff[frame_data->current_image_index], 0, 1, &scissor);

			VkBuffer     v_bufs[1]  = {frame_data->vertex_buff};
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(frame_data->cmd_buff[frame_data->current_image_index], 0, 1, v_bufs, offsets);

			vkCmdBindIndexBuffer(frame_data->cmd_buff[frame_data->current_image_index], frame_data->index_buff, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(frame_data->cmd_buff[frame_data->current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, setup_info->pipeline.layout, 0, 1, &frame_data->descriptor_sets[frame_data->current_image_index], 0, nullptr);

			vkCmdDrawIndexed(frame_data->cmd_buff[frame_data->current_image_index], frame_data->index_count, 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(frame_data->cmd_buff[frame_data->current_image_index]);
	}
	res = vkEndCommandBuffer(frame_data->cmd_buff[frame_data->current_image_index]);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAW CALL] Could not end the command buffer recording: %s\n", VkResult_str(res));
	}
	return true;
}

VkSemaphore get_image_available_semaphore(const NGVRendererSettings *settings, NGVRenderer *renderer) {
	auto temp = renderer->frame_data.image_acquisition_index;
	if (!settings->accumulation_buffer) {
		renderer->frame_data.image_acquisition_index = (renderer->frame_data.image_acquisition_index + 1) % MAX_CONCURRENT_FRAMES;
	}
	return renderer->frame_data.image_available[temp];
}

uint32_t get_next_image(NGVRenderer *renderer) {

	uint32_t img_index = 0;
	auto     res       = vkAcquireNextImageKHR(renderer->frame_data.logical_dev, renderer->setup_info.swapchain.swapchain, UINT64_MAX, renderer->frame_data.image_available[renderer->frame_data.image_acquisition_index], VK_NULL_HANDLE, &img_index);

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		llog(LOG_INFO, "[DRAWING] The swapchain is out of date, recreating it\n");
		re_create_swapchain(&renderer->setup_info, &renderer->static_info);
	} else if (res == VK_SUBOPTIMAL_KHR) {
		llog(LOG_INFO, "[DRAWING] The swapchain is suboptimal, doing nothing about it\n");
	} else if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[DRAWING] Image Acquisition from swapchain failed: %s\n", VkResult_str(res));
	}
	return img_index;
}

void draw_frame(const NGVRendererSettings *settings, NGVRenderer *renderer) {
	if (renderer->frame_data.current_image_index != UINT32_MAX) {
		vkWaitForFences(renderer->frame_data.logical_dev, 1, &renderer->frame_data.in_flight_fence[renderer->frame_data.current_image_index], VK_TRUE, UINT64_MAX);
	}

	VkSemaphore img_available = VK_NULL_HANDLE;

	if (!first_img_acquired || !settings->accumulation_buffer) {
		first_img_acquired                       = true;
		renderer->frame_data.current_image_index = get_next_image(renderer);
		img_available                            = get_image_available_semaphore(settings, renderer);
	}

	update_uniform_buffer(&renderer->setup_info, &renderer->frame_data.uniform_buff_mapped[renderer->frame_data.current_image_index]);

	vkResetFences(renderer->frame_data.logical_dev, 1, &renderer->frame_data.in_flight_fence[renderer->frame_data.current_image_index]);

	vkResetCommandBuffer(renderer->frame_data.cmd_buff[renderer->frame_data.current_image_index], 0);

	record_cmd_buff(settings, &renderer->setup_info, &renderer->frame_data);

	VkSemaphore          signal_sems[] = {renderer->frame_data.render_finished[renderer->frame_data.current_image_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info = {
	    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .waitSemaphoreCount   = img_available != VK_NULL_HANDLE,
	    .pWaitSemaphores      = &img_available,
	    .signalSemaphoreCount = 1,
	    .pSignalSemaphores    = signal_sems,
	    .pWaitDstStageMask    = wait_stages,
	    .commandBufferCount   = 1,
	    .pCommandBuffers      = &renderer->frame_data.cmd_buff[renderer->frame_data.current_image_index],
	};

	// in_flight_fence is signaled when the command submission is done
	auto res = vkQueueSubmit(renderer->setup_info.device_queues.graphics, 1, &submit_info, renderer->frame_data.in_flight_fence[renderer->frame_data.current_image_index]);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAWING] Could not submit command to queue: %s\n", VkResult_str(res));
	}

	VkSwapchainKHR swapchains[] = {renderer->setup_info.swapchain.swapchain};

	VkPresentInfoKHR present_info = {
	    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores    = signal_sems, // wait on the previously signaled semaphore
	    .swapchainCount     = 1,
	    .pSwapchains        = swapchains,
	    .pImageIndices      = &renderer->frame_data.current_image_index,
	    .pResults           = nullptr, // Optional
	};

	// may use VK_KHR_swapchain_maintenance1 to user a fence here for timing
	vkQueuePresentKHR(renderer->setup_info.device_queues.present, &present_info);
}

bool begin_temporary_command_buffer(VulkanSetupInfo *setup_info, QueueKind kind, VkCommandBuffer *command_buffer) {

	VkCommandPool cp;

	switch (kind) {

	case GRAPHIC_QUEUE:
		cp = setup_info->graphics_cmd_pool;
		break;

	case TRANSFER_QUEUE:
		cp = setup_info->transfer_cmd_pool;
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

	vkAllocateCommandBuffers(setup_info->logical_dev, &alloc_info, command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(*command_buffer, &begin_info);

	return true;
}

bool end_temporary_command_buffer(VulkanSetupInfo *setup_info, QueueKind kind, VkCommandBuffer command_buffer) {

	VkCommandPool cp;
	VkQueue       q;

	switch (kind) {

	case GRAPHIC_QUEUE:
		cp = setup_info->graphics_cmd_pool;
		q  = setup_info->device_queues.graphics;
		break;

	case TRANSFER_QUEUE:
		cp = setup_info->transfer_cmd_pool;
		q  = setup_info->device_queues.transfer;
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

	vkFreeCommandBuffers(setup_info->logical_dev, cp, 1, &command_buffer);

	return true;
}

void transition_image_layout(VulkanSetupInfo *setup_info, VkImage image, VkFormat format, VkImageLayout from_layout, VkImageLayout to_layout) {
	VkCommandBuffer cmd_buff;
	begin_temporary_command_buffer(setup_info, GRAPHIC_QUEUE, &cmd_buff);

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

	if (from_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

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

	} else if (from_layout == VK_IMAGE_LAYOUT_UNDEFINED && to_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage             = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage             = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	} else {
		llog(LOG_ERROR, "[IMAGE] Unsuppoerted layout transition.\n");
		return;
	}

	vkCmdPipelineBarrier(cmd_buff, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	end_temporary_command_buffer(setup_info, GRAPHIC_QUEUE, cmd_buff);
}

void copy_buffer_to_image(VulkanSetupInfo *setup_info, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer cmd_buff;
	begin_temporary_command_buffer(setup_info, GRAPHIC_QUEUE, &cmd_buff);

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

	end_temporary_command_buffer(setup_info, GRAPHIC_QUEUE, cmd_buff);
}
