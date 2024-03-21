#include "converter.h"

unsigned int detectBase(const char *string) {
    if (string[1] == 'b') return 2;
    else if (string[1] == 'o') return 8;
    else if (string[1] == 'x') return 16;
    return 10;
}

bool charToDigit(char c, unsigned int base, unsigned int *digitValue) {
    if (isdigit(c)) *digitValue = c - '0';
    else if (isalpha(c)) *digitValue = toupper(c) - 'A' + 10;
    else return false;

    return *digitValue < base;
}

unsigned long long *stringToValue(const char *string, unsigned int l) {
	unsigned long long *result = (unsigned long long*)allocate(sizeof(unsigned long long));
	*result = 0;

	if (string[0] == '\'') {
		if (string[2] == '\'' && string[3] == '\0') {
			*result = (unsigned long long)string[1];
			return result;
		}
		else {
			fprintf(stderr, "Error: Invalid use of single quote in %s at line %i\n", string, l);
			free(result);
			return NULL;
		}
	}

	unsigned int base = detectBase(string);
	for (int i = (base != 10 ? 2 : 0); string[i] != '\0'; i++) {
		unsigned int digitValue;
		if (!charToDigit(string[i], base, &digitValue)) {
			fprintf(stderr, "Error: Invalid digit \'%c\' (c. %d) in \"%s\" at line %i\n", string[i], i + 1, string, l);
			free(result);
			return NULL;
		}

		*result *= base;
		*result += digitValue;
	}

	return result;
}

DataItem getStringValue(const char *s, unsigned int l) {
	if (s[0] == '$') {
		return getSymbolValue(&s[1], l);
	}
	else {
		unsigned long long *value = stringToValue(s, l);
		if (value != NULL)
			return (DataItem){DIRECT, .data.directData = value, sizeof(unsigned long long)};
	}
	
	return (DataItem){DIRECT, NULL, 0};
}