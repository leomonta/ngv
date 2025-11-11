#include "vulkan_frame.h"

#include "logger.h"
#include "shader.h"
#include "vkinit_utils.h"
#include "vulkan_memory.h"

#include <stb/stb_image.h>
#include <string.h>
#include <errno.h>

/*
typedef struct {
    VkPhysicalDevice physical_dev; // the same present in static info, just a shortcut
    VkDevice         logical_dev;  // the same present in setup info, just a shortcut
    VkBuffer         index_buff;
    VkDeviceMemory   index_buff_mem;
    VkBuffer         vertex_buff;
    VkDeviceMemory   vertex_buff_mem;
    VkCommandBuffer  cmd_buff[MAX_CONCURRENT_FRAMES];
    VkSemaphore      image_available[MAX_CONCURRENT_FRAMES];
    VkSemaphore      render_finished[MAX_CONCURRENT_FRAMES];
    VkFence          in_flight_fence[MAX_CONCURRENT_FRAMES];
    VkDescriptorSet  descriptor_sets[MAX_CONCURRENT_FRAMES];
    VkBuffer         uniform_buff[MAX_CONCURRENT_FRAMES];
    VkDeviceMemory   uniform_buff_mem[MAX_CONCURRENT_FRAMES];
    void            *uniform_buff_mapped[MAX_CONCURRENT_FRAMES];
    FrameData        frame_data_objects;
    TextureData      textures;
} VulkanFrameData;
*/

bool create_frame_data(const NGVRendererSettings *settings, VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {
	if (!create_texture_image(setup_info, frame_data)) {
		return false;
	}
	if (!create_texture_view(0, frame_data)) {
		return false;
	}
	if (!create_texture_sampler(frame_data, 0)) {
		return false;
	}
	if (!create_vertex_buffer(frame_data, setup_info)) {
		return false;
	}
	if (!create_index_buffer(frame_data, setup_info)) {
		return false;
	}
	if (!create_uniform_buffer(frame_data, setup_info)) {
		return false;
	}
	if (!create_descriptor_set(frame_data, setup_info)) {
		return false;
	}
	if (!create_sync_objects(frame_data, setup_info)) {
		return false;
	}
	return true;
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

bool create_texture_image(VulkanSetupInfo *setup_info, VulkanFrameData *frame_data) {
	int          tex_width, tex_height, tex_channels;
	stbi_uc     *pixels   = stbi_load("this_is_snake.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
	VkDeviceSize image_sz = (unsigned long)tex_width * (unsigned long)tex_height * 4;

	if (!pixels) {
		llog(LOG_FATAL, "[TEXTURE] Failed to load texture image.");
	}

	VkBuffer       staging;
	VkDeviceMemory staging_mem;

	create_buffer(setup_info, frame_data->physical_dev, image_sz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, &staging_mem);

	void *data;
	vkMapMemory(setup_info->logical_dev, staging_mem, 0, image_sz, 0, &data);
	memcpy(data, pixels, image_sz);
	vkUnmapMemory(setup_info->logical_dev, staging_mem);

	stbi_image_free(pixels);

	create_image(setup_info->logical_dev, frame_data->physical_dev, (uint32_t)tex_width, (uint32_t)tex_height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &frame_data->textures.objects[0], &frame_data->textures.memory[0]);

	transition_image_layout(setup_info, frame_data->textures.objects[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copy_buffer_to_image(setup_info, staging, frame_data->textures.objects[0], (uint32_t)tex_width, (uint32_t)tex_height);
	transition_image_layout(setup_info, frame_data->textures.objects[0], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	llog(LOG_DEBUG, "[TEXTURE] Command pools successfully created\n");

	vkDestroyBuffer(setup_info->logical_dev, staging, nullptr);
	vkFreeMemory(setup_info->logical_dev, staging_mem, nullptr);

	return true;
}

bool destroy_texture_image(VulkanFrameData *frame_data) {

	for (size_t i = 0; i < frame_data->textures.count; ++i) {
		vkDestroyImage(frame_data->logical_dev, frame_data->textures.objects[i], nullptr);
		vkFreeMemory(frame_data->logical_dev, frame_data->textures.memory[i], nullptr);
	}

	llog(LOG_DEBUG, "[TEXTURE] Command pools successfully destroyed\n");
	return true;
}

bool create_texture_view(const uint32_t index, VulkanFrameData *frame_data) {

	VkImageViewCreateInfo vw_create           = {};
	vw_create.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vw_create.image                           = frame_data->textures.objects[index];
	vw_create.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	vw_create.format                          = VK_FORMAT_R8G8B8A8_SRGB;
	vw_create.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	vw_create.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	vw_create.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	vw_create.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	vw_create.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	vw_create.subresourceRange.baseMipLevel   = 0;
	vw_create.subresourceRange.levelCount     = 1;
	vw_create.subresourceRange.baseArrayLayer = 0;
	vw_create.subresourceRange.layerCount     = 1;

	auto res = vkCreateImageView(frame_data->logical_dev, &vw_create, nullptr, &frame_data->textures.views[index]);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[TEXTURE] Could not create texture views: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[TEXTURE] Image views successfully created\n");

	return true;
}

bool destroy_texture_view(const uint32_t index, VulkanFrameData *frame_data) {

	vkDestroyImageView(frame_data->logical_dev, frame_data->textures.views[index], nullptr);

	return true;
}

bool create_texture_sampler(VulkanFrameData *frame_data, uint32_t index) {

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(frame_data->physical_dev, &properties);

	VkSamplerCreateInfo samplerInfo     = {};
	samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter               = VK_FILTER_LINEAR;
	samplerInfo.minFilter               = VK_FILTER_LINEAR;
	samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.anisotropyEnable        = VK_TRUE;
	samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable           = VK_FALSE;
	samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias              = 0.0f;
	samplerInfo.minLod                  = 0.0f;
	samplerInfo.maxLod                  = 0.0f;

	auto res = vkCreateSampler(frame_data->logical_dev, &samplerInfo, nullptr, &frame_data->textures.samplers[index]);
	if (res != VK_SUCCESS) {
		llog(LOG_ERROR, "[TEXTURE] Could not create image sampler: %s\n", VkResult_str(res));
		return false;
	}

	return true;
}
bool destroy_texture_sampler(VulkanFrameData *frame_data, uint32_t index) {

	vkDestroySampler(frame_data->logical_dev, frame_data->textures.samplers[index], nullptr);

	return true;
}

bool create_framebuffers(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {

	setup_info->swapchain.framebuffers = realloc(setup_info->swapchain.framebuffers, sizeof(VkFramebuffer) * setup_info->swapchain.buffers_count);
	TEST_MALLOC(setup_info->swapchain.framebuffers)

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		VkImageView attachments[2] = {
		    setup_info->swapchain.views[i],
		    setup_info->depth_objects.view};

		VkFramebufferCreateInfo fb_create = {};
		fb_create.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_create.renderPass              = setup_info->renderpass;
		fb_create.attachmentCount         = 2;
		fb_create.pAttachments            = attachments;
		fb_create.width                   = setup_info->swapchain.extent.width;
		fb_create.height                  = setup_info->swapchain.extent.height;
		fb_create.layers                  = 1;

		auto res = vkCreateFramebuffer(frame_data->logical_dev, &fb_create, nullptr, &setup_info->swapchain.framebuffers[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[FRAMBUFFER] Could not create a frambuffer: %s\n", VkResult_str(res));
			free(setup_info->swapchain.framebuffers);
			return false;
		}
	}

	llog(LOG_DEBUG, "[FRAMBUFFER] frambuffers successfully created\n");

	return true;
}

bool destroy_framebuffers(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {

	for (size_t i = 0; i < setup_info->swapchain.buffers_count; ++i) {
		vkDestroyFramebuffer(frame_data->logical_dev, setup_info->swapchain.framebuffers[i], nullptr);
	}
	free(setup_info->swapchain.framebuffers);
	setup_info->swapchain.framebuffers = nullptr;

	llog(LOG_DEBUG, "[FRAMBUFFER] framebuffers successfully destroyed\n");
	return true;
}

bool create_descriptor_set(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {

	VkDescriptorSetLayout layouts[MAX_CONCURRENT_FRAMES];
	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
		layouts[i] = setup_info->pipeline.descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool              = setup_info->descriptor_pool;
	alloc_info.descriptorSetCount          = MAX_CONCURRENT_FRAMES;
	alloc_info.pSetLayouts                 = layouts;

	auto res = vkAllocateDescriptorSets(frame_data->logical_dev, &alloc_info, frame_data->frame_data_objects.descriptor_sets);

	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[DESCRIPTOR SET] Could not crate descriptor set: %s\n", VkResult_str(res));
	}

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
		VkDescriptorBufferInfo db_info = {};
		db_info.buffer                 = frame_data->frame_data_objects.uniform_buff[i];
		db_info.offset                 = 0;
		db_info.range                  = VK_WHOLE_SIZE;

		VkDescriptorImageInfo img_info = {};
		img_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		img_info.imageView             = frame_data->textures.views[0];
		img_info.sampler               = frame_data->textures.samplers[0];

		VkWriteDescriptorSet desc_write[2] = {};
		// uniform buffer
		desc_write[0].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write[0].dstSet               = frame_data->frame_data_objects.descriptor_sets[i];
		desc_write[0].dstBinding           = 0;
		desc_write[0].dstArrayElement      = 0;
		desc_write[0].descriptorType       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		desc_write[0].descriptorCount      = 1;
		desc_write[0].pBufferInfo          = &db_info;
		desc_write[0].pImageInfo           = nullptr; // Optional
		desc_write[0].pTexelBufferView     = nullptr; // Optional
		// image sampler
		desc_write[1].sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		desc_write[1].dstSet               = frame_data->frame_data_objects.descriptor_sets[i];
		desc_write[1].dstBinding           = 1;
		desc_write[1].dstArrayElement      = 0;
		desc_write[1].descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		desc_write[1].descriptorCount      = 1;
		desc_write[1].pImageInfo           = &img_info;
		desc_write[1].pTexelBufferView     = nullptr; // Optional

		vkUpdateDescriptorSets(frame_data->logical_dev, 2, desc_write, 0, nullptr);
	}

	llog(LOG_DEBUG, "[DESCRIPTOR SET] Descriptor set successfully created\n");

	return true;
}

bool create_sync_objects(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {

	VkCommandBufferAllocateInfo g_cmd_crate = {};
	g_cmd_crate.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	g_cmd_crate.commandPool                 = setup_info->graphics_cmd_pool;
	g_cmd_crate.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	g_cmd_crate.commandBufferCount          = MAX_CONCURRENT_FRAMES;

	auto res = vkAllocateCommandBuffers(frame_data->logical_dev, &g_cmd_crate, frame_data->frame_data_objects.cmd_buff);
	if (res != VK_SUCCESS) {
		llog(LOG_FATAL, "[COMMAND BUFFER] Could not create the graphics command buffer: %s\n", VkResult_str(res));
		return false;
	}

	llog(LOG_DEBUG, "[COMMAND BUFFER] Graphics command buffer successfully created\n");

	VkSemaphoreCreateInfo sem_create = {};
	sem_create.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fnc_create = {};
	fnc_create.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fnc_create.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {

		auto res = vkCreateSemaphore(frame_data->logical_dev, &sem_create, nullptr, &frame_data->frame_data_objects.image_available[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the Image available semaphore: %s\n", VkResult_str(res));
			return false;
		}

		res = vkCreateSemaphore(frame_data->logical_dev, &sem_create, nullptr, &frame_data->frame_data_objects.render_finished[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the render finished semaphore: %s\n", VkResult_str(res));
			return false;
		}

		res = vkCreateFence(frame_data->logical_dev, &fnc_create, nullptr, &frame_data->frame_data_objects.in_flight_fence[i]);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SYNCHRO] Could not create the render in in flight fence fence: %s\n", VkResult_str(res));
			return false;
		}
	}

	llog(LOG_DEBUG, "[SYNCHRO] Synchronizaion semphores successfully created\n");

	return true;
}

bool destroy_sync_objects(VulkanFrameData *frame_data) {

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; ++i) {
		vkDestroyFence(frame_data->logical_dev, frame_data->frame_data_objects.in_flight_fence[i], nullptr);
		vkDestroySemaphore(frame_data->logical_dev, frame_data->frame_data_objects.render_finished[i], nullptr);
		vkDestroySemaphore(frame_data->logical_dev, frame_data->frame_data_objects.image_available[i], nullptr);
	}

	llog(LOG_DEBUG, "[SYNCHRO] Synchronization objects successfully destroyed\n");

	return true;
}

bool create_vertex_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {
	VkDeviceSize   buf_size = sizeof(__temp__data);
	VkBuffer       buf_staging;
	VkDeviceMemory buf_staging_mem;
	create_buffer(setup_info, frame_data->physical_dev, buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buf_staging, &buf_staging_mem);

	void *data;
	vkMapMemory(frame_data->logical_dev, buf_staging_mem, 0, buf_size, 0, &data);
	memcpy(data, __temp__data, buf_size);
	vkUnmapMemory(frame_data->logical_dev, buf_staging_mem);

	create_buffer(setup_info, frame_data->physical_dev, buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &frame_data->vertex_buff, &frame_data->vertex_buff_mem);
	copy_buffer(setup_info, buf_staging, frame_data->vertex_buff, buf_size);

	llog(LOG_DEBUG, "[VMEM] Vertex buffer objects successfully created and allocated\n");

	vkDestroyBuffer(frame_data->logical_dev, buf_staging, nullptr);
	vkFreeMemory(frame_data->logical_dev, buf_staging_mem, nullptr);

	return true;
}

bool destroy_vertex_buffer(VulkanFrameData *frame_data) {
	vkFreeMemory(frame_data->logical_dev, frame_data->vertex_buff_mem, nullptr);
	vkDestroyBuffer(frame_data->logical_dev, frame_data->vertex_buff, nullptr);

	llog(LOG_DEBUG, "[VMEM] Vertex buffer objects successfully destroyed and freed\n");

	return true;
}

bool create_index_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {
	VkDeviceSize   buf_size = sizeof(__temp__indicies);
	VkBuffer       buf_staging;
	VkDeviceMemory buf_staging_mem;
	create_buffer(setup_info, frame_data->physical_dev, buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buf_staging, &buf_staging_mem);

	void *data;
	vkMapMemory(frame_data->logical_dev, buf_staging_mem, 0, buf_size, 0, &data);
	memcpy(data, __temp__indicies, buf_size);
	vkUnmapMemory(frame_data->logical_dev, buf_staging_mem);

	create_buffer(setup_info, frame_data->physical_dev, buf_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &frame_data->index_buff, &frame_data->index_buff_mem);
	copy_buffer(setup_info, buf_staging, frame_data->index_buff, buf_size);

	llog(LOG_DEBUG, "[VMEM] Index buffer objects successfully created and allocated\n");

	vkDestroyBuffer(frame_data->logical_dev, buf_staging, nullptr);
	vkFreeMemory(frame_data->logical_dev, buf_staging_mem, nullptr);

	return true;
}

bool destroy_index_buffer(VulkanFrameData *frame_data) {
	vkFreeMemory(frame_data->logical_dev, frame_data->index_buff_mem, nullptr);
	vkDestroyBuffer(frame_data->logical_dev, frame_data->index_buff, nullptr);

	llog(LOG_DEBUG, "[VMEM] Index buffer objects successfully destroyed and freed\n");

	return true;
}

bool create_uniform_buffer(VulkanFrameData *frame_data, VulkanSetupInfo *setup_info) {
	VkDeviceSize sz = sizeof(MVP);

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		create_buffer(setup_info, frame_data->physical_dev, sz, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &frame_data->frame_data_objects.uniform_buff[i], &frame_data->frame_data_objects.uniform_buff_mem[i]);

		vkMapMemory(frame_data->logical_dev, frame_data->frame_data_objects.uniform_buff_mem[i], 0, sz, 0, &frame_data->frame_data_objects.uniform_buff_mapped[i]);
	}
	llog(LOG_DEBUG, "[VMEM] Uniform buffers objects successfully created and allocated\n");

	return true;
}

bool destroy_uniform_buffer(VulkanFrameData *frame_data) {

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		vkDestroyBuffer(frame_data->logical_dev, frame_data->frame_data_objects.uniform_buff[i], nullptr);
		vkFreeMemory(frame_data->logical_dev, frame_data->frame_data_objects.uniform_buff_mem[i], nullptr);
	}

	llog(LOG_DEBUG, "[VMEM] Uniform buffer objects successfully destroyed and freed\n");

	return true;
}
