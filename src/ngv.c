#include "ngv.h"

#include "logger.h"
#include "vulkan_frame.h"
#include "vulkan_memory.h"
#include "vulkan_ops.h"
#include "vulkan_setup.h"
#include "vulkan_static.h"

bool create_renderer(const NGVRendererSettings *settings, NGVRenderer *renderer) {

	if (!create_static_info(settings, &renderer->static_info)) {
		return false;
	}
	if (!create_setup_info(settings, &renderer->setup_info, &renderer->static_info)) {
		return false;
	}
	renderer->frame_data.physical_dev = renderer->static_info.physical_dev;
	renderer->frame_data.logical_dev  = renderer->setup_info.logical_dev;
	if (!create_frame_data(settings, &renderer->frame_data, &renderer->setup_info)) {
		return false;
	}

	llog(LOG_INFO, "[NGV] Renderer successfully created\n");
	return true;
}

bool destroy_renderer(NGVRenderer *renderer) {

	if (!destroy_frame_data(&renderer->frame_data)) {
		return false;
	}
	if (!destroy_setup_info(&renderer->setup_info)) {
		return false;
	}
	if (!destroy_static_info(&renderer->static_info)) {
		return false;
	}

	return true;
}

bool draw(const NGVRendererSettings *settings, NGVRenderer *renderer) {
	draw_frame(settings, renderer);
	return true;
}

bool push_data(NGVRenderer *renderer, const void *verticies, const uint32_t verticies_size, const uint32_t *indicies, const uint32_t indicies_count) {
	push_to_buffer(&renderer->setup_info, &renderer->frame_data, renderer->frame_data.index_buff, indicies, indicies_count * sizeof(uint32_t));
	renderer->frame_data.index_count = indicies_count;
	push_to_buffer(&renderer->setup_info, &renderer->frame_data, renderer->frame_data.vertex_buff, verticies, verticies_size);
	return true;
}
