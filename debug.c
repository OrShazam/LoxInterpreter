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
static int byteInstruction(const char* name, PChunk chunk, int offset){
	uint8_t slot = chunk->code[offset + 1];
	printf("%-16s %4d\n", name, slot);
	return offset + 2;
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
		case OP_GLOBAL_SET:
			return constantInstruction("OP_GLOBAL_SET",chunk,offset);
		case OP_GLOBAL_SET_LONG:
			return constantLongInstruction("OP_GLOBAL_SET_LONG",chunk,offset);
		case OP_GLOBAL_GET:
			return constantInstruction("OP_GLOBAL_GET",chunk,offset);
		case OP_GLOBAL_GET_LONG:
			return constantLongInstruction("OP_GLOBAL_GET_LONG",chunk,offset);
		case OP_LOCAL_GET:
			return byteInstruction("OP_LOCAL_GET", chunk, offset);
		case OP_LOCAL_SET:
			return byteInstruction("OP_LOCAL_SET", chunk, offset);
		case OP_PRINT:
			return simpleInstruction("OP_PRINT", offset);
		case OP_POP:
			return simpleInstruction("OP_POP",offset);
		case OP_POPN: {
			return byteInstruction("OP_POPN",chunk,offset);
		}		
		case OP_NIL:
			return simpleInstruction("OP_NIL", offset);
		case OP_TRUE:
			return simpleInstruction("OP_TRUE", offset);
		case OP_FALSE:
			return simpleInstruction("OP_FALSE", offset);
		case OP_EQUAL:
			return simpleInstruction("OP_EQUAL", offset);
		case OP_GREATER:
			return simpleInstruction("OP_GREATER", offset);
		case OP_LESS:
			return simpleInstruction("OP_LESS", offset);
		case OP_ADD:
			return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT:
			return simpleInstruction("OP_SUBTRACT", offset);
		case OP_MULTIPLY:
			return simpleInstruction("OP_MULTIPLY", offset);
		case OP_DIVIDE:
			return simpleInstruction("OP_DIVIDE", offset);
		case OP_NEGATE:
			return simpleInstruction("OP_NEGATE",offset);
		case OP_NOT:
			return simpleInstruction("OP_NOT",offset);
		default:
			printf("Unknown opcode %d\n", opcode);
			return offset + 1;
	}
}