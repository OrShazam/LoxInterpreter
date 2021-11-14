#include <stdio.h>
#include "debug.h"
#include "value.h"
static int simpleInstruction(const char* name, int offset){
	printf("%s\n",name);
	return offset + 1;
}
static int constantInstruction(const char* name, PChunk chunk, int offset){
	uint8_t constantIdx = chunk->code[offset+1];
	printf("%-16s %08d '",name,constantIdx);
	printValue((chunk->constants).values[constantIdx]);
	printf("'\n");
	return offset + 2;
}
static int constantLongInstruction(const char* name, PChunk chunk, int offset){
	int constantIdx = chunk->code[offset+1] + chunk->code[offset+2] << 8 \
		+ chunk->code[offset+3] << 16;
	printf("%-16s %08d '",name,constantIdx);
	printValue((chunk->constants).values[constantIdx]);
	printf("'\n");
	return offset + 4;
}

void disassembleChunk(PChunk chunk, const char* name){
	printf("\t== %s ==\n",name);
	for (int offset = 0; offset < chunk->count;){
		offset = disassembleInstruction(chunk, offset);
	}
}
int disassembleInstruction(PChunk chunk, int offset){
	printf("\t%04d ",offset);
	uint8_t opcode = chunk->code[offset];
	switch(opcode){
		case OP_RETURN:
			return simpleInstruction("OP_RETURN",offset);
		case OP_CONSTANT:
			return constantInstruction("OP_CONSTANT",chunk,offset);
		case OP_CONSTANT_LONG:
			return constantLongInstruction("OP_CONSTANT_LONG",chunk,offset);
		default:
			printf("Unknown opcode %d\n", opcode);
			return offset + 1;
	}
}