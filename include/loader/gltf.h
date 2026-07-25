#pragma once

#include <cglm_proxy.h>
#include <stddef.h>

typedef enum {
	GLTF_CAMERA_TYPE_PERSPECTIVE,
	GLTF_CAMERA_TYPE_ORTHOGRAPHIC,
} GLTF_CAMERA_TYPE;

typedef enum {
	GLTF_BUFFER_TYPE_ARRAYBUFFER,
	GLTF_BUFFER_TYPE_TEXT,
} GLTF_BUFFER_TYPE;

typedef enum {
	GLTF_BUFFERVIEW_TARGET_ARRAY_BUFFER,
	GLTF_BUFFERVIEW_TARGET_ELEMENT_ARRAY_BUFFER,
	GLTF_BUFFERVIEW_TARGET_ANIMATION_OR_SKIN,
} GLTF_BUFFERVIEW_TARGET;

typedef enum {
	GLTF_ACCESSOR_COMPONENT_TYPE_BYTE,
	GLTF_ACCESSOR_COMPONENT_TYPE_UNSIGNED_BYTE,
	GLTF_ACCESSOR_COMPONENT_TYPE_SHORT,
	GLTF_ACCESSOR_COMPONENT_TYPE_UNSIGNED_SHORT,
	GLTF_ACCESSOR_COMPONENT_TYPE_FLOAT,
} GLTF_ACCESSOR_COMPONENT_TYPE;

typedef enum {
	GLTF_ACCESSOR_TYPE_SCALAR,
	GLTF_ACCESSOR_TYPE_VEC2,
	GLTF_ACCESSOR_TYPE_VEC3,
	GLTF_ACCESSOR_TYPE_VEC4,
	GLTF_ACCESSOR_TYPE_MAT2,
	GLTF_ACCESSOR_TYPE_MAT3,
	GLTF_ACCESSOR_TYPE_MAT4,
} GLTF_ACCESSOR_TYPE;

typedef enum {
	GLTF_SAMPLER_FILTER_NEAREST,
	GLTF_SAMPLER_FILTER_LINEAR,
	GLTF_SAMPLER_FILTER_NEAREST_MIPMAP_NEAREST,
	GLTF_SAMPLER_FILTER_LINEAR_MIPMAP_NEAREST,
	GLTF_SAMPLER_FILTER_NEAREST_MIPMAP_LINEAR,
	GLTF_SAMPLER_FILTER_LINEAR_MIPMAP_LINEAR,
} GLTF_SAMPLER_FILTER;

typedef enum {
	GLTF_SAMPLER_WRAPPING_CLAMP_TO_EDGE,
	GLTF_SAMPLER_WRAPPING_MIRRORED_REPEAT,
	GLTF_SAMPLER_WRAPPING_REPEAT
} GLTF_SAMPLER_WRAPPING;

typedef enum {
	GLTF_CHANNLEL_TARGET_NAME_TRANSLATION,
	GLTF_CHANNLEL_TARGET_NAME_ROTATION,
	GLTF_CHANNLEL_TARGET_NAME_SCALE,
} GLTF_CHANNEL_TARGET_NAME;

typedef struct {
	const double xmag;
	const double ymag;
	const double zfar;
	const double znear;
} GLTF_CAMERA_ORTHOGRAPHIC;

typedef struct {
	const double aspect_ration;
	const double yfov;
	const double zfar;
	const double znear;
} GLTF_CAMERA_PERSPECTIVE;

typedef struct {
	const char *name;
	union {
		const GLTF_CAMERA_ORTHOGRAPHIC *orthographic;
		const GLTF_CAMERA_PERSPECTIVE  *perspective;
	};
	const GLTF_CAMERA_TYPE camera_type;
} GLTF_CAMERA;

typedef struct {
	const char            *name;
	const char            *uri;
	const uint64_t         size;
	const GLTF_BUFFER_TYPE type;
} GLTF_BUFFER;

typedef struct {
	const char                  *name;
	const GLTF_BUFFER           *buffer;
	const uint64_t               byte_offset;
	const uint64_t               size;
	const GLTF_BUFFERVIEW_TARGET target;
} GLTF_BUFFERVIEW;

typedef struct {
	const char                        *name;
	const uint64_t                     byte_offset;
	const uint64_t                     byte_stride;
	const GLTF_ACCESSOR_COMPONENT_TYPE component_type;
	const uint64_t                     count;
	const GLTF_ACCESSOR_TYPE           type;
	const size_t                      *max;
	const size_t                      *min;
} GLTF_ACCESSOR;

typedef struct {
	const char                 *name;
	const mat4                  bind_shape_matrix;
	const struct GLTF_ACCESSOR *inverse_bind_matrices;
	const struct GLTF_NODE     *children;
} GLTF_SKIN;

typedef struct {
	const char               *name;
	const char               *id;
	const struct GLTF_NODE   *children;
	const struct GLTF_CAMERA *camera;
	const struct GLTF_NODE   *skeletons;
	const struct GLTF_SKING  *skin;

} GLTF_NODE;

typedef struct {
	const char                 *name;
	const GLTF_SAMPLER_FILTER   magFilter;
	const GLTF_SAMPLER_FILTER   minFilter;
	const GLTF_SAMPLER_WRAPPING wrapS;
	const GLTF_SAMPLER_WRAPPING wrapT;
} GLTF_SAMPLER;

typedef struct {
	const GLTF_NODE                *ids;
	const GLTF_CHANNEL_TARGET_NAME *path;
} GLTF_CHANNEL_TARGET;

typedef struct {
	const GLTF_SAMPLER        *samplers;
	const GLTF_CHANNEL_TARGET *target;
} GLTF_ANIMATION_CHANNEL;


typedef struct {
	const char                     *name;
	const GLTF_ANIMATION_CHANNEL   *channels;
	const GLTF_ACCESSOR *;
} GLTF_ANIMATION;

typedef struct {
	const char      *name;
	const GLTF_NODE *nodes;
} GLTF_SCENE;

typedef struct {
	// file info
	const char       *filename;
	const uint64_t    file_size;
	const GLTF_SCENE *main_scene;
	const GLTF_SCENE *scences;
	const GLTF_SKIN  *skins;
} GLTF;
