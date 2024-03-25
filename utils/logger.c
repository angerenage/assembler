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
		printf("%s", setTextColor(highestLevel));

		if (fileContext) {
			printf("%s > ", fileContext);
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

		printf(setTextColor(-1));
	}
}

void printLevel(LogLevel level) {
	switch (level)
	{
	case LOG_DEBUG:
		printf("\e[42m DEBUG \e[0m ");
		break;

	case LOG_ERROR:
		printf("\e[41m ERROR \e[0m ");
		break;

	case LOG_WARNING:
		printf("\e[43m WARNING \e[0m ");
		break;

	case LOG_MESSAGE:
		printf("\e[46m MESSAGE \e[0m ");
		break;

	case LOG_VERBOSE:
		printf("\e[47m INFO \e[0m ");
		break;
	
	default:
		break;
	}
}

const char *setTextColor(LogLevel level) {
	switch (level) {
		case LOG_ERROR:
			return "\e[31m";

		case LOG_WARNING:
			return "\e[33m";

		case LOG_MESSAGE:
			return "\e[36m";

		case LOG_DEBUG:
			return "\e[32m";

		default:
			return "\e[0m";
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