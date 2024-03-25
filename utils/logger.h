#pragma once

#include <stdarg.h>
#include <stdio.h>

#include "../fileIO.h"
#include "exception.h"

typedef enum e_logLevel {
	LOG_DEBUG   = 0b00001,
	LOG_ERROR   = 0b00010,
	LOG_WARNING = 0b00100,
	LOG_MESSAGE = 0b01000,
	LOG_VERBOSE = 0b10000
} LogLevel;

#define DEBUG

#define MAX_LOG LOG_DEBUG
#define MIN_LOG LOG_VERBOSE

extern LogLevel logLevel;

void log_f(LogLevel level, char *format, ...);

void printLevel(LogLevel level);
const char *setTextColor(LogLevel level);

LogLevel getHighestLogableLevel(LogLevel level);