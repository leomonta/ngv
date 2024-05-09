#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

#define RED   "\x1B[031m"
#define GRN   "\x1B[032m"
#define YEL   "\x1B[033m"
#define BLU   "\x1B[034m"
#define MAG   "\x1B[035m"
#define CYN   "\x1B[036m"
#define WHT   "\x1B[037m"
#define GRY   "\x1B[090m"
#define RESET "\x1B[0m"

static char log_level = LOG_DEBUG;

void set_log_level(logLevel ll) {
	log_level = ll;
}

void logger(logLevel ll, const char *file_name, const unsigned line_num, const char *function_name, const char *format, ...) {

	if (log_level < ll) {
		return;
	}

	va_list args;
	va_start(args, mex);

	const char *prefix = "[ UNKWN ]";

	switch (ll) {
	case LOG_DEBUG:
		prefix = BLU"[ DEBUG ]"RESET;
		break;
	case LOG_INFO:
		prefix = GRN"[  INFO ]"RESET;
		break;
	case LOG_WARNING:
		prefix = YEL"[WARNING]"RESET;
		break;
	case LOG_ERROR:
		prefix = MAG"[ ERROR ]"RESET;
		break;
	case LOG_FATAL:
		prefix = RED"[ FATAL ]"RESET;
		break;
	}

	printf("%s %s:%d "GRY"in"RESET" %s(...)\t"RESET, prefix, file_name, line_num, function_name);
	vprintf(format, args);
	fflush(stdout);

	va_end(args);
}
