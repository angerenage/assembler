#include "instruction.h"

static void freeElements(InstructionElement* elements, unsigned int elementNbr) {
	for (unsigned int i = 0; i < elementNbr; i++) {
		free(elements[i].element);
	}
	free(elements);
}

InstructionElement* splitElements(const char* s, unsigned int *num, unsigned int l) {
	int elemAlloc = 1, part = 0, elemId = 0;
	InstructionElement* elements = (InstructionElement*)allocate(sizeof(InstructionElement) * elemAlloc);

	int j = 0;
	for (int i = 0; s[i] != '\0'; i = j) {
		while (isblank(s[i]) || s[i] == ',') {
			if (s[i] == ',') {
				part++;
				if (part >= 3) {
					log_f(LOG_ERROR, "Unexpected ',' (c. %u) on line %u\n", i, l);

					freeElements(elements, elemId);

					throw(EXIT_FAILURE);
				}
			}
			i++;
		}
		j = i;

		while (s[j] != '\0' && !isblank(s[j]) && s[j] != ',') j++;

		if (j > i) {
			char* element = stringCopyLength(&s[i], j - i);
			elements[elemId++] = (InstructionElement){element, part};

			if (elemId == elemAlloc) {
				elemAlloc *= 2;
				elements = (InstructionElement*)reallocate(elements, sizeof(InstructionElement) * elemAlloc);
			}
		}
	}

	if (num != NULL) *num = elemId;
	return elements;
}

Instruction parseInstruction(const char* s, unsigned int l) {
	unsigned int elementNbr;
	InstructionElement* elements = splitElements(s, &elementNbr, l);

	Instruction inst;
	OpCode op = parseOpCode(elements[0].element);
	if (op == ERROR) {
		freeElements(elements, elementNbr);
		throw(EXIT_FAILURE);
	}
	inst.opcode = op;

	//

	freeElements(elements, elementNbr);

	return inst;
}

OpCode parseOpCode(const char *s) {
	return NOP;
}
