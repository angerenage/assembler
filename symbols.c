#include "symbols.h"

Symbol *symbols;
unsigned int symbolNbr = 0;

bool isValidSymbolChar(char c) {
    return isalnum(c) || c == '_' || c == '-';
}

Symbol parseVariableDef(const char *s, unsigned int l) {
	Symbol var = {5381, NULL, 0, l, VARIABLE, 0, true};

	// Parsing name
	int charIndex;
	for (charIndex = 1; !isblank(s[charIndex]) && s[charIndex] != '='; charIndex++) {
		if (isspace(s[charIndex]) || s[charIndex] == '\0') {
			fprintf(stderr, "Error: Invalid variable declaration on line %u\n", l);
			throw(EXIT_FAILURE);
		}
		else if (!isValidSymbolChar(s[charIndex])) {
			fprintf(stderr, "Error: Invalid character \'%c\' (c. %d) in variable name \"%s\" on line %u\n", s[charIndex], charIndex + 1, s, l);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		var.nameHash = ((var.nameHash << 5) + var.nameHash) + s[charIndex];
	}
	
	int nameEndIndex = charIndex;

	// Converting value
	bool equalFinded = false;
	while (isblank(s[charIndex]) || s[charIndex] == '=') {
		if (s[charIndex] == '=') equalFinded = true;
		charIndex++;
	}

	if (!equalFinded && s[charIndex] != '=') {
		fprintf(stderr, "Error: Invalid element after the definition of variables %.*s (c. %d) on line %u, '=' expected\n", nameEndIndex - 1, &s[1], charIndex + 1, l);
		throw(EXIT_FAILURE);
	}

	unsigned long long *value = stringToValue(&s[charIndex], l);
	if (value != NULL) {
		var.value = *value;
		free(value);
	}
	else throw(EXIT_FAILURE);

	var.name = stringCopyLength(&s[1], nameEndIndex - 1);

	return var;
}

Symbol parseLabel(const char *s, unsigned int l, unsigned long address) {
	Symbol label = {5381, NULL, address, l, LABEL, 0, true};

	// Parsing name
	int charIndex;
	for (charIndex = 0; !isblank(s[charIndex]) && s[charIndex] != ':'; charIndex++) {
		if (s[charIndex] == '\0') {
			fprintf(stderr, "Error: Invalid label on line %u\n", l);
			throw(EXIT_FAILURE);
		}
		else if (!isValidSymbolChar(s[charIndex])) {
			fprintf(stderr, "Error: Invalid character \'%c\' (c. %d) in label name \"%s\" on line %u\n", s[charIndex], charIndex + 1, s, l);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		label.nameHash = ((label.nameHash << 5) + label.nameHash) + s[charIndex];
	}

	label.name = stringCopyLength(s, charIndex);

	// Checking the conformity of the rest of the line
	while (s[charIndex] != '\0' && s[charIndex] != ':') {
		if (!isspace(s[charIndex]) && s[charIndex] != ':') {
			fprintf(stderr, "Error: Invalid element after the definition of label %s (c. %d) on line %u\n", s, charIndex + 1, l);
			throw(EXIT_FAILURE);
		}
		charIndex++;
	}

	return label;
}

void addSymbol(Symbol symb, unsigned int l) {
	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].nameHash == symb.nameHash) {
			if (!symbols[i].resolved && symb.resolved) {
				symbols[i].value = symb.value;
				symbols[i].type = symb.type;

				symbols[i].definitionLine = l;
				symbols[i].resolved = true;
				if (symbols[i].type == EXTERNAL) symbols[i].type == GLOBAL_VAR;
				
				free(symb.name);
				return;
			}
			else {
				fprintf(stderr, "Error: Redefinition of symbol %s on line %u, already defined on line %u\n", symb.name, l, symb.definitionLine);
				throw(EXIT_FAILURE);
			}
		}
	}

	symbolNbr++;
	if (!symbols) {
		symbols = (Symbol*)allocate(sizeof(Symbol));
	}
	else {
		symbols = (Symbol*)reallocate(symbols, sizeof(Symbol) * symbolNbr);
	}

	memcpy(&symbols[symbolNbr - 1], &symb, sizeof(Symbol));
}

DataItem getSymbolValue(const char *name, unsigned int l) {
	unsigned long nameHash = 5381, length = 0;
	for (int i = 0; name[i] != '\0'; i++) {
		if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			fprintf(stderr, "Error: Invalid character \'%c\' (c. %d) in symbol name \"%s\" on line %u\n", name[i], i + 1, name, l);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		nameHash = ((nameHash << 5) + nameHash) + name[i];
		length++;
	}

	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].nameHash == nameHash) {
			symbols[i].useNumber++;
			return (DataItem){SYMBOL_REF, .data.symbolIndex = i, sizeof(unsigned long long)};
		}
	}

	addSymbol((Symbol){nameHash, stringCopy(name), 0, l, VARIABLE, 1, false}, l);
	return (DataItem){SYMBOL_REF, .data.symbolIndex = (symbolNbr - 1), sizeof(unsigned long long)};
}

int getSymbolIndex(const char *name, unsigned int l) {
	unsigned long nameHash = 5381;
	for (int i = 0; name[i] != '\0'; i++) {
		if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			fprintf(stderr, "Error: Invalid character \'%c\' (c. %d) in symbol name \"%s\" on line %u\n", name[i], i + 1, name, l);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		nameHash = ((nameHash << 5) + nameHash) + name[i];
	}

	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].nameHash == nameHash)
			return i;
	}

	addSymbol((Symbol){nameHash, stringCopy(name), 0, l, EXTERNAL, 1, false}, l);
	return symbolNbr - 1;
}

void freeSymbols() {
	if (symbols != NULL) {
		for (unsigned int i = 0; i < symbolNbr; i++) {
			free(symbols[i].name);
		}
		free(symbols);
	}
}