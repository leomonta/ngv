#pragma once

#include "ngv_objects.h"

#define NEW_NGVRENDERER(nm)             \
	VulkanStaticInfo __ngv_static = {}; \
	VulkanSetupInfo  __ngv_setup  = {}; \
	VulkanFrameData  __ngv_frame  = {}; \
	NGVRenderer      nm           = {__ngv_static, __ngv_setup, __ngv_frame}

bool create_renderer(const NGVRendererSettings *settings, NGVRenderer *renderer);

bool destroy_renderer(NGVRenderer *renderer);

bool draw(const NGVRendererSettings *settings, NGVRenderer *renderer);

bool push_data(NGVRenderer *renderer, const void *verticies, const uint32_t verticies_size, const uint32_t *indicies, const uint32_t indicies_count);
