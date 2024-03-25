#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils/mem.h"
#include "converter.h"
#include "fileIO.h"

typedef enum symbolType {LABEL, VARIABLE, GLOBAL_VAR, EXTERNAL} SymbolType;

typedef struct symbol {
	unsigned long nameHash;
	char* name;
	unsigned long long value;
	FileContext definitionContext;
	SymbolType type;
	unsigned int useNumber;
	bool resolved;
} Symbol;

extern Symbol *symbols;
extern unsigned int symbolNbr;

bool isValidSymbolChar(char c);

Symbol parseVariableDef(const char *s);
Symbol parseLabel(const char *s, unsigned long address);
void addSymbol(Symbol symb);

DataItem getSymbolValue(const char *name);
int getSymbolIndex(const char *name);

void freeSymbols();