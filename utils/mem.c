#include "mem.h"

void *allocate(size_t size) {
	void* val = malloc(size);
	if (!val) {
		log_f(LOG_ERROR, "Memory allocation failed, %s\n", strerror(errno));
		throw(1);
	}
	return val;
}

void *allocateFill(size_t size) {
	void *ptr = calloc(1, size);
	if (!ptr) {
		log_f(LOG_ERROR, "Array allocation failed, %s\n", strerror(errno));
		throw(1);
	}
	return ptr;
}

void *reallocate(void* ptr, size_t size) {
	void *temp = realloc(ptr, size);
	if (!temp) {
		free(ptr);
		log_f(LOG_ERROR, "Memory reallocation failed, %s\n", strerror(errno));
		throw(1);
	}
	return temp;
}

void *reallocateFill(void *ptr, size_t oldSize, size_t newSize) {
	void *newPtr = realloc(ptr, newSize);
	if (!newPtr) {
		free(ptr);
		log_f(LOG_ERROR, "Memory reallocation failed, %s\n", strerror(errno));
		throw(1);
	}
	memset((char*)newPtr + oldSize, 0, newSize - oldSize);
	return newPtr;
}

char *stringCopy(const char* string) {
	char *temp = strdup(string);
	if (!temp) {
		log_f(LOG_ERROR, "String copy failed, %s\n", strerror(errno));
		throw(1);
	}
	return temp;
}

char *stringCopyLength(const char* string, size_t length) {
	char *stringCpy = (char*)allocate(sizeof(char) * (length + 1));
	memcpy(stringCpy, string, sizeof(char) * length);
	stringCpy[length] = '\0';
	return stringCpy;
}