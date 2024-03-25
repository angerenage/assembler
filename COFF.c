#include "COFF.h"

static char *stringTable = NULL;

static void *data = NULL;
static void **relocations = NULL;

static unsigned long *relocationNumbers = NULL;
static unsigned long dataOffset = 0;
static unsigned long stringTableOffset = 0;
static size_t stringTableAllocatedSize = STRING_TABLE_ALLOC_SIZE;

void WriteObjectFile(FILE* fp, const char *fileName) {
	uint32_t dataPtr;
	uint32_t relocPtr;

	size_t dataSize;

	try {
		stringTable = (char*)allocateFill(stringTableAllocatedSize * sizeof(char));
		relocations = allocateFill(sizeof(void*) * sectionNbr);
		relocationNumbers = (unsigned long*)allocate(sizeof(unsigned long) * sectionNbr);
		
		dataSize = writeHeader(fp, fileName, &dataPtr, &relocPtr);
	}
	catch {
		if (stringTable != NULL) free(stringTable);
		freeRelocation();
		return;
	} end_try

	if (dataSize > 0) {
		data = allocateFill(dataSize);
	}
	
	try {
		writeSections(fp, fileName, &dataPtr, &relocPtr, dataSize);
		writeSymbols(fp, fileName);
		writeStringTable(fp, fileName);
	} catch end_try

	freeRelocation();
	if (stringTable != NULL) free(stringTable);
	if (data != NULL) free(data);
}

size_t writeHeader(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr) {
	uint32_t symboleTablePtr = sizeof(struct objHeader);
	*dataPtr = sizeof(struct objHeader);
	*relocPtr = sizeof(struct objHeader);

	size_t dataSize = 0;
	for (unsigned int i = 0; i < sectionNbr; i++) {
		symboleTablePtr += sizeof(struct sectionHeader);
		*dataPtr += sizeof(struct sectionHeader);
		*relocPtr += sizeof(struct sectionHeader);

		symboleTablePtr += sections[i].size;
		dataSize += sections[i].size;
	}
	for (unsigned int i = 0; i < symbolNbr; i++) {
		if (symbols[i].type == LABEL || symbols[i].type == EXTERNAL || !symbols[i].resolved) {
			symboleTablePtr += sizeof(struct relocationEntry) * symbols[i].useNumber;
			*dataPtr += sizeof(struct relocationEntry) * symbols[i].useNumber;
		}
	}

	struct objHeader header = {
		MAGIC_NUMBER,
		sectionNbr,
		(uint32_t)time(NULL),
		symboleTablePtr,
		symbolNbr,
		0,
		0x0000,
	};

	if (!writeData(fp, fileName, &header, sizeof(struct objHeader))) {
		throw(EXIT_FAILURE);
	}

	return dataSize;
}

void writeSectionHeader(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr) {
	for (unsigned int i = 0; i < sectionNbr; i++) {
		struct sectionHeader header = {
			.paddr = sections[i].address,
			.vaddr = sections[i].address,
			.size = (uint32_t)sections[i].size,
			.scnptr = *dataPtr,
			.relptr = *relocPtr,
			.lnnoptr = 0,
			.nlnno = 0,
			.flags = sections[i].sectionType,
		};
		*dataPtr += sections[i].size;

		char *stringRef = addString(sections[i].name);
		strncpy((char*)&header.name, stringRef, 8);
		if (stringRef[0] == '/') free(stringRef);

		unsigned int relocationNumber = 0;
		struct relocationEntry *relocationData = NULL;
		if (sections[i].content != NULL) {
			DataBlock *current = sections[i].content;
			do {
				if (current->data.type == DIRECT && current->data.data.directData != NULL) {
					memcpy((char*)data + dataOffset, current->data.data.directData, current->data.size);
				}
				else if (current->data.type != DIRECT) {
					if (symbols[current->data.data.symbolIndex].type == LABEL || symbols[current->data.data.symbolIndex].type == EXTERNAL || !symbols[current->data.data.symbolIndex].resolved) {
						relocationNumber++;
						if (relocationData == NULL) {
							relocationData = (struct relocationEntry*)allocate(sizeof(struct relocationEntry));
						}
						else {
							relocationData = (struct relocationEntry*)reallocate(relocationData, sizeof(struct relocationEntry) * relocationNumber);
						}
						relocationData[relocationNumber - 1] = (struct relocationEntry){header.vaddr + dataOffset, current->data.data.symbolIndex, getRelocType(current->data)};
					}

					if (symbols[current->data.data.symbolIndex].resolved) {
						if (current->data.type == SYMBOL_REF)
							memcpy((char*)data + dataOffset, &symbols[current->data.data.symbolIndex].value, current->data.size);
						else if (current->data.type == RESERVED_SPACE)
							memset((char*)data + dataOffset, (int)symbols[current->data.data.symbolIndex].value, current->data.size);
					}
				}

				dataOffset += current->data.size;
				current = current->next;
			}
			while (current != NULL);
		}

		header.nreloc = relocationNumber;
		*relocPtr += sizeof(struct relocationEntry) * relocationNumber;

		relocations[i] = (void*)relocationData;
		relocationNumbers[i] = relocationNumber;

		// Write header
		if (!writeData(fp, fileName, &header, sizeof(struct sectionHeader))) {
			throw(EXIT_FAILURE);
		}
	}
}

void writeSections(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr, size_t dataSize) {
	writeSectionHeader(fp, fileName, dataPtr, relocPtr);

	// Write relocation
	for (unsigned int i = 0; i < sectionNbr; i++) {
		if (relocations[i] != NULL) {
			if (!writeData(fp, fileName, relocations[i], sizeof(struct relocationEntry) * relocationNumbers[i])) {
				throw(EXIT_FAILURE);
			}
		}
	}

	// Write data
	if (data != NULL) {
		if (!writeData(fp, fileName, data, dataSize)) {
			throw(EXIT_FAILURE);
		}
		free(data);
		data = NULL;
	}
}

void writeSymbols(FILE* fp, const char *fileName) {
	if (symbolNbr > 0) {
		struct symbolEntry *symbolTable = NULL;

		unsigned int j = 0;
		for (unsigned int i = 0; i < symbolNbr; i++) {
			if (symbols[i].type != VARIABLE) {
				if (symbolTable == NULL) {
					symbolTable = (struct symbolEntry*)allocate(sizeof(struct symbolEntry));
				}
				else {
					try {
						symbolTable = (struct symbolEntry*)reallocate(symbolTable, sizeof(struct symbolEntry) * (j + 1));
					}
					catch {
						free(symbolTable);
						throw(EXIT_FAILURE);
					} end_try
				}

				struct symbolEntry symbol = {
					.scnum = (symbols[i].type == GLOBAL_VAR ? -1 : 0),
					.type = (symbols[i].type == LABEL ? 1 : 0),
					.numaux = 0,
				};

				if (symbols[i].type != EXTERNAL)
					symbol.sclass = (symbols[i].type == LABEL ? 6 : 1);
				else symbol.sclass = 3;

				if (symbols[i].resolved)
					symbol.value = (uint32_t)symbols[i].value;
				
				char *stringRef = addString(symbols[i].name);
				strncpy((char*)&symbol.name, stringRef, 8);
				if (stringRef[0] == '/') free(stringRef);

				memcpy(&symbolTable[j], &symbol, sizeof(struct symbolEntry));

				j++;
			}
			else if (!symbols[i].resolved) {
				log_f(LOG_ERROR, "Error: Use of the non external unresolved symbol \"%s\" at line %u\n", symbols[j].name, symbols[j].definitionLine);
				if (symbolTable != NULL) free(symbolTable);
				throw(EXIT_FAILURE);
			}
		}

		if (symbolTable != NULL && !writeData(fp, fileName, symbolTable, sizeof(struct symbolEntry) * j)) {
			free(symbolTable);
			throw(EXIT_FAILURE);
		}

		if (symbolTable != NULL) free(symbolTable);
	}
}

void writeStringTable(FILE *fp, const char *fileName) {
	uint32_t stringTableSize = stringTableOffset + sizeof(uint32_t);
	if (!writeData(fp, fileName, &stringTableSize, sizeof(uint32_t))) {
		throw(EXIT_FAILURE);
	}

	if (stringTableOffset > 0) {
		if (!writeData(fp, fileName, stringTable, stringTableOffset)) {
			throw(EXIT_FAILURE);
		}
	}
}

char *addString(char *s) {
	size_t length = strlen(s);
	if (length <= 8) {
		return s;
	}

	size_t sSize = length + 1;

	if (stringTableOffset + sSize > stringTableAllocatedSize) {
		size_t oldSize = stringTableAllocatedSize;
		stringTableAllocatedSize += STRING_TABLE_ALLOC_SIZE;
		stringTable = (char*)reallocateFill(stringTable, oldSize, stringTableAllocatedSize);
	}

	memcpy(stringTable + stringTableOffset, s, sSize);

	char *offsetString = (char*)allocate(9);
	snprintf(offsetString, 8, "/%lu", (unsigned int)stringTableOffset + sizeof(uint32_t));

	stringTableOffset += sSize;
	return offsetString;
}

uint16_t getRelocType(DataItem data) {
	if (data.type == SYMBOL_REF) {
		RelocationType reloc = RT_IMMEDIATE;
		if (data.size == sizeof(uint8_t)) reloc |= RT_BYTE;
		else if (data.size == sizeof(uint16_t)) reloc |= RT_WORD;
		else if (data.size == sizeof(uint32_t)) reloc |= RT_LONG;
		else if (data.size == sizeof(uint64_t)) reloc |= RT_QUAD;

		return reloc;
	}
	else if (data.type == RESERVED_SPACE) {
		return RT_SPACE;
	}
	else if (data.type == INSTRCTION_REF) {
		RelocationType reloc = RT_INSTRUCTION;
		
		// DIFFERENT INSTRUCTIONS LAYOUT FOR IMMEDIATE VALUES

		return reloc;
	}

	return RT_IGNORE;
}

void freeRelocation() {
	if (relocations != NULL) {
		for (unsigned int i = 0; i < sectionNbr; i++) {
			if (relocations[i] != NULL) free(relocations[i]);
		}
		free(relocations);
	}

	if (relocationNumbers != NULL) free(relocationNumbers);
}