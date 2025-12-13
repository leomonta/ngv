#pragma once

#include "ngv_objects.h"

bool create_renderer(const NGVRendererSettings *settings, NGVRenderer *renderer);

bool draw(const NGVRendererSettings *settings, NGVRenderer *renderer);

bool push_data(const NGVRenderer *renderer, const char *verticies, const size_t verticies_size, const char *indicies, const size_t indicies_size);
