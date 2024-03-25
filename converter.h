#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils/mem.h"
#include "symbols.h"
#include "fileIO.h"

unsigned int detectBase(const char *string);
bool charToDigit(char c, unsigned int base, unsigned int *digitValue);
unsigned long long *stringToValue(const char *string);

DataItem getStringValue(const char *s);