#include "symbols.h"

Symbol *symbols;
unsigned int symbolNbr = 0;

bool isValidSymbolChar(char c) {
    return isalnum(c) || c == '_' || c == '-';
}

Symbol parseVariableDef(const char *s) {
	Symbol var = {5381, NULL, 0, getFileContext(), VARIABLE, 0, true};

	// Parsing name
	int charIndex;
	for (charIndex = 1; !isblank(s[charIndex]) && s[charIndex] != '='; charIndex++) {
		if (isspace(s[charIndex]) || s[charIndex] == '\0') {
			log_f(LOG_ERROR, "Invalid variable declaration\n");
			throw(EXIT_FAILURE);
		}
		else if (!isValidSymbolChar(s[charIndex])) {
			log_f(LOG_ERROR, "Invalid character \'%c\' (c. %d) in variable name \"%s\"\n", s[charIndex], charIndex + 1, s);
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
		log_f(LOG_ERROR, "Invalid element after the definition of variables %.*s (c. %d), '=' expected\n", nameEndIndex - 1, &s[1], charIndex + 1);
		throw(EXIT_FAILURE);
	}

	unsigned long long *value = stringToValue(&s[charIndex]);
	if (value != NULL) {
		var.value = *value;
		free(value);
	}
	else throw(EXIT_FAILURE);

	var.name = stringCopyLength(&s[1], nameEndIndex - 1);

	return var;
}

Symbol parseLabel(const char *s, unsigned long address) {
	Symbol label = {5381, NULL, address, getFileContext(), LABEL, 0, true};

	// Parsing name
	int charIndex;
	for (charIndex = 0; !isblank(s[charIndex]) && s[charIndex] != ':'; charIndex++) {
		if (s[charIndex] == '\0') {
			log_f(LOG_ERROR, "Invalid label\n");
			throw(EXIT_FAILURE);
		}
		else if (!isValidSymbolChar(s[charIndex])) {
			log_f(LOG_ERROR, "Invalid character \'%c\' (c. %d) in label name \"%s\"\n", s[charIndex], charIndex + 1, s);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		label.nameHash = ((label.nameHash << 5) + label.nameHash) + s[charIndex];
	}

	label.name = stringCopyLength(s, charIndex);

	// Checking the conformity of the rest of the line
	while (s[charIndex] != '\0' && s[charIndex] != ':') {
		if (!isspace(s[charIndex]) && s[charIndex] != ':') {
			log_f(LOG_ERROR, "Invalid element after the definition of label %s (c. %d)\n", s, charIndex + 1);
			throw(EXIT_FAILURE);
		}
		charIndex++;
	}

	return label;
}

void addSymbol(Symbol symb) {
	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].nameHash == symb.nameHash) {
			if (!symbols[i].resolved && symb.resolved) {
				symbols[i].value = symb.value;
				symbols[i].type = symb.type;

				symbols[i].definitionContext = getFileContext();
				symbols[i].resolved = true;
				if (symbols[i].type == EXTERNAL) symbols[i].type == GLOBAL_VAR;
				
				free(symb.name);
				return;
			}
			else {
				char *fileContext = NULL;
				try {
					fileContext = fileContextToString(symb.definitionContext);
				}catch {} end_try

				if (fileContext) {
					log_f(LOG_ERROR, "Redefinition of symbol %s, already defined at %s\n", symb.name, fileContext);
					free(fileContext);
				}
				else {
					log_f(LOG_ERROR, "Redefinition of symbol %s, already defined\n", symb.name);
				}
				
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

DataItem getSymbolValue(const char *name) {
	unsigned long nameHash = 5381, length = 0;
	for (int i = 0; name[i] != '\0'; i++) {
		if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			log_f(LOG_ERROR, "Invalid character \'%c\' (c. %d) in symbol name \"%s\"\n", name[i], i + 1, name);
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

	addSymbol((Symbol){nameHash, stringCopy(name), 0, getFileContext(), VARIABLE, 1, false});
	return (DataItem){SYMBOL_REF, .data.symbolIndex = (symbolNbr - 1), sizeof(unsigned long long)};
}

int getSymbolIndex(const char *name) {
	unsigned long nameHash = 5381;
	for (int i = 0; name[i] != '\0'; i++) {
		if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-') {
			log_f(LOG_ERROR, "Invalid character \'%c\' (c. %d) in symbol name \"%s\"\n", name[i], i + 1, name);
			throw(EXIT_FAILURE);
		}

		// Computing hash
		nameHash = ((nameHash << 5) + nameHash) + name[i];
	}

	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].nameHash == nameHash)
			return i;
	}

	addSymbol((Symbol){nameHash, stringCopy(name), 0, getFileContext(), EXTERNAL, 1, false});
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