#pragma once

#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "utils/mem.h"
#include "fileIO.h"
#include "converter.h"
#include "sections.h"

typedef enum directiveKeyWord {
	BYTE, WORD, LONG, QUAD,
	TEXT, DATA, BSS,
	SECTION,
	GLOBAL, EXTERN,
	ASCII, ASCIZ,
	ORG,
	INCLUDE,
	ALIGN,
	SPACE, RESERVE,
	//FILE, LINE
} DirectiveKeyWord;

typedef struct directive {
	char *keywordLiteral;
	DirectiveKeyWord keyword;
	char **parameter;
	unsigned int parameterNumber;
} Directive;

Directive parseDirective(const char *s);
DirectiveKeyWord literalToDirective(char *literal);
char **splitParameters(const char *s, unsigned int *num);

DataItem *executeDirective(Directive directive, unsigned int *elementNumber, uint32_t *address);

DataItem *writeValue(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *changeSectionShort(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *changeSection(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *setSymbolVisibility(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *writeString(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *setAddress(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *includeFile(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *alignAddress(Directive directive, unsigned int *elementNumber, uint32_t *address);
DataItem *reserveSpace(Directive directive, unsigned int *elementNumber, uint32_t *address);

void freeDirective(Directive directive);