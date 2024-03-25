#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "utils/mem.h"

typedef struct FileContext {
	FILE *filePtr;
	unsigned int lineNumber;
	char *fileName;
	char *fileDirectory;
} FileContext;

void initFileStack(const char* fileName);
void freeFileStack();

typedef enum dataType {
	DIRECT,
	SYMBOL_REF,
	RESERVED_SPACE,
	INSTRCTION_REF,
} DataType;

typedef struct dataItem {
	DataType type;
	union {
		void* directData;
		unsigned int symbolIndex;
	} data;
	size_t size;
} DataItem;

char *constructPath(const char *directory, const char *fileName);
bool openFile(const char *fileName);

char *fileContextToString(FileContext context);
FileContext getFileContext();

char *readLine(unsigned int *l);
bool writeData(FILE* fp, const char *fileName, const void *data, size_t size);