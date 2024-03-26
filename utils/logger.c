#include "logger.h"

LogLevel logLevel = LOG_ERROR | LOG_WARNING | LOG_MESSAGE;

void log_f(LogLevel level, char *format, ...) {
	if (logLevel & level) {
		LogLevel highestLevel = getHighestLogableLevel(level);

		char *fileContext = NULL;
		try {
			fileContext = fileContextToString(getFileContext());
		} catch {} end_try

		printLevel(highestLevel);
		setTextColor(highestLevel);

		if (fileContext) {
			fprintf((highestLevel > LOG_ERROR ? stdout : stderr), "%s > ", fileContext);
			free(fileContext);
		}

		va_list args;
		va_start(args, format);
		if (highestLevel > LOG_ERROR) {
			vfprintf(stdout, format, args);
		}
		else {
			vfprintf(stderr, format, args);
		}
		va_end(args);

		setTextColor(0);
	}
}

void printLevel(LogLevel level) {
	FILE* stream = (level > LOG_ERROR ? stdout : stderr);
	switch (level)
	{
		case LOG_DEBUG:
			fprintf(stream, "\e[42m DEBUG \e[0m ");
			break;

		case LOG_ERROR:
			fprintf(stream, "\e[41m ERROR \e[0m ");
			break;

		case LOG_WARNING:
			fprintf(stream, "\e[43m WARNING \e[0m ");
			break;

		case LOG_MESSAGE:
			fprintf(stream, "\e[46m MESSAGE \e[0m ");
			break;

		case LOG_VERBOSE:
			fprintf(stream, "\e[47m INFO \e[0m ");
			break;
		
		default:
			break;
	}
}

void setTextColor(LogLevel level) {
	FILE* stream = (level > LOG_ERROR ? stdout : stderr);
	switch (level) {
		case LOG_ERROR:
			fprintf(stream, "\e[31m");
			break;

		case LOG_WARNING:
			fprintf(stream, "\e[33m");
			break;

		case LOG_MESSAGE:
			fprintf(stream, "\e[36m");
			break;

		case LOG_DEBUG:
			fprintf(stream, "\e[32m");
			break;

		default:
			fprintf(stream, "\e[0m");
			break;
	}
}

LogLevel getHighestLogableLevel(LogLevel level) {
	for (LogLevel i = MAX_LOG; i <= MIN_LOG; i = i << 1) {
		if (logLevel & level & i) {
			return i;
		}
	}

	return 0;
}