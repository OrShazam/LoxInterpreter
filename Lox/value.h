#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef double Value;

typedef struct {
	int capacity;
	int count;
	Value* values;
} ValueArray, *PValueArray;
void initValueArray(PValueArray);
void writeValueArray(PValueArray, Value);
void freeValueArray(PValueArray);
void printValue(Value);
#endif