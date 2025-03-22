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
