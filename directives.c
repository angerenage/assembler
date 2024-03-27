#include "directives.h"

typedef struct {
	unsigned int minParams;
	int maxParams;
	DataItem *(*execute)(Directive directive, unsigned int *elementNumber, uint32_t *address);
} DirectiveMetadata;

static DirectiveMetadata directiveMetadata[] = {
	{1, -1, writeValue}, // BYTE
	{1, -1, writeValue}, // WORD
	{1, -1, writeValue}, // LONG
	{1, -1, writeValue}, // QUAD
	{0, 0, changeSectionShort}, // TEXT
	{0, 0, changeSectionShort}, // DATA
	{0, 0, changeSectionShort}, // BSS
	{1, 2, changeSection}, // SECTION
	{1, -1, setSymbolVisibility}, // GLOBAL
	{1, -1, setSymbolVisibility}, // EXTERN
	{1, -1, writeString}, // ASCII
	{1, -1, writeString}, // ASCIZ
	{1, 1, setAddress}, // ORG
	{1, 1, includeFile}, // INCLUDE
	{1, 1, alignAddress}, // ALIGN
	{1, 2, reserveSpace}, // SPACE
	{1, 2, reserveSpace}, // RESERVE
};

Directive parseDirective(const char *s) {
	int charIndex = 1;
	while (!isspace(s[charIndex]) && s[charIndex] != '\0') charIndex++;

	char *keyword = stringCopyLength(&s[1], charIndex - 1);
	DirectiveKeyWord keywrd = literalToDirective(keyword);

	while (s[charIndex] != '\0' && isblank(s[charIndex])) charIndex++;

	Directive directive;
	try {
		unsigned int pNum;
		directive = (Directive){
			.keywordLiteral = keyword,
			.keyword = keywrd,
			.parameter = s[charIndex] != '\0' ? splitParameters(&s[charIndex], &pNum) : NULL,
			.parameterNumber = pNum,
		};
	}
	catch {
		free(keyword);
		throw(EXIT_FAILURE);
	} end_try

	return directive;
}

DirectiveKeyWord literalToDirective(char *literal) {
	for(int i = 0; literal[i] != '\0'; i++) {
		literal[i] = tolower(literal[i]);
	}

	if (strcmp(literal, "byte") == 0) return BYTE;
	else if (strcmp(literal, "word") == 0) return WORD;
	else if (strcmp(literal, "long") == 0) return LONG;
	else if (strcmp(literal, "quad") == 0) return QUAD;
	else if (strcmp(literal, "data") == 0) return DATA;
	else if (strcmp(literal, "text") == 0) return TEXT;
	else if (strcmp(literal, "bss") == 0) return BSS;
	else if (strcmp(literal, "section") == 0) return SECTION;
	else if (strcmp(literal, "global") == 0) return GLOBAL;
	else if (strcmp(literal, "extern") == 0) return EXTERN;
	else if (strcmp(literal, "align") == 0) return ALIGN;
	else if (strcmp(literal, "ascii") == 0) return ASCII;
	else if (strcmp(literal, "asciz") == 0) return ASCIZ;
	else if (strcmp(literal, "org") == 0) return ORG;
	else if (strcmp(literal, "include") == 0) return INCLUDE;
	else if (strcmp(literal, "space") == 0) return SPACE;
	else if (strcmp(literal, "reserve") == 0) return RESERVE;

	log_f(LOG_ERROR, "Unknown \"%s\" directive\n", literal);
	free(literal);
	throw(EXIT_FAILURE);
}

char** splitParameters(const char *s, unsigned int *num) {
	int paramAlloc = 1, paramId = 0;
	char** parameters = (char**)allocate(sizeof(char*) * paramAlloc);

	int j;
	for (int i = 0; s[i] != '\0'; i = j) {
		for (j = i; s[j] != ',' && s[j] != '\0'; j++);

		char* parameter = stringCopyLength(&s[i], j - i);
		parameters[paramId++] = parameter;

		if (paramId == paramAlloc) {
			paramAlloc *= 2;
			parameters = (char**)reallocate(parameters, sizeof(char*) * paramAlloc);
		}

		if (s[j] == ',') j++;
		while (isblank(s[j])) j++; // Triming space
	}

	if (num != NULL) *num = paramId;
	return parameters;
}

DataItem *executeDirective(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	DirectiveMetadata meta = directiveMetadata[directive.keyword];

	if (directive.parameterNumber < meta.minParams) {
		log_f(LOG_ERROR, "Too few parameters for the .%s directive, expected at least %u\n", directive.keywordLiteral, meta.minParams);
		freeDirective(directive);
		throw(EXIT_FAILURE);
	}
	else if (meta.maxParams != -1 && directive.parameterNumber > (unsigned int)meta.maxParams) {
		log_f(LOG_ERROR, "Too many parameters for the .%s directive, expected at most %u\n", directive.keywordLiteral,  meta.maxParams);
		freeDirective(directive);
		throw(EXIT_FAILURE);
	}

	DataItem *data;
	try {
		data = meta.execute(directive, elementNumber, address);
	}
	catch {
		freeDirective(directive);
		throw(EXIT_FAILURE);
	} end_try
	
	if (directive.parameter != NULL) freeDirective(directive);
	return data;
}

DataItem *writeValue(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	size_t writeSize;
	switch (directive.keyword) {
		case BYTE:
			writeSize = sizeof(uint8_t);
			break;

		case WORD:
			writeSize = sizeof(uint16_t);
			break;

		case LONG:
			writeSize = sizeof(uint32_t);
			break;

		case QUAD:
			writeSize = sizeof(uint64_t);
			break;
	}

	DataItem *data = (DataItem*)allocate(sizeof(DataItem) * directive.parameterNumber);

	for (unsigned int i = 0; i < directive.parameterNumber; i++) {
		DataItem value = getStringValue(directive.parameter[i]);
		if (value.size == 0) {
			for (unsigned int j = 0; j < i; j++) {
				if (data[j].type == DIRECT) free(data[j].data.directData);
			}

			free(data);

			throw(EXIT_FAILURE);
		}
		value.size = writeSize;
		data[i] = value;
	}

	*elementNumber = directive.parameterNumber;
	return data;
}

DataItem *changeSectionShort(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	switch (directive.keyword) {
		case TEXT:
			setSection(0);
			break;

		case DATA:
			setSection(1);
			break;

		case BSS:
			setSection(2);
			break;
	}

	*elementNumber = 0;
	return NULL;
}

DataItem *changeSection(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	setSection(getSectionId(directive.parameter[0]));

	*elementNumber = 0;
	return NULL;
}

DataItem *setSymbolVisibility(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	for (unsigned int i = 0; i < directive.parameterNumber; i++) {
		if (directive.parameter[i][0] != '$') {
			log_f(LOG_ERROR, "Something other than a variable was passed as a parameter to the .global directive. A variable always begins with a $ character\n");
			throw(EXIT_FAILURE);
		}
		int symbolIndex = getSymbolIndex(&directive.parameter[i][1]);

		if (symbols[symbolIndex].resolved && directive.keyword == EXTERN) { // can't be a label and undefined
			log_f(LOG_ERROR, "Bad extern definition, the variable already has a value\n");
			throw(EXIT_FAILURE);
		}
		else if (symbols[symbolIndex].type == EXTERNAL) {
			log_f(LOG_ERROR, "Invalid global declaration. Symbol %s already defined as extern\n", symbols[symbolIndex].name);
			throw(EXIT_FAILURE);
		}
		else if (symbols[symbolIndex].type != VARIABLE) {
			log_f(LOG_ERROR, "Invalid global declaration. Only variables can be declared as global\n");
			throw(EXIT_FAILURE);
		}
		else if (symbols[symbolIndex].type == LABEL) {
			log_f(LOG_MESSAGE, "Useless global declaration. Labels are public by default and do not require global declaration\n");
		}

		if (symbols[symbolIndex].type == GLOBAL_VAR) {
			if (directive.keyword == EXTERN) {
				log_f(LOG_ERROR, "Bad extern definition, variable %s already defined as global\n", symbols[symbolIndex].name);
				throw(EXIT_FAILURE);
			}
			
			log_f(LOG_WARNING, "Useless global declaration. Variable %s already defined as global\n", symbols[symbolIndex].name);
		}
		else if (symbols[symbolIndex].type != LABEL) {
			symbols[symbolIndex].type = directive.keyword == GLOBAL ? GLOBAL_VAR : EXTERNAL;
		}
	}

	*elementNumber = 0;
	return NULL;
}

DataItem *writeString(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	DataItem *data = (DataItem*)allocate(sizeof(DataItem) * directive.parameterNumber);

	for (unsigned int i = 0; i < directive.parameterNumber; i++) {
		size_t paramLen = strlen(directive.parameter[i]);
		if (directive.parameter[i][0] != '"' || directive.parameter[i][paramLen - 1] != '"') {
			log_f(LOG_ERROR, "Invalid data format for a ascii write directive, expected a string between double quotes\n");
			throw(EXIT_FAILURE);
		}

		size_t resultSize = sizeof(char) * paramLen - (directive.keyword == ASCII ? 2 : 1);
		try {
			data[i] = (DataItem){DIRECT, .data.directData = allocate(resultSize), resultSize};
		}
		catch {
			free(data);
			throw(EXIT_FAILURE);
		} end_try

		memcpy(data[i].data.directData, &directive.parameter[i][1], resultSize);
		if (directive.keyword == ASCIZ) *((char*)data[i].data.directData + resultSize - 1) = '\0';
	}

	*elementNumber = directive.parameterNumber;
	return data;
}

DataItem *setAddress(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	uint32_t *targetAdd = (uint32_t*)stringToValue(directive.parameter[0]);
	if (targetAdd == NULL) {
		throw(EXIT_FAILURE);
	}
	uint32_t targetAddress = *targetAdd;
	free(targetAdd);

	if (sections[currentIndex].size == 0 || *address == targetAddress) {
		sections[currentIndex].address = targetAddress;
	}
	else {
		sections[currentIndex].subNumber++;
		
		char *name = (char*)allocate(sizeof(char) * (strlen(sections[currentIndex].name) + 4));
		sprintf(name, "%s$%d", sections[currentIndex].name, sections[currentIndex].subNumber);

		try {
			addSection((Section){0, name, 0, targetAddress, NULL, 0, sections[currentIndex].sectionType});
		}
		catch {
			free(name);
			throw(EXIT_FAILURE);
		} end_try

		setSection(sectionNbr - 1);
	}

	*address = targetAddress;
	*elementNumber = 0;
	return NULL;
}

DataItem *includeFile(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	char *param = directive.parameter[0];
	size_t length = strlen(&param[1]);

	if (param[0] == '"' && param[length] == '"') {
		char *fileName = stringCopyLength(&param[1], length - 1);
		if (!openFile(fileName)) {
			free(fileName);
			throw(EXIT_FAILURE);
		}
		free(fileName);
	}
	else {
		log_f(LOG_ERROR, "Expected a file path between double quote but got '%s'\n", param);
		throw(EXIT_FAILURE);
	}
	
	*elementNumber = 0;
	return NULL;
}

DataItem *alignAddress(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	unsigned long long *alignment = stringToValue(directive.parameter[0]);
	unsigned int shift = *address % *alignment;
	free(alignment);

	if (shift > 0) {
		DataItem *shifter = (DataItem*)allocate(sizeof(DataItem));
		try {
			*shifter = (DataItem){DIRECT, .data.directData = allocate(shift), shift};
		}
		catch {
			free(shifter->data.directData);
		} end_try

		*elementNumber = 1;
		return shifter;
	}

	*elementNumber = 0;
	return NULL;
}

DataItem *reserveSpace(Directive directive, unsigned int *elementNumber, uint32_t *address) {
	DataItem sizeDI = getStringValue(directive.parameter[0]);
		
	bool valSet = directive.parameterNumber == 2;
	DataItem valDI;

	if (valSet)
		valDI = getStringValue(directive.parameter[1]);
	else
		valDI = (DataItem){DIRECT, NULL, 1};

	if (sizeDI.size == 0 || valDI.size == 0) {
		free(sizeDI.data.directData);
		if (valSet) free(valDI.data.directData);
		throw(EXIT_FAILURE);
	}
	else if (sizeDI.type == SYMBOL_REF) {
		log_f(LOG_ERROR, "Illegal use of an unresolved symbol as size parameter for a spacer directive\n");
		free(sizeDI.data.directData);
		if (valSet) free(valDI.data.directData);
		throw(EXIT_FAILURE);
	}

	size_t size = *(size_t*)sizeDI.data.directData;
	free(sizeDI.data.directData);

	DataItem *data;
	try {
		data = (DataItem*)allocate(sizeof(DataItem));
	}
	catch {
		if (valSet && valDI.type == DIRECT) free(valDI.data.directData);
		throw(EXIT_FAILURE);
	} end_try

	if (valSet) {
		if (valDI.type == DIRECT) {
			try {
				*data = (DataItem){DIRECT, .data.directData = allocate(size), size};
			}
			catch {
				free(valDI.data.directData);
				free(data);
				throw(EXIT_FAILURE);
			} end_try

			memset(data->data.directData, *(char*)valDI.data.directData, size);
			free(valDI.data.directData);
		}
		else {
			*data = (DataItem){RESERVED_SPACE, .data.symbolIndex = valDI.data.symbolIndex, size};
		}
	}
	else {
		try {
			*data = (DataItem){DIRECT, .data.directData = allocate(size), size};
		}
		catch {
			free(data);
			throw(EXIT_FAILURE);
		} end_try

		memset(data->data.directData, 0, size);
	}

	*elementNumber = 1;
	return data;
}

void freeDirective(Directive directive) {
	free(directive.keywordLiteral);
	
	for (unsigned int i = 0; i < directive.parameterNumber; i++) {
		free(directive.parameter[i]);
	}

	free(directive.parameter);
	directive.parameter = NULL;
}