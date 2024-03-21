#pragma once

#include <stdint.h>
#include "fileIO.h" 
#include "utils/mem.h"

typedef enum opCode {NOP, ERROR} OpCode;
typedef enum condition {EQ, NE, MI, PL, VS, VC, CS, CC, PE, PO, LT, GE} Condition;
typedef enum flag {
	SIZE_8   = 0b00,
	SIZE_16  = 0b01,
	SIZE_32  = 0b10,
	SIZE_64  = 0b11,
	UNSIGNED = 0b100,

	FP32_1616 = 0b000,
	FP32_248  = 0b001,
	FP32_824  = 0b010,
	//FP32_NORM  = 0b011,
	FP64_3232 = 0b100,
	FP64_4816 = 0b101,
	FP64_1648 = 0b110,
	//FP64_NORM  = 0b111
} Flag;

typedef struct instruction {
	OpCode opcode;
	uint8_t destReg;
	uint8_t src1Reg;
	uint8_t src2Reg;
	Condition condition;
	Flag flags;
} Instruction;

typedef struct instructionElement {
	char* element;
	unsigned int part;
} InstructionElement;

InstructionElement *splitElements(const char* s, unsigned int *num, unsigned int l);
Instruction parseInstruction(const char* s, unsigned int l);
DataItem buildInstruction(Instruction instruction);

OpCode parseOpCode(const char* s);