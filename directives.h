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
	GLOBAL,
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

Directive parseDirective(const char *s, unsigned int l);
DirectiveKeyWord literalToDirective(char *literal, unsigned int l);
char **splitParameters(const char *s, unsigned int *num, unsigned int l);

DataItem *executeDirective(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);

DataItem *writeValue(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *changeSectionShort(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *changeSection(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *setSymbolVisibility(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *writeString(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *setAddress(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *includeFile(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *alignAddress(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);
DataItem *reserveSpace(Directive directive, unsigned int *elementNumber, uint32_t *address, unsigned int l);

void freeDirective(Directive directive);