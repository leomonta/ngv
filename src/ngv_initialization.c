#include "ngv_initialization.h"

#include "vulkan_static.h"

bool create_ngv_renderer(const RendererSettings settings) {

	VulkanStaticInfo vsi;

	create_static_info(settings.static_settings, &vsi);

	return true;
}

