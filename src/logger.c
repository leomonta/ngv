#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

static char log_level = LOG_DEBUG;

void set_log_level(logLevel ll) {
	log_level = ll;
}

void logger(logLevel ll, const char *module_name, const unsigned line_num, const char *format, ...) {

	if (log_level < ll) {
		return;
	}

	va_list args;
	va_start(args, mex);

	const char *prefix = "[ UNKWN ]";

	switch (ll) {
	case LOG_DEBUG:
		prefix = "[ DEBUG ]";
		break;
	case LOG_INFO:
		prefix = "[  INFO ]";
		break;
	case LOG_WARNING:
		prefix = "[WARNING]";
		break;
	case LOG_ERROR:
		prefix = "[ ERROR ]";
		break;
	case LOG_FATAL:
		prefix = "[ FATAL ]";
		break;
	}

	printf("%s %s:%d \t", prefix, module_name, line_num);
	vprintf(format, args);
	fflush(stdout);

	va_end(args);
}
