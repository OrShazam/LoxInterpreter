#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"
#include "line.h"

typedef enum {
	OP_RETURN,
	OP_CONSTANT,
	OP_CONSTANT_LONG
} OpCode;

typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	LineInfo lineInfo;
	ValueArray constants;
} Chunk, *PChunk;
void initChunk(PChunk);
void writeChunk(PChunk, uint8_t,int);
void freeChunk(PChunk);
int addConstant(PChunk, Value);
int getLine(PChunk, int);
void writeConstant(PChunk,Value,int);
#endif