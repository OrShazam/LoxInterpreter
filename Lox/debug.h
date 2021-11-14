#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

void disassembleChunk(PChunk, const char*);
int disassembleInstruction(PChunk, int);
static int simpleInstruction(const char* name, int offset);
static int constantInstruction(const char* name, PChunk chunk, int offset);
static int constantLongInstruction(const char* name, PChunk chunk, int offset);
#endif