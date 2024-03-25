#include <stdio.h>
#include <string.h>

#include "fileIO.h"
#include "COFF.h"
#include "instruction.h"
#include "symbols.h"
#include "directives.h"

#define MAX_FILE_NAME 255

static char* outFileName;
static FILE* outputFile;

typedef enum LineClass { BLANK, DIRECTIVE, INSTRUCTION, VARIABLE_DEF, MACRO_DEF } LineClass;

LineClass classifyLine(char *s) {
	for (int i = 0; s[i] != '\0'; i++) {
		if (isspace(s[i])) continue;

		if (s[i] == '$') return VARIABLE_DEF;
		else if (s[i] == '#') return MACRO_DEF;
		else if (s[i] == '.') return DIRECTIVE;
		else return INSTRUCTION;
	}

	return BLANK;
}

static uint32_t address = 0;
static Directive directive;

void parseLine(char *line, char *fileName, unsigned int l) {
	int charIndex = 0;

	bool inSQuotes = false, inDQuotes = false;
	for (unsigned int i = 0; line[i] != '\0'; i++) {
		if (line[i] == '\'') inSQuotes = !inSQuotes;
		if (line[i] == '"') inDQuotes = !inDQuotes;
		
		if (line[i] == ':' && !inSQuotes && !inDQuotes) {
			charIndex = i + 1;
			break;
		}
	}

	// Case of a label before other statement
	if (charIndex > 0) {
		char *label = stringCopyLength(line, charIndex);
		try {
			addSymbol(parseLabel(label, address));
		}
		catch {
			free(label);
			throw(EXIT_FAILURE);
		} end_try
		free(label);

		// Remove empty characters before the rest of the line
		while (line[charIndex] != '\0' && isblank(line[charIndex])) charIndex++;
	}

	// Parse the rest of the line
	if (line[charIndex] != '\0') {
		switch (classifyLine(&line[charIndex])) {
			case DIRECTIVE:
				directive = parseDirective(&line[charIndex]);
				DataItem *data;

				unsigned int elementNumber = 0;
				try {
					data = executeDirective(directive, &elementNumber, &address);
				}
				catch {
					throw(EXIT_FAILURE);
				} end_try

				if (data != NULL) {
					for (unsigned int i = 0; i < elementNumber; i++) {
						if (data[i].size > 0) {
							address += (uint32_t)data[i].size;
							try {
								addSectionData(data[i]);
							}
							catch {
								if (data[i].type == DIRECT) free(data[i].data.directData);
								free(data);
								throw(EXIT_FAILURE);
							} end_try
						}
					}
					free(data);
				}
				break;

			case INSTRUCTION:
				log_f(LOG_DEBUG, "line %u is an instruction\n", l);
				parseInstruction(&line[charIndex]);
				break;

			case VARIABLE_DEF:
				Symbol sym = parseVariableDef(&line[charIndex]);
				try {
					addSymbol(sym);
				}
				catch {
					free(sym.name);
					throw(EXIT_FAILURE);
				} end_try
				break;

			case MACRO_DEF:
				log_f(LOG_DEBUG, "line %u is a macro definition\n", l);
				break;

			case BLANK:
			default:
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	#ifdef DEBUG
		try {
			initFileStack("./test.asm");
		}
		catch {
			return EXIT_FAILURE;
		} end_try

		outputFile = fopen("./test.obj", "wb");
		if (outputFile == NULL) {
			log_f(LOG_ERROR, "Could not open test files\n");
			freeFileStack();
			return EXIT_FAILURE;
		}

		logLevel |= LOG_DEBUG;
	#else
		if (argc <= 1) {
			log_f(LOG_ERROR, "Missing argument : input file\n");
			return EXIT_FAILURE;
		}
		else {
			for (int i = 1; i < argc; i++) {
				if (strcmp(argv[i], "-o") == 0) {
					i++; // Skip to the next argument

					if (strlen(argv[i]) <= MAX_FILE_NAME)
						outFileName = argv[i];
					else {
						log_f(LOG_ERROR, "Output file name too long, using a.obj\n");
					}
					break;
				}
			}

			if (outFileName == NULL) outFileName = "a.obj";
			outputFile = fopen(outFileName, "wb");
			if (outputFile == NULL) {
				log_f(LOG_ERROR, "Could not open output file \"%s\"\n", outFileName);
				freeFileStack();
				return EXIT_FAILURE;
			}
		}

		try {
			initFileStack(argv[1]);
		}
		catch {
			if (outputFile != NULL) fclose(outputFile);
			return EXIT_FAILURE;
		} end_try
	#endif

	try {
		initSections();
	}
	catch {
		freeFileStack();
		if (outputFile != NULL) fclose(outputFile);
		return EXIT_FAILURE;
	} end_try

	unsigned int l = 0;
	try {
		char* rline;
		while ((rline = readLine(&l)) != NULL) {
			if (*rline != '\0') {
				try {
					parseLine(rline, argv[argc - 1], l);
				}
				catch {
					free(rline);
					throw(EXIT_FAILURE);
				} end_try
				free(rline);
			}
		}

		WriteObjectFile(outputFile, outFileName);
	}
	catch {
		freeSections();
		freeSymbols();
		fclose(outputFile);
		freeFileStack();

		return EXIT_FAILURE;
	} end_try

	freeSections();
	freeSymbols();
	fclose(outputFile);
	freeFileStack();

	return EXIT_SUCCESS;
}