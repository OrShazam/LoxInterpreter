#ifndef clox_vm_h
#define clox_vm_h
#include "chunk.h"
typedef struct {
	PChunk chunk;
	uint8_t* ip;
} VM;
typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;
void initVM();
void freeVM();
InterpretResult interpret(PChunk chunk);
static InterpretResult run();

#endif