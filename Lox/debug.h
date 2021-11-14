#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

void disassembleChunk(PChunk, const char*);
int disassembleInstruction(PChunk, int);
#endif