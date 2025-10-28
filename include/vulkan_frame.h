#pragma once

#include "ngv_objects.h"

/*
typedef struct {
    VkBuffer       index_buff;
    VkDeviceMemory index_buff_mem;
    VkBuffer       vertex_buff;
    VkDeviceMemory vertex_buff_mem;
    FrameData      frame_data_objects;
    TextureData    textures;
} VulaknFrameData;
*/

bool create_frame_info(VulkanFrameData *frame_data);
