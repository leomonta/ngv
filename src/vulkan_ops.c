#include "vulkan_ops.h"

#include "logger.h"
#include "shader.h"
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

		VkClearValue clear_colors[2] = {};
		clear_colors[0].color        = (VkClearColorValue){
		           .float32 = {0.0f, 0.0f, 0.0f, 1.0f}
        };
		clear_colors[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderPass            = vri->renderpass;
		renderpass_info.framebuffer           = vri->swapchain.framebuffers[img_index];
		renderpass_info.renderArea.offset     = (VkOffset2D){0, 0};
		renderpass_info.renderArea.extent     = vri->swapchain.extent;
		renderpass_info.clearValueCount       = 2;
		renderpass_info.pClearValues          = clear_colors;

		vkCmdBeginRenderPass(vri->frame_data_objects.cmd_buff[frame_index], &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
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

			VkBuffer     v_bufs[1]  = {vri->vertex_buff};
			VkDeviceSize offsets[1] = {0};
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
