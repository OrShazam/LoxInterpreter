#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"
#include "line.h"

typedef enum {
	OP_RETURN,
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_GLOBAL_SET,
	OP_GLOBAL_SET_LONG,
	OP_GLOBAL_GET,
	OP_GLOBAL_GET_LONG,
	OP_LOCAL_GET,
	OP_LOCAL_SET,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_POPN,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_NIL,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
	OP_NOT,
	OP_PRINT
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
int writeConstant(PChunk,Value,int);
#endif