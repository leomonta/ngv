#pragma once
/**
 * Compile defines switches to enable / disable various functionality due to DEBUG / non DEBUG compilation
 */

#if defined(DEBUG) && defined(NDEBUG)
#	error "cannot define both DEBUG and NDEBUG at the same time"
#endif

#ifdef DEBUG
#	define USE_VALIDATION_LAYERS
#	define RAW_PRINTS // enable / disable random printfs in the code
#endif
#ifdef NDEBUG

#endif
