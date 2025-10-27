// The place to define any kind of magic number of name string
//
// A #define will be used unless a pointer to the data is necessary
#pragma once

#include <vulkan/vulkan_core.h>

// ------------------------------------------------------------------------------------------------
// SWITCHES
// ------------------------------------------------------------------------------------------------

/**
 * Compile defines switches to enable / disable various functionality due to DEBUG / non DEBUG compilation
 */
#if !defined(DEBUG) && !defined(NDEBUG)
#	error "At least one between DEBUG and NDEBUG must be defined at compile time"
#endif

#if defined(DEBUG) && defined(NDEBUG)
#	error "Cannot define both DEBUG and NDEBUG at the same time"
#endif

#ifdef DEBUG
#	define USE_VALIDATION_LAYERS
#	define RAW_PRINTS // enable / disable random printfs in the code
#endif
#ifdef NDEBUG

#endif

// ------------------------------------------------------------------------------------------------
// NAMES
// ------------------------------------------------------------------------------------------------

#define NGV_DEFUALT_APPLICATION_NAME "NGV application"
#define NGV_ENGINE_NAME              "Neon Genesis Vulkan"

// ------------------------------------------------------------------------------------------------
// VALIDATION LAYERS
// ------------------------------------------------------------------------------------------------

// ugly I KNOW
#ifdef USE_VALIDATION_LAYERS
static const char        *VALIDATION_LAYERS[]     = {"VK_LAYER_KHRONOS_validation"};
static constexpr unsigned VALIDATION_LAYERS_COUNT = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);

static const char        *VALIDATION_EXTENSIONS[]     = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
static constexpr unsigned VALIDATION_EXTENSIONS_COUNT = sizeof(VALIDATION_EXTENSIONS) / sizeof(VALIDATION_EXTENSIONS[0]);
#endif

// ------------------------------------------------------------------------------------------------
// EXTENSIONS
// ------------------------------------------------------------------------------------------------

const char        *PHYSICAL_EXTENSIONS[]     = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr unsigned PHYSICAL_EXTENSIONS_COUNT = sizeof(PHYSICAL_EXTENSIONS) / sizeof(PHYSICAL_EXTENSIONS[0]);

// ------------------------------------------------------------------------------------------------
// VERSION
// ------------------------------------------------------------------------------------------------

#define NGV_APPLICATION_VERSION VK_MAKE_VERSION(0, 0, 0)
#define NGV_ENGINE_VERSION      VK_MAKE_VERSION(0, 0, 1)

// ------------------------------------------------------------------------------------------------
// DYNAMIC STATE
// ------------------------------------------------------------------------------------------------

constexpr VkDynamicState PIPELINE_DYNAMIC_STATE[]     = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}; // Do i really need these as dynamic state?
constexpr unsigned       PIPELINE_DYNAMIC_STATE_COUNT = sizeof(PIPELINE_DYNAMIC_STATE) / sizeof(PIPELINE_DYNAMIC_STATE[0]);


// ------------------------------------------------------------------------------------------------
// MAGIC_NUMBERS
// ------------------------------------------------------------------------------------------------

#define DEFAULT_WINDOW_HEIGHT 1080

#define DEFAULT_WINDOW_WIDTH 1920

// ????

#define VULKAN_CHOSEN_PHYSICAL_DEVICE_ID 9504

#define MAX_CONCURRENT_FRAMES 2

#define TEMP_ARRAY_SIZE 10
