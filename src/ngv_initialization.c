#include "ngv_initialization.h"

#include "vulkan_static.h"
#include "vulkan_setup.h"
#include "vulkan_frame.h"

bool create_ngv_renderer(const RendererSettings settings) {

	VulkanStaticInfo static_info;
	VulkanSetupInfo setup_info;
	VulkanFrameData frame_data;

	create_static_info(&settings.static_settings, &static_info);
	create_setup_info(&settings.setup_settings, &setup_info, &static_info);
	create_frame_data(&frame_data, &setup_info);

	return true;
}
