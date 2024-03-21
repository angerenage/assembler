#pragma once

#include <stdint.h>

#include "utils/mem.h"
#include "fileIO.h"

typedef enum sectionType {
	SECTYP_TEXT		= 0b00100000,
	SECTYP_DATA		= 0b01000000,
	SECTYP_BSS		= 0b10000000,
} SectionType;

typedef struct dataBlock {
	DataItem data;
	struct dataBlock *next;
} DataBlock;

typedef struct section {
	unsigned long nameHash;
	char *name;
	unsigned int subNumber;
	uint32_t address;
	DataBlock *content;
	size_t size;
	SectionType sectionType;
} Section;

extern Section *sections;
extern unsigned int sectionNbr;
extern unsigned int currentIndex;

void initSections();
void addSection(Section sec);
void addSectionData(DataItem data);
void setSection(unsigned int sectionId);

unsigned int getSectionId(const char *name, unsigned int l);

void freeSections();