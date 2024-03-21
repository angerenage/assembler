#include "fileIO.h"

typedef struct FileContext {
	FILE *filePtr;
	unsigned int lineNumber;
	char *fileName;
	char *fileDirectory;
} FileContext;

static FileContext* fileStack = NULL;
static unsigned int stackSize = 0;
static size_t stackCapacity = 0;

void initFileStack(const char* fileName) {
	fileStack = (FileContext*)allocate(sizeof(FileContext));
	if (!openFile(fileName)) {
		free(fileStack);
		throw(EXIT_FAILURE);
	}
}

void freeFileStack() {
	for (unsigned int i = 0; i < stackSize; i++) {
		fclose(fileStack[i].filePtr);
		free(fileStack[i].fileName);
		free(fileStack[i].fileDirectory);
	}
	if (fileStack != NULL) free(fileStack);
}

char* constructPath(const char* directory, const char* fileName) {
	if (!directory || !fileName) return stringCopy(fileName);

	bool trim = strncmp(fileName, "./", 2) == 0;
	size_t bufferSize = strlen(directory) + strlen(fileName) + (trim ? 0 : 2);

	char* fullPath = (char*)allocate(bufferSize);

	if (directory[strlen(directory) - 1] == '/') {
		snprintf(fullPath, bufferSize, "%s%s", directory, &fileName[trim ? 2 : 0]);
	} else {
		snprintf(fullPath, bufferSize, "%s/%s", directory, &fileName[trim ? 2 : 0]);
	}

	return fullPath;
}

bool openFile(const char *filePath) {
	char *baseDir = stackSize > 0 ? fileStack[stackSize - 1].fileDirectory : NULL;
	char *fullPath;

	try {
		fullPath = constructPath(baseDir, filePath);
	}
	catch {
		return false;
	} end_try

	char *dirEnd = strrchr(fullPath, '/');
	char *fileName = dirEnd ? stringCopy(dirEnd + 1) : stringCopy(fullPath);
	char *directory = dirEnd ? stringCopyLength(fullPath, dirEnd - fullPath + 1) : NULL;

	for (unsigned int i = 0; i < stackSize; i++) {
		char *existingFullPath = constructPath(fileStack[i].fileDirectory, fileStack[i].fileName);
		if (strcmp(fullPath, existingFullPath) == 0) {
			if (i == stackSize - 1) 
				fprintf(stderr, "Error: Self include detected in file \"%s\"\n", fileStack[i].fileName);
			else
				fprintf(stderr, "Error: Circular include detected with file \"%s\" in \"%s\"\n", fileName, fileStack[stackSize - 1].fileName);
			
			free(fileName);
			if (directory) free(directory);
			free(fullPath);
			free(existingFullPath);

			return false;
		}
		free(existingFullPath);
	}

	FILE* inputFile = fopen(fullPath, "r");
	if (!inputFile) {
		fprintf(stderr, "Error: Failed to open input file \"%s\", %s\n", fullPath, strerror(errno));
		
		free(fileName);
		if (directory) free(directory);
		free(fullPath);

		return false;
	}
	free(fullPath);

	if (sizeof(FileContext) * stackSize == stackCapacity) {
		stackCapacity += sizeof(FileContext);
		fileStack = (FileContext*)reallocate(fileStack, stackCapacity);
	}

	fileStack[stackSize++] = (FileContext){inputFile, 1, fileName, directory};
	return true;
}

char* readLine(unsigned int *l) {
	int bufferSize = 64;
	char* buffer = (char*)allocate(bufferSize);
	int position = 0;
	char c;
	bool inLeadingSpaces = true;

	*l = fileStack[stackSize - 1].lineNumber;
	fileStack[stackSize - 1].lineNumber++;

	FILE* inputFile = fileStack[stackSize - 1].filePtr;

	while ((c = fgetc(inputFile)) != EOF) {
		// Trim starting blank characters
		if (inLeadingSpaces && (isblank((unsigned char)c) || c == '\r')) continue;
		else inLeadingSpaces = false;

		if (!isascii(c)) {
			free(buffer);

			char extendedChar[4] = {0, 0, 0, 0};
			extendedChar[0] = c;

			int bytesRead = 0;
			for (int i = 1; i < sizeof(extendedChar) - 1; ++i) {
				int nextChar = fgetc(inputFile);
				if (nextChar == EOF || isascii(nextChar)) break;
				extendedChar[i] = nextChar;
				bytesRead++;
			}

			fprintf(stderr, "Error: Non ASCII character '%.*s' (c. %d) on line %d\n", bytesRead + 1, extendedChar, position + 1, *l);
			throw(EXIT_FAILURE);
		}
		else if (c == '\n') {
			break;
		}
		else if (c == ';') {
			while (fgetc(inputFile) != '\n' && !feof(inputFile)); // Flush the rest of the line
			break;
		}

		buffer[position++] = c;

		if (position >= bufferSize) {
			bufferSize *= 2;
			buffer = (char*)reallocate(buffer, bufferSize);
		}
	}

	if (position == 0) {
		free(buffer);
		if (c == EOF) {
			if (stackSize > 1) {
				stackSize--;
				free(fileStack[stackSize].fileDirectory);

				if (fclose(fileStack[stackSize].filePtr) == EOF) {
					fprintf(stderr, "Error: Failed to close file \"%s\", %s\n", fileStack[stackSize].fileName, strerror(errno));
					free(fileStack[stackSize].fileName);
					throw(EXIT_FAILURE);
				}

				free(fileStack[stackSize].fileName);

				return "";
			}
			
			return NULL; // End of code
		}
	}
	else {
		// Trim ending blank characters
		for (position -= 1; position >= 0; position--) {
			if (!isblank(buffer[position]) && buffer[position] != '\r' && buffer[position] != '\n') {
				buffer[position + 1] = '\0';
				return buffer;
			}
		}
	}

	return "";
}

bool writeData(FILE* fp, const char *fileName, const void *data, size_t size) {
	if (data != NULL) {
		if (fwrite(data, size, 1, fp) != 1) {
			fprintf(stderr, "Error: Error writing data to file %s.\n", fileName);
			return false;
		}
	}

	return true;
}