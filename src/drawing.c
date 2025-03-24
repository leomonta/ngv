#include "drawing.h"

#include "logger.h"
#include "vkinit_utils.h"

bool record_cmd_buff(VulkanRuntimeInfo *vri, uint32_t img_index) {
	VkCommandBufferBeginInfo beg_info = {0};
	beg_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beg_info.flags                    = 0;       // Optional
	beg_info.pInheritanceInfo         = nullptr; // Optional

	auto res = vkBeginCommandBuffer(vri->cmd_buffer, &beg_info);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND BUFFER] Could not begin recording the command buffer: %s\n", VkResult_str(res));
	}

	VkClearValue          clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	VkRenderPassBeginInfo rp_info    = {0};
	rp_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_info.renderPass               = vri->renderpass;
	rp_info.framebuffer              = vri->swapchain.framebuffers[img_index];
	rp_info.renderArea.offset        = (VkOffset2D){0, 0};
	rp_info.renderArea.extent        = vri->swapchain.extent;
	rp_info.clearValueCount          = 1;
	rp_info.pClearValues             = &clearColor;
	vkCmdBeginRenderPass(vri->cmd_buffer, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(vri->cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vri->pipeline.pipeline);
	VkViewport viewport = {0};
	viewport.x          = 0.0f;
	viewport.y          = 0.0f;
	viewport.width      = (float)(vri->swapchain.extent.width);
	viewport.height     = (float)(vri->swapchain.extent.height);
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;
	vkCmdSetViewport(vri->cmd_buffer, 0, 1, &viewport);

	VkRect2D scissor = {0};
	scissor.offset   = (VkOffset2D){0, 0};
	scissor.extent   = vri->swapchain.extent;
	vkCmdSetScissor(vri->cmd_buffer, 0, 1, &scissor);

	vkCmdDraw(vri->cmd_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(vri->cmd_buffer);

	res = vkEndCommandBuffer(vri->cmd_buffer);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAW CALL] Could not end the command buffer recording: %s\n", VkResult_str(res));
	}
	return true;
}

void draw_frame(VulkanRuntimeInfo *vri) {
	vkWaitForFences(vri->logical_dev, 1, &vri->in_flight_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(vri->logical_dev, 1, &vri->in_flight_fence);

	uint32_t img_index;
	vkAcquireNextImageKHR(vri->logical_dev, vri->swapchain.swapchain, UINT64_MAX, vri->image_available, VK_NULL_HANDLE, &img_index);

	vkResetCommandBuffer(vri->cmd_buffer, 0);

	record_cmd_buff(vri, img_index);

	VkSemaphore          signal_sems[] = {vri->render_finished};
	VkSemaphore          wait_sems[]   = {vri->image_available};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo         submit_info   = {0};
	submit_info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount     = 1;
	submit_info.pWaitSemaphores        = wait_sems;
	submit_info.pWaitDstStageMask      = wait_stages;
	submit_info.commandBufferCount     = 1;
	submit_info.pCommandBuffers        = &vri->cmd_buffer;
	submit_info.signalSemaphoreCount   = 1;
	submit_info.pSignalSemaphores      = signal_sems;

	auto res = vkQueueSubmit(vri->device_queues.graphics, 1, &submit_info, vri->in_flight_fence);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DRAWING] Could not submit command to queue: %s\n", VkResult_str(res));
	}

	VkPresentInfoKHR present_info   = {0};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = signal_sems;

	VkSwapchainKHR swapchains[] = {vri->swapchain.swapchain};
	present_info.swapchainCount  = 1;
	present_info.pSwapchains     = swapchains;
	present_info.pImageIndices   = &img_index;
	present_info.pResults        = nullptr; // Optional

	vkQueuePresentKHR(vri->device_queues.present, &present_info);
}
