
#include <stdlib.h>
#include "chunk.h"
#include "memory.h"

void initChunk(PChunk chunk){
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL;
	initValueArray(&(chunk->constants));
	initLineInfo(&(chunk->lineInfo));
}
void writeChunk(PChunk chunk, uint8_t byte, int line){
	if (chunk->capacity < chunk->count + 1){
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(uint8_t,chunk->code, \
			oldCapacity,chunk->capacity);
	}
	chunk->code[chunk->count] = byte;
	writeLineInfo(&(chunk->lineInfo),line,chunk->count);
	chunk->count++;
}
void freeChunk(PChunk chunk){
	FREE_ARRAY(uint8_t,chunk->code,chunk->capacity);
	freeLineInfo(&(chunk->lineInfo));
	freeValueArray(&(chunk->constants));
	initChunk(chunk);
}
int addConstant(PChunk chunk,Value value){
	writeValueArray(&(chunk->constants),value);
	return (chunk->constants).count - 1;
}
int getLine(PChunk chunk, int offset){
	int lineCounter = 1;
	while (chunk->lineInfo.lines[lineCounter] < offset){
		lineCounter++;
	}
	return lineCounter;
}
void writeConstant(PChunk chunk, Value value,int line){
	int constantIdx = addConstant(chunk,value);
	if (constantIdx <= 255){
		writeChunk(chunk, OP_CONSTANT,line);
		writeChunk(chunk, constantIdx & 0xff,line);
	}
	else {
		writeChunk(chunk, OP_CONSTANT_LONG, line);
		writeChunk(chunk, constantIdx & 0xff, line);
		writeChunk(chunk, (constantIdx >> 8) & 0xff, line);
		writeChunk(chunk, (constantIdx >> 16) & 0xff, line);
	}
}