#pragma once
/**
 * Compile defines switches to enable / disable various functionality due to DEBUG / non DEBUG compilation
 */
#if ! defined(DEBUG) && ! defined(NDEBUG)
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

#define VULKAN_CHOSEN_PHYSICAL_DEVICE_ID 9504
