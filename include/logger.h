#pragma once

// quick macro to use the automatic __FILE__ and __LINE__ gcc defines
#define llog(level, format, ...) logger(level, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__, format __VA_OPT__(, ) __VA_ARGS__)

typedef enum : char {
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG
} logLevel;

void set_log_level(logLevel ll);

void logger(logLevel ll, const char *file_name, const unsigned line_num, const char *function_name, const char *format, ...);
