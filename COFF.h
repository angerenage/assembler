#pragma once

#include <time.h>

#include "fileIO.h"
#include "sections.h"
#include "symbols.h"

#define MAGIC_NUMBER 0x14C//0x014D
#define STRING_TABLE_ALLOC_SIZE 255

typedef enum relocationType {
	RT_IGNORE =			0b0000,
	RT_IMMEDIATE =		0b0001,
	RT_SPACE =			0b0010,
	RT_INSTRUCTION =	0b0011,
	RT_BYTE =			0b0000,
	RT_WORD =			0b0100,
	RT_LONG =			0b1000,
	RT_QUAD =			0b1100,
} RelocationType;

#if defined(_MSC_VER) // Microsoft Visual Studio Compiler
#pragma pack(push, 1)
struct objHeader {
	uint16_t	magic;		// Magic number   
	uint16_t	nscns;		// Number of Sections
	int32_t		timdat;		// Time & date stamp
	int32_t		symptr;		// File pointer to Symbol Table
	int32_t		nsyms;		// Number of Symbols
	uint16_t	opthdr;		// sizeof(Optional Header)
	uint16_t	flags;		// Flags
};

struct sectionHeader {
	uint8_t		name[8];	// Section Name
	uint32_t	paddr;		// Physical Address
	uint32_t	vaddr;		// Virtual Address
	uint32_t	size;		// Section Size in Bytes
	uint32_t	scnptr;		// File offset to the Section data
	uint32_t	relptr;		// File offset to the Relocation table for this Section
	uint32_t	lnnoptr;	// File offset to the Line Number table for this Section
	uint16_t	nreloc;		// Number of Relocation table entries
	uint16_t	nlnno;		// Number of Line Number table entries
	uint32_t	flags;		// Flags for this section
};

struct relocationEntry {
	uint32_t	vaddr;		// Reference Address
	uint32_t	symndx;		// Symbol index
	uint16_t	type;		// Type of relocation
};

struct symbolEntry {
	uint8_t		name[8]; 	// Symbol name
	uint32_t	value;		// Value of symbol
	int16_t		scnum;		// Section number
	uint16_t	type;		// Symbol type
	uint8_t		sclass;		// Storage class
	uint8_t		numaux;		// Auxiliary count
};
#pragma pack(pop)
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
struct __attribute__((packed)) objHeader {
	uint16_t	magic;		// Magic number   
	uint16_t	nscns;		// Number of Sections
	int32_t		timdat;		// Time & date stamp
	int32_t		symptr;		// File pointer to Symbol Table
	int32_t		nsyms;		// Number of Symbols
	uint16_t	opthdr;		// sizeof(Optional Header)
	uint16_t	flags;		// Flags
};

struct __attribute__((packed)) sectionHeader {
	uint8_t		name[8];	// Section Name
	uint32_t	paddr;		// Physical Address
	uint32_t	vaddr;		// Virtual Address
	uint32_t	size;		// Section Size in Bytes
	uint32_t	scnptr;		// File offset to the Section data
	uint32_t	relptr;		// File offset to the Relocation table for this Section
	uint32_t	lnnoptr;	// File offset to the Line Number table for this Section
	uint16_t	nreloc;		// Number of Relocation table entries
	uint16_t	nlnno;		// Number of Line Number table entries
	uint32_t	flags;		// Flags for this section
};

struct __attribute__((packed)) relocationEntry {
	uint32_t	vaddr;		// Reference Address
	uint32_t	symndx;		// Symbol index
	uint16_t	type;		// Type of relocation
};

struct __attribute__((packed)) symbolEntry {
	uint8_t		name[8]; 	// Symbol name
	uint32_t	value;		// Value of symbol
	uint16_t	scnum;		// Section number
	uint16_t	type;		// Symbol type
	uint8_t		sclass;		// Storage class
	uint8_t		numaux;		// Auxiliary count
};
#else
#error "Unsupported compiler"
#endif

void WriteObjectFile(FILE* fp, const char *fileName);

size_t writeHeader(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr);
void writeSectionHeader(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr);
void writeSections(FILE* fp, const char *fileName, uint32_t *dataPtr, uint32_t *relocPtr, size_t dataSize);
void writeSymbols(FILE* fp, const char *fileName);
void writeStringTable(FILE *fp, const char *fileName);

char *addString(char *s);
uint16_t getRelocType(DataItem data);

void freeRelocation();