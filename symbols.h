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
	unsigned int definitionLine;
	SymbolType type;
	unsigned int useNumber;
	bool resolved;
} Symbol;

// Variables
extern Symbol *symbols;
extern unsigned int symbolNbr;

bool isValidSymbolChar(char c);

Symbol parseVariableDef(const char *s, unsigned int l);
Symbol parseLabel(const char *s, unsigned int l, unsigned long address);
void addSymbol(Symbol symb, unsigned int l);

DataItem getSymbolValue(const char *name, unsigned int l);
int getSymbolIndex(const char *name, unsigned int l);

void freeSymbols();