#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exception.h"
#include "logger.h"

void *allocate(size_t size);
void *allocateFill(size_t size);
void *reallocate(void* ptr, size_t size);
void *reallocateFill(void *ptr, size_t oldSize, size_t newSize);

char *stringCopy(const char* string);
char *stringCopyLength(const char* string, size_t length);