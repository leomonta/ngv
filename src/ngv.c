#include "ngv.h"

#include "vulkan_frame.h"
#include "vulkan_memory.h"
#include "vulkan_setup.h"
#include "vulkan_static.h"
#include "vulkan_ops.h"


bool create_renderer(const NGVRendererSettings *settings, NGVRenderer *renderer) {

	if (!create_static_info(settings, renderer->static_info)) {
		return false;
	}
	if (!create_setup_info(settings, renderer->setup_info, renderer->static_info)) {
		return false;
	}
	if (!create_frame_data(settings, renderer->frame_data, renderer->setup_info)) {
		return false;
	}
	return true;
}


bool draw(const NGVRendererSettings *settings, NGVRenderer *renderer) {
	draw_frame(settings, renderer);
	return true;
}

bool push_data(const NGVRenderer *renderer, const char *verticies, const size_t verticies_size, const char *indicies, const size_t indicies_size) {
	push_to_buffer(renderer->setup_info, renderer->frame_data, renderer->frame_data->index_buff, indicies, indicies_size);
	push_to_buffer(renderer->setup_info, renderer->frame_data, renderer->frame_data->vertex_buff, verticies, verticies_size);
	return true;
}
