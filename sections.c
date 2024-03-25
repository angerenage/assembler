#include "sections.h"

Section *sections;
unsigned int sectionNbr = 0;
unsigned int currentIndex = 0;

static DataBlock *currentDataBlock = NULL;

void initSections() {
	sections = (Section*)allocate(sizeof(Section) * 3);

	sections[0] = (Section){
		0, ".text", 0, 0, NULL, 0, SECTYP_TEXT,
	};
	sections[1] = (Section){
		0, ".data", 0, 0, NULL, 0, SECTYP_DATA,
	};
	sections[2] = (Section){
		0, ".bss", 0, 0, NULL, 0, SECTYP_BSS,
	};
	sectionNbr = 3;

	currentIndex = 0;
	currentDataBlock = NULL;
}

void addSection(Section sec) {
	sectionNbr++;
	sections = (Section*)reallocate(sections, sizeof(Section) * sectionNbr);
	sections[sectionNbr - 1] = sec;
}

void addSectionData(DataItem data) {
	sections[currentIndex].size += data.size;

	if (data.type == DIRECT && currentDataBlock != NULL && currentDataBlock->data.type == DIRECT) {
		size_t currentSize = currentDataBlock->data.size;

		currentDataBlock->data.size += data.size;
		currentDataBlock->data.data.directData = reallocate(currentDataBlock->data.data.directData, currentDataBlock->data.size);

		memcpy((uint8_t*)currentDataBlock->data.data.directData + currentSize, data.data.directData, data.size);

		free(data.data.directData);
	}
	else {
		DataBlock *newBlock = (DataBlock*)allocate(sizeof(DataBlock));
		*newBlock = (DataBlock){data, NULL};
		if (currentDataBlock != NULL) currentDataBlock->next = newBlock;
		else sections[currentIndex].content = newBlock;
		currentDataBlock = newBlock;
	}
}

void setSection(unsigned int sectionId) {
	currentIndex = sectionId;

	currentDataBlock = sections[currentIndex].content;
	if (sections[currentIndex].content != NULL) {
		while (currentDataBlock->next != NULL) {
			currentDataBlock = currentDataBlock->next;
		}
	}
}

unsigned int getSectionId(const char *name) {
	unsigned long nameHash = 0;
	
	int charIndex;
	for (charIndex = 0; name[charIndex] != '$' && name[charIndex] != '\0'; charIndex++) {
		if (!isalnum(name[charIndex]) && name[charIndex] != '_' && name[charIndex] != '-') {
			log_f(LOG_ERROR, "Invalid character \'%c\' (c. %d) in section name \"%s\"\n", name[charIndex], charIndex + 1, name);
			throw(EXIT_FAILURE);
		}

		nameHash = nameHash * 7 + name[charIndex];
	}
	
	char *realName = stringCopyLength(name, charIndex);
	if (strcmp(realName, "text") == 0) {
		free(realName);
		return 0;
	}
	else if (strcmp(realName, "data") == 0) {
		free(realName);
		return 1;
	}
	else if (strcmp(realName, "bss") == 0) {
		free(realName);
		return 2;
	}

	if (sectionNbr > 3) {
		for (unsigned int i = 3; i < sectionNbr; i++) {
			if (sections[i].nameHash == nameHash) {
				free(realName);
				return i;
			}
		}
	}

	addSection((Section){nameHash, realName, 0,0, NULL, 0, SECTYP_TEXT});

	return sectionNbr - 1;
}

void freeSections() {
	if (sections != NULL) {
		for (unsigned int i = 0; i < sectionNbr; i++) {
			if (i > 2) free(sections[i].name);

			if (sections[i].content != NULL) {
				DataBlock *current = sections[i].content;
				do {
					if (current->data.type == DIRECT && current->data.data.directData != NULL) {
						free(current->data.data.directData);
						current->data.data.directData = NULL;
					}

					DataBlock *last = current;
					current = current->next;
					free(last);
				}
				while (current != NULL);
			}
		}
		free(sections);
	}
}